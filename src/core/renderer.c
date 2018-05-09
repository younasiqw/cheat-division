#include "renderer.h"
#include "renderer_shader_ps.h"
#include "renderer_shader_vs.h"
#include "renderer_texture.h"
#include <initguid.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdarg.h>

#define RELEASE(x) if (x) { (x)->lpVtbl->Release(x); x = 0; }
#define MAX_SPRITES 16384
#define TEXT_W 7
#define TEXT_H 9

typedef struct
{
	i32 sx;
	i32 sy;
	i32 sw;
	i32 sh;
	i32 dx;
	i32 dy;
	i32 dw;
	i32 dh;
	u32 col;
} Sprite;

typedef struct
{
	i32 sx;
	i32 sy;
} UV;

static ID3D11Device* g_device;
static ID3D11DeviceContext* g_context;
static ID3D11RenderTargetView* g_rtv;
static ID3D11BlendState* g_blend;
static ID3D11DepthStencilState* g_depth;
static ID3D11RasterizerState* g_rast;
static ID3D11SamplerState* g_samp;
static ID3D11VertexShader* g_vs;
static ID3D11PixelShader* g_ps;
static ID3D11InputLayout* g_il;
static ID3D11Buffer* g_vb;
static ID3D11Buffer* g_ib;
static ID3D11Buffer* g_cb;
static ID3D11Buffer* g_sprite_buf;
static ID3D11Texture2D* g_tex;
static ID3D11ShaderResourceView* g_srv;
static Sprite* g_sprites;
static UV g_uvs[256];
static u32 g_num_sprites;
static u32 g_max_sprites;
static u32 g_vsync;
static b32 g_is_init;

void renderer_create(void* device, void* context)
{
	if (device == 0 || context == 0)
	{
		return;
	}

	g_device = device;
	g_context = context;

	D3D11_BLEND_DESC blend_desc = { 0 };
	blend_desc.AlphaToCoverageEnable = TRUE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

	g_device->lpVtbl->CreateBlendState(g_device, &blend_desc, &g_blend);

	D3D11_DEPTH_STENCIL_DESC depth_desc = { 0 };
	depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_desc.DepthFunc = D3D11_COMPARISON_GREATER;
	depth_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depth_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depth_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depth_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_desc.BackFace = depth_desc.FrontFace;

	g_device->lpVtbl->CreateDepthStencilState(g_device, &depth_desc, &g_depth);

	D3D11_RASTERIZER_DESC rast_desc = { 0 };
	rast_desc.CullMode = D3D11_CULL_BACK;
	rast_desc.FillMode = D3D11_FILL_SOLID;

	g_device->lpVtbl->CreateRasterizerState(g_device, &rast_desc, &g_rast);

	D3D11_SAMPLER_DESC samp_desc = { 0 };
	samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samp_desc.MaxLOD = D3D11_FLOAT32_MAX;

	g_device->lpVtbl->CreateSamplerState(g_device, &samp_desc, &g_samp);
	g_device->lpVtbl->CreateVertexShader(g_device, g_vs_bc, sizeof(g_vs_bc), NULL, &g_vs);
	g_device->lpVtbl->CreatePixelShader(g_device, g_ps_bc, sizeof(g_ps_bc), NULL, &g_ps);

	D3D11_INPUT_ELEMENT_DESC ild[] = {
		{ "P", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "T", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "S", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "D", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "C", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	g_device->lpVtbl->CreateInputLayout(g_device, ild, ARRAYSIZE(ild), g_vs_bc, sizeof(g_vs_bc), &g_il);

	f32 verts[] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
	};

	D3D11_BUFFER_DESC buf_desc = { 0 };
	buf_desc.ByteWidth = sizeof(verts);
	buf_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA init_data = { 0 };
	init_data.pSysMem = verts;

	g_device->lpVtbl->CreateBuffer(g_device, &buf_desc, &init_data, &g_vb);

	buf_desc.ByteWidth = sizeof(Sprite) * MAX_SPRITES;
	buf_desc.Usage = D3D11_USAGE_DYNAMIC;
	buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_device->lpVtbl->CreateBuffer(g_device, &buf_desc, 0, &g_sprite_buf);

	u16 ind[] = { 0, 1, 2, 3, 0, 2 };

	buf_desc.ByteWidth = sizeof(ind);
	buf_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buf_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buf_desc.CPUAccessFlags = 0;
	init_data.pSysMem = ind;

	g_device->lpVtbl->CreateBuffer(g_device, &buf_desc, &init_data, &g_ib);

	buf_desc.ByteWidth = 16;
	buf_desc.Usage = D3D11_USAGE_DYNAMIC;
	buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_device->lpVtbl->CreateBuffer(g_device, &buf_desc, 0, &g_cb);

	D3D11_TEXTURE2D_DESC tex_desc = { 0 };
	tex_desc.Width = 128;
	tex_desc.Height = 128;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA tex_data = { 0 };
	tex_data.pSysMem = g_tex_bc;
	tex_data.SysMemPitch = 128;
	tex_data.SysMemSlicePitch = sizeof(g_tex_bc);

	g_device->lpVtbl->CreateTexture2D(g_device, &tex_desc, &tex_data, &g_tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
	srv_desc.Format = tex_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;

	g_device->lpVtbl->CreateShaderResourceView(g_device, (ID3D11Resource*)g_tex, &srv_desc, &g_srv);

	for (u32 i = 33; i < 256; ++i)
	{
		u32 c = i - 33;
		g_uvs[i].sx = (c % 18) * TEXT_W;
		g_uvs[i].sy = (c / 18) * TEXT_H;
	}

	g_is_init = true;
}

void renderer_destroy(void)
{
	RELEASE(g_srv);
	RELEASE(g_tex);
	RELEASE(g_sprite_buf);
	RELEASE(g_cb);
	RELEASE(g_ib);
	RELEASE(g_vb);
	RELEASE(g_il);
	RELEASE(g_ps);
	RELEASE(g_vs);
	RELEASE(g_samp);
	RELEASE(g_rast);
	RELEASE(g_depth);
	RELEASE(g_blend);

	g_is_init = false;
}

void renderer_draw_area(i32 dx, i32 dy, i32 dw, i32 dh, i32 sx, i32 sy, i32 sw, i32 sh, u32 color)
{
	i32 i = g_num_sprites;

	g_sprites[i].sx = sx;
	g_sprites[i].sy = sy;
	g_sprites[i].sw = sw;
	g_sprites[i].sh = sh;

	g_sprites[i].dx = dx;
	g_sprites[i].dy = dy;
	g_sprites[i].dw = dw;
	g_sprites[i].dh = dh;

	g_sprites[i].col = color;

	g_num_sprites = (i + 1) % MAX_SPRITES;
}

void renderer_draw_esp(i32 x, i32 y, i32 w, i32 h, u32 color)
{
	renderer_draw_rect(x, y, w, 1, color);
	renderer_draw_rect(x, y + h, w, 1, color);
	renderer_draw_rect(x, y, 1, h, color);
	renderer_draw_rect(x + w, y, 1, h + 1, color);
}

void renderer_draw_hp(i32 x, i32 y, i32 w, i32 h, u32 color, f32 hp)
{
	u32 hp_w = (u32)(hp * w);
	renderer_draw_esp(x, y, w, 5, color);
	renderer_draw_rect(x + 1, y + 1, hp_w - 2, 3, RGBA(0, 255, 0, 255));
}

void renderer_draw_rect(i32 x, i32 y, i32 w, i32 h, u32 color)
{
	renderer_draw_area(x, y, w, h, 29, 46, 1, 1, color);
}

void renderer_draw_text(i32 x, i32 y, u32 color, i8 *src)
{
	i32 cx = x;
	i32 cy = y;

	for (u32 i = 0; src[i]; ++i)
	{
		u32 c = (u8)src[i];
	
		if (c != '\n')
		{
			if (c != ' ')
			{
				renderer_draw_area(cx, cy, TEXT_W, TEXT_H, g_uvs[c].sx, g_uvs[c].sy, TEXT_W, TEXT_H, color);
			}

			cx += TEXT_W;
		}
		else
		{
			cx = x;
			cy += TEXT_H;
		}
	}
}

void renderer_printf(i32 x, i32 y, u32 color, i8 const * fmt, ...)
{
	i8 buf[512];

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, arg);
	va_end(arg);

	renderer_draw_text(x, y, color, buf);
}

void renderer_frame_init(void* rtv, u32 res_x, u32 res_y)
{
	g_rtv = (ID3D11RenderTargetView*)rtv;

	D3D11_MAPPED_SUBRESOURCE ms;
	if (g_sprite_buf && SUCCEEDED(g_context->lpVtbl->Map(g_context, (ID3D11Resource*)g_sprite_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
	{
		g_num_sprites = 0;
		g_max_sprites = MAX_SPRITES;
		g_sprites = ms.pData;

		D3D11_VIEWPORT vp = { 0 };
		vp.Width = (f32)res_x;
		vp.Height = (f32)res_y;
		vp.MaxDepth = 1.f;
		g_context->lpVtbl->RSSetViewports(g_context, 1, &vp);

		f32 pbd[4] = { 0 };
		pbd[0] = (f32)(2.0 / vp.Width);
		pbd[1] = (f32)(2.0 / vp.Height);

		if (g_cb && SUCCEEDED(g_context->lpVtbl->Map(g_context, (ID3D11Resource*)g_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
		{
			f32 *data = ms.pData;

			for (u32 i = 0; i < 4; ++i)
			{
				data[i] = pbd[i];
			}

			g_context->lpVtbl->Unmap(g_context, (ID3D11Resource*)g_cb, 0);
		}

		g_context->lpVtbl->OMSetBlendState(g_context, g_blend, 0, 0xffffffff);
		g_context->lpVtbl->OMSetDepthStencilState(g_context, g_depth, 0);
		g_context->lpVtbl->OMSetRenderTargets(g_context, 1, &g_rtv, 0);
		g_context->lpVtbl->RSSetState(g_context, g_rast);
		g_context->lpVtbl->PSSetSamplers(g_context, 0, 1, &g_samp);
		g_context->lpVtbl->VSSetShader(g_context, g_vs, 0, 0);
		g_context->lpVtbl->PSSetShader(g_context, g_ps, 0, 0);
		g_context->lpVtbl->GSSetShader(g_context, 0, 0, 0);
		g_context->lpVtbl->DSSetShader(g_context, 0, 0, 0);
		g_context->lpVtbl->CSSetShader(g_context, 0, 0, 0);
		g_context->lpVtbl->IASetInputLayout(g_context, g_il);
		g_context->lpVtbl->IASetPrimitiveTopology(g_context, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_context->lpVtbl->IASetIndexBuffer(g_context, g_ib, DXGI_FORMAT_R16_UINT, 0);
		g_context->lpVtbl->VSSetConstantBuffers(g_context, 0, 1, &g_cb);
		g_context->lpVtbl->PSSetShaderResources(g_context, 0, 1, &g_srv);

		ID3D11Buffer *vbs[2];
		vbs[0] = g_vb;
		vbs[1] = g_sprite_buf;

		UINT strides[2] = { 16, sizeof(Sprite) };
		UINT offsets[2] = { 0 };
		g_context->lpVtbl->IASetVertexBuffers(g_context, 0, 2, vbs, strides, offsets);
	}
	else
	{
		g_max_sprites = 0;
		g_sprites = 0;
	}
}

void renderer_frame_post(void)
{
	if (g_sprites == 0)
	{
		return;
	}

	g_context->lpVtbl->Unmap(g_context, (ID3D11Resource*)g_sprite_buf, 0);

	if (g_num_sprites > 0)
	{
		g_context->lpVtbl->DrawIndexedInstanced(g_context, 6, g_num_sprites, 0, 0, 0);
	}
}

b32 renderer_is_init(void)
{
	return g_is_init;
}

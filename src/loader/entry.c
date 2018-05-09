#include "core/types.h"
#include "hook/hook.h"
#include <Windows.h>
#include <initguid.h>
#include <d3d11.h>

// The path to the mod to dynamically load.
#define MOD_PATH "mod.dll"

// The temporary path to copy the meter to before loading, to allow for DLL replacement.
#define TEMP_PATH "tmp.dll"

// Hooked D3D11 function typedefs.
typedef HRESULT(STDMETHODCALLTYPE *Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE *ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

// Mod functions typedefs.
typedef void(*mod_create_t)(ID3D11Device*, ID3D11DeviceContext*);
typedef void(*mod_method_t)(void);
typedef b32(*mod_update_t)(ID3D11RenderTargetView* rtv, u32 res_x, u32 res_y);

// Hooked D3D11 functions.
static Present_t _imp_Present;
static ResizeBuffers_t _imp_ResizeBuffers;

// Mod functions.
static mod_create_t _imp_mod_create;
static mod_method_t _imp_mod_destroy;
static mod_update_t _imp_mod_update;

// D3D11 state.
static ID3D11Device* g_device;
static ID3D11DeviceContext* g_context;
static ID3D11RenderTargetView* g_rtv;
static u32 g_res_x;
static u32 g_res_y;

// Loader state.
static HANDLE g_mod;
static b32 g_is_loaded;
static b32 g_is_loading;
static HANDLE g_load_thread;


// Loads the mod.
static DWORD WINAPI mod_load(void* param)
{
	// Copy the meter to the temporary path.
	b32 copy_res = CopyFile(TEXT(MOD_PATH), TEXT(TEMP_PATH), FALSE);
	TCHAR* path = copy_res ? TEXT(TEMP_PATH) : TEXT(MOD_PATH);

	// Try to load the meter.
	g_mod = LoadLibrary(path);
	if (g_mod == 0)
	{
		MessageBox(0, TEXT("Failed to load the meter."), TEXT("Info"), MB_OK);
		return 0;
	}

	// Import functions from the meter.
	_imp_mod_create = (mod_create_t)GetProcAddress(g_mod, "mod_create");
	_imp_mod_destroy = (mod_method_t)GetProcAddress(g_mod, "mod_destroy");
	_imp_mod_update = (mod_update_t)GetProcAddress(g_mod, "mod_update");

	// Try initializing the meter.
	if (_imp_mod_create)
	{
		_imp_mod_create(g_device, g_context);
	}

	g_is_loaded = true;

	return 0;
}

// Triggers a reload of the meter.
static void mod_reload(void)
{
	// Check if the meter is already reloading.
	if (g_is_loading == true)
	{
		return;
	}

	// Release the resources from the old meter.
	if (g_mod)
	{
		if (_imp_mod_destroy)
		{
			_imp_mod_destroy();
		}

		FreeLibrary(g_mod);

		_imp_mod_create = 0;
		_imp_mod_destroy = 0;
		_imp_mod_update = 0;
		g_mod = 0;
	}

	// Otherwise, we reset the loading and loaded state.
	g_is_loading = true;
	g_is_loaded = false;

	// Create the loading thread.
	g_load_thread = CreateThread(0, 0, mod_load, 0, 0, 0);

	// If thread creation failed, then we're done loading.
	if (g_load_thread == 0)
	{
		g_is_loading = false;
	}
}

// Updates the render target view.
static void rtv_update(IDXGISwapChain* swap)
{
	if (g_context == 0 || g_device == 0)
	{
		return;
	}

	ID3D11Texture2D *rtv_tex;
	if (FAILED(swap->lpVtbl->GetBuffer(swap, 0, &IID_ID3D11Texture2D, &rtv_tex)))
	{
		return;
	}

	if (SUCCEEDED(g_device->lpVtbl->CreateRenderTargetView(g_device, (ID3D11Resource*)rtv_tex, 0, &g_rtv)))
	{
		D3D11_TEXTURE2D_DESC rtv_desc;
		rtv_tex->lpVtbl->GetDesc(rtv_tex, &rtv_desc);

		g_res_x = (u32)rtv_desc.Width;
		g_res_y = (u32)rtv_desc.Height;
	}

	rtv_tex->lpVtbl->Release(rtv_tex);
}

// Present hook.
HRESULT STDMETHODCALLTYPE present_hook(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	// Check if we need to grab the d3d11 device and device context.
	static IDXGISwapChain* last_swap;
	if (last_swap != This)
	{
		last_swap = This;

		ID3D11Device* device;
		if (SUCCEEDED(This->lpVtbl->GetDevice(This, &IID_ID3D11Device, &device)))
		{
			g_device = device;
			device->lpVtbl->GetImmediateContext(device, &g_context);
			rtv_update(This);
			mod_reload();
		}
	}

	// Check if the DLL is currently loading or not.
	if (g_is_loading == false)
	{
		static b32 key_toggle;

		// If Ctrl + F9 is pressed, try reloading the DLL.
		b32 is_ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
		b32 is_f9 = (GetAsyncKeyState(VK_F9) & 0x8000);

		if (!key_toggle && is_ctrl && is_f9)
		{
			mod_reload();
			key_toggle = true;
		} else if (is_f9 == false)
		{
			key_toggle = false;
		}

		// If the DLL is loaded, then run the update.
		if (g_is_loaded && _imp_mod_update && _imp_mod_update(g_rtv, g_res_x, g_res_y))
		{
			mod_reload();
		}
	} else
	{
		// Check if the meter has finished reloading, but don't block.
		DWORD res = WaitForSingleObject(g_load_thread, 0);
		if (res != WAIT_TIMEOUT)
		{
			g_is_loading = false;
		}
	}

	return _imp_Present(This, SyncInterval, Flags);
}

// Resize hook.
HRESULT STDMETHODCALLTYPE resize_buffers_hook(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (g_rtv)
	{
		g_rtv->lpVtbl->Release(g_rtv);
		g_rtv = 0;
	}

	HRESULT hr = _imp_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	if (SUCCEEDED(hr))
	{
		rtv_update(This);
	}

	return hr;
}

// Initializes the loader.
static DWORD WINAPI loader_create(void* param)
{
	// Initialize hooking.
	if (MH_Initialize() != MH_OK)
	{
		MessageBox(0, TEXT("DLL hook failed to initialize."), TEXT("Error"), MB_OK);
		return 0;
	}

	// Create the dummy window to create the dummy swap chain.
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = GetModuleHandle(0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOWTEXT + 1);
	wc.lpszClassName = TEXT("wc");
	RegisterClass(&wc);

	RECT screen = { 0 };
	GetWindowRect(GetDesktopWindow(), &screen);

	HWND wnd = CreateWindow(wc.lpszClassName, TEXT("dummy"), WS_POPUP, 0, 0, 640, 480, 0, 0, wc.hInstance, 0);
	if (wnd == 0)
	{
		MessageBox(0, TEXT("Could not create dummy window."), TEXT("Error"), MB_OK);
		return 0;
	}

	// Create the dummy swap chain.
	DXGI_SWAP_CHAIN_DESC swap_desc = { 0 };
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_desc.BufferCount = 1;
	swap_desc.SampleDesc.Count = 1;
	swap_desc.OutputWindow = wnd;
	swap_desc.Windowed = TRUE;

	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain *swap;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swap_desc, &swap, &device, 0, &context);

	if (SUCCEEDED(hr))
	{
		context->lpVtbl->Release(context);
		device->lpVtbl->Release(device);

		MH_STATUS h1 = MH_CreateHook((LPVOID)swap->lpVtbl->Present, (LPVOID)&present_hook, (LPVOID*)&_imp_Present);
		MH_STATUS h2 = MH_CreateHook((LPVOID)swap->lpVtbl->ResizeBuffers, (LPVOID)&resize_buffers_hook, (LPVOID*)&_imp_ResizeBuffers);

		if (h1 == MH_OK && h2 == MH_OK)
		{
			if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
			{
				MessageBox(0, TEXT("Enabling the hooks failed."), TEXT("Error"), MB_OK);
			}
		} else if (h1 != MH_ERROR_ALREADY_CREATED || h2 != MH_ERROR_ALREADY_CREATED)
		{
			MessageBox(0, TEXT("Creating the hooks failed."), TEXT("Error"), MB_OK);
		}

		swap->lpVtbl->Release(swap);
	} 
	else
	{
		MessageBox(0, TEXT("Could not create dummy swap chain."), TEXT("Error"), MB_OK);
	}

	// Get rid of the dummy window.
	DestroyWindow(wnd);

	return 0;
}

// Destroys the loader.
static void loader_destroy(void)
{
	// Free dlls we loaded.
	if (g_mod)
	{
		FreeLibrary(g_mod);
	}
}

BOOL WINAPI DllMain(HANDLE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			CreateThread(0, 0, loader_create, 0, 0, 0);
		} break;

		case DLL_PROCESS_DETACH:
		{
			// Release our hooks.
			MH_Uninitialize();
		} break;
	}

	return TRUE;
}

#include "vector.h"
#include <math.h>

vec2 vec2_add(vec2 v1, vec2 v2)
{
	vec2 r;

	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;

	return r;
}

f32 vec2_dot(vec2 v1, vec2 v2)
{
	return (v1.x * v2.x + v1.y * v2.y);
}

vec2 vec2_identity()
{
	vec2 r = { 1.0f, 1.0f };

	return r;
}

f32 vec2_len(vec2 v1)
{
	return sqrtf(v1.x * v1.x + v1.y * v1.y);
}

vec2 vec2_lerp(vec2 v1, vec2 v2, f32 k)
{
	vec2 r;

	r.x = v1.x + k * (v2.x - v1.x);
	r.y = v1.y + k * (v2.y - v1.y);

	return r;
}

vec2 vec2_mul(vec2 v1, vec2 v2)
{
	vec2 r;

	r.x = v1.x * v2.x;
	r.y = v1.y * v2.y;

	return r;
}


vec2 vec2_neg(vec2 v1)
{
	vec2 r;

	r.x = -v1.x;
	r.y = -v1.y;

	return r;
}

vec2 vec2_norm(vec2 v1)
{
	vec2 r;

	f32 norm = sqrtf(v1.x * v1.x + v1.y * v1.y);

	if (norm == 0.0f)
	{
		r.x = 0.0f;
		r.y = 0.0f;
	}
	else
	{
		r.x = v1.x / norm;
		r.y = v1.y / norm;
	}

	return r;
}

vec2 vec2_perp(vec2 v1)
{
	vec2 r;

	r.x = v1.y;
	r.y = -v1.x;

	return r;
}


vec2 vec2_proj(vec2 v1, vec2 v2)
{
	vec2 r;

	f32 s =
		(v1.x * v2.x + v1.y * v2.y) /
		(v2.x * v2.x + v2.y * v2.y);

	r.x = v2.x * s;
	r.y = v2.y * s;

	return r;
}

vec2 vec2_scale(vec2 v1, f32 k)
{
	vec2 r;

	r.x = v1.x * k;
	r.y = v1.y * k;

	return r;
}

vec2 vec2_set(f32 x, f32 y)
{
	vec2 r;

	r.x = x;
	r.y = y;

	return r;
}

vec2 vec2_sub(vec2 v1, vec2 v2)
{
	vec2 r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;

	return r;
}

vec2 vec2_zero()
{
	vec2 r = { 0.0f, 0.0f };

	return r;
}

vec3 vec3_add(vec3 v1, vec3 v2)
{
	vec3 r;

	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;
	r.z = v1.z + v2.z;

	return r;
}

vec3 vec3_cross(vec3 v1, vec3 v2)
{
	vec3 r;

	r.x = (v1.y * v2.z) - (v1.z * v2.y);
	r.y = (v1.z * v2.x) - (v1.x * v2.z);
	r.z = (v1.x * v2.y) - (v1.y * v2.x);

	return r;
}

f32 vec3_dot(vec3 v1, vec3 v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

vec3 vec3_identity()
{
	vec3 r = { 1.0f, 1.0f, 1.0f };

	return r;
}


f32 vec3_len(vec3 v1)
{
	return sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
}

vec3 vec3_lerp(vec3 v1, vec3 v2, f32 k)
{
	vec3 r;

	r.x = v1.x + k * (v2.x - v1.x);
	r.y = v1.y + k * (v2.y - v1.y);
	r.z = v1.z + k * (v2.z - v1.z);

	return r;
}

vec3 vec3_mul(vec3 v1, vec3 v2)
{
	vec3 r;

	r.x = v1.x * v2.x;
	r.y = v1.y * v2.y;
	r.z = v1.z * v2.z;

	return r;
}

vec3 vec3_neg(vec3 v1)
{
	vec3 r;

	r.x = -v1.x;
	r.y = -v1.y;
	r.z = -v1.z;

	return r;
}

vec3 vec3_norm(vec3 v1)
{
	vec3 r;

	f32 norm = sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

	if (norm == 0.0f)
	{
		r.x = 0.0f;
		r.y = 0.0f;
		r.z = 0.0f;
	}
	else
	{
		r.x = v1.x / norm;
		r.y = v1.y / norm;
		r.z = v1.z / norm;
	}

	return r;
}

vec3 vec3_proj(vec3 v1, vec3 v2)
{
	vec3 r;

	f32 s =
		(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) /
		(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);

	r.x = v2.x * s;
	r.y = v2.y * s;
	r.z = v2.z * s;

	return r;
}

vec3 vec3_scale(vec3 v1, f32 k)
{
	vec3 r;

	r.x = v1.x * k;
	r.y = v1.y * k;
	r.z = v1.z * k;

	return r;
}

vec3 vec3_set(f32 x, f32 y, f32 z)
{
	vec3 r;

	r.x = x;
	r.y = y;
	r.z = z;

	return r;
}

vec3 vec3_sub(vec3 v1, vec3 v2)
{
	vec3 r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	r.z = v1.z - v2.z;

	return r;
}

vec3 vec3_zero()
{
	vec3 r = { 0.0f, 0.0f, 0.0f };

	return r;
}

vec4 vec4_add(vec4 v1, vec4 v2)
{
	vec4 r;

	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;
	r.z = v1.z + v2.z;
	r.w = v1.w + v2.w;

	return r;
}

f32 vec4_dot(vec4 v1, vec4 v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w);
}

vec4 vec4_identity()
{
	vec4 r = { 1.0f, 1.0f, 1.0f, 1.0f };

	return r;
}

f32 vec4_len(vec4 v1)
{
	return sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z + v1.w * v1.w);
}

vec4 vec4_lerp(vec4 v1, vec4 v2, f32 k)
{
	vec4 r;

	r.x = v1.x + k * (v2.x - v1.x);
	r.y = v1.y + k * (v2.y - v1.y);
	r.z = v1.z + k * (v2.z - v1.z);
	r.w = v1.w + k * (v2.w - v1.w);

	return r;
}

vec4 vec4_mul(vec4 v1, vec4 v2)
{
	vec4 r;

	r.x = v1.x * v2.x;
	r.y = v1.y * v2.y;
	r.z = v1.z * v2.z;
	r.w = v1.w * v2.w;

	return r;
}

vec4 vec4_neg(vec4 v1)
{
	vec4 r;

	r.x = -v1.x;
	r.y = -v1.y;
	r.z = -v1.z;
	r.w = -v1.w;

	return r;
}

vec4 vec4_norm(vec4 v1)
{
	vec4 r;

	f32 norm = sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z + v1.w * v1.w);

	if (norm == 0.0f)
	{
		r.x = 0.0f;
		r.y = 0.0f;
		r.z = 0.0f;
		r.w = 0.0f;
	}
	else
	{
		r.x = v1.x / norm;
		r.y = v1.y / norm;
		r.z = v1.z / norm;
		r.w = v1.w / norm;
	}

	return r;
}

vec4 vec4_proj(vec4 v1, vec4 v2)
{
	vec4 r;

	f32 s =
		(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w) /
		(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z + v2.w * v2.w);

	r.x = v2.x * s;
	r.y = v2.y * s;
	r.z = v2.z * s;
	r.w = v2.w * s;

	return r;
}

vec4 vec4_scale(vec4 v1, f32 k)
{
	vec4 r;

	r.x = v1.x * k;
	r.y = v1.y * k;
	r.z = v1.z * k;
	r.w = v1.w * k;

	return r;
}

vec4 vec4_set(f32 x, f32 y, f32 z, f32 w)
{
	vec4 r;

	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;

	return r;
}

vec4 vec4_sub(vec4 v1, vec4 v2)
{
	vec4 r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	r.z = v1.z - v2.z;
	r.w = v1.w - v2.w;

	return r;
}

vec4 vec4_zero()
{
	vec4 r = { 0.0f, 0.0f, 0.0f, 0.0f };

	return r;
}

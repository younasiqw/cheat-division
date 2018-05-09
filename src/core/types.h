#pragma once

// Signed integer types.
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long int i64;

// Unsigned integer types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

// Floating point types.
typedef float f32;
typedef double f64;

// Boolean type.
typedef u32 b32;
#define false 0
#define true 1

// Two dimension vector.
typedef struct vec2
{
	f32 x;
	f32 y;
} vec2;

// Three dimension vector.
typedef struct vec3
{
	f32 x;
	f32 y;
	f32 z;
} vec3;

// Four dimension vector.
typedef struct vec4
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} vec4;


// 4x4 Matrix.
typedef struct
{
	f32 _11, _12, _13, _14;
	f32 _21, _22, _23, _24;
	f32 _31, _32, _33, _34;
	f32 _41, _42, _43, _44;
} mat4;

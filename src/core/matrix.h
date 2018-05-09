#pragma once
#include "types.h"

// Matrix addition.
mat4 mat4_add(mat4 m1, mat4 m2);

// Matrix determinant.
f32 mat4_det(mat4 m1);

// Identity matrix.
mat4 mat4_identity();

// Matrix inverse.
mat4 mat4_inverse(mat4 m1);

// Matrix multiplication.
mat4 mat4_mul(mat4 m1, mat4 m2);

// Perspective matrix.
mat4 mat4_perspective_fov(f32 fov_y, f32 aspect, f32 z_near, f32 z_far);

// Matrix scaling.
mat4 mat4_scale(mat4 m1, f32 k);

// Matrix subtraction.
mat4 mat4_sub(mat4 m1, mat4 m2);

// Matrix transpose.
mat4 mat4_transpose(mat4 m1);

// Zero matrix.
mat4 mat4_zero();

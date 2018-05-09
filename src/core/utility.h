#pragma once

// PI values.
#define M_PI_2 1.57079632679
#define M_PI 3.14159265359f
#define M_3_PI_2 4.71238898038f
#define M_2_PI 6.28318530718f

// Returns the value of x aligned up to a multiple of y, where y is a power of 2.
#define ALIGN(x, y) (((x) + (y) - 1) & ~((y) - 1))

// Returns the number of elements in a array.
#define COUNT(x) (sizeof(x) / sizeof(*(x)))

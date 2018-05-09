#include "time.h"
#include <Windows.h>

static i64 g_freq;

void time_create(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&g_freq);
}

i64 time_get(void)
{
	i64 current;
	QueryPerformanceCounter((LARGE_INTEGER*)&current);
	return (current * 1000000) / g_freq;
}

#include "config.h"
#include <Windows.h>
#include <string.h>

// The path to the config file.
#define CONFIG_PATH "./config.ini"

i32 config_get_int(i8 const* sec, i8 const* key, i32 val)
{
	return (i32)GetPrivateProfileIntA(sec, key, val, CONFIG_PATH);
}

void config_get_str(i8* dst, u32 dst_bytes, i8 const* sec, i8 const* key, i8 const* val)
{
	GetPrivateProfileStringA(sec, key, val, dst, dst_bytes, CONFIG_PATH);
}

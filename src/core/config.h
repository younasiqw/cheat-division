#pragma once
#include "core/bind.h"
#include "core/types.h"

// Reads an integer value.
i32 config_get_int(i8 const* sec, i8 const* key, i32 val);

// Reads a string value.
void config_get_str(i8* dst, u32 dst_bytes, i8 const* sec, i8 const* key, i8 const* val);

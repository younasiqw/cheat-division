#pragma once
#include "core/types.h"

// Reads memory in a process.
#define PROCESS_READ(b, x) process_read(b, x, sizeof(*(x)))

// Writes data into a process.
#define PROCESS_WRITE(b, x) process_write(b, x, sizeof(*(x)))

// Type generic process memory read function creator.
#define PROCESS_READ_T(t) __forceinline t process_read_##t(u64 base) { t x; PROCESS_READ(base, &x); return x; } 

// Type generic process memory write function creator.
#define PROCESS_WRITE_T(t) __forceinline b32 process_write_##t(u64 base, t x) { return PROCESS_WRITE(base, &x); } 

// Closes a process.
void process_close(void);

// Scans and follows a relative address jump.
u64 process_follow(i8 const* pattern, i8 const* mask, u32 off);

// Returns the base address of a foreign process.
u64 process_get_base(i8* name);

// Returns the process handle.
void* process_get_handle(void);

// Returns the process id.
u32 process_get_id(void);

// Returns the handle to the window for the process.
void *process_get_window(void);

// Returns true if the process is open.
b32 process_is_open(void);

// Opens the process.
b32 process_open(void);

// Reads memory from a process.
b32 process_read(u64 base, void* dst, u32 bytes);

// Reads a string from a process.
b32 process_read_str(u64 base, i8* dst, u32 bytes);

// Scans for a pattern in a process.
u64 process_scan(void const* sig, i8 const* msk);

// Writes data into a process.
b32 process_write(u64 base, void const* src, u32 bytes);

// Type generic process memory reading functions.
PROCESS_READ_T(i8);
PROCESS_READ_T(i16);
PROCESS_READ_T(i32);
PROCESS_READ_T(i64);
PROCESS_READ_T(u8);
PROCESS_READ_T(u16);
PROCESS_READ_T(u32);
PROCESS_READ_T(u64);
PROCESS_READ_T(f32);
PROCESS_READ_T(f64);
PROCESS_READ_T(vec2);
PROCESS_READ_T(vec3);
PROCESS_READ_T(vec4);
PROCESS_READ_T(mat4);

// Type generic process memory writing functions.
PROCESS_WRITE_T(i8);
PROCESS_WRITE_T(i16);
PROCESS_WRITE_T(i32);
PROCESS_WRITE_T(i64);
PROCESS_WRITE_T(u8);
PROCESS_WRITE_T(u16);
PROCESS_WRITE_T(u32);
PROCESS_WRITE_T(u64);
PROCESS_WRITE_T(f32);
PROCESS_WRITE_T(f64);
PROCESS_WRITE_T(vec2);
PROCESS_WRITE_T(vec3);
PROCESS_WRITE_T(vec4);
PROCESS_WRITE_T(mat4);
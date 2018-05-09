#include "core/types.h"

// The maximum number of string bytes for type conversions.
#define STR_U32_LEN 18

// Appends src to the end of dst. Returns the length of the string in bytes.
u32 string_append(i8* dst, u32 dst_bytes, i8 const* src);

// Returns the length of the string in bytes.
u32 string_bytes(i8 const* str);

// Combines a list of strings into dst. Returns the length of the string in bytes.
u32 string_combine(i8* dst, u32 dst_bytes, i8 const** src, u32 src_num);

// Compares str1 to str2. Returns 0 if they are the same.
i32 string_compare(i8 const* s1, i8 const* s2);

// Copies src into dst. Returns the length of the string in bytes.
u32 string_copy(i8* dst, u32 dst_bytes, i8 const* src);

// Returns true if str1 ends with str2.
b32 string_ends_with(i8 const* str1, i8 const* str2);

// Converts a u3 to string and writes it to dst.
void string_from_u32(i8* dst, u32 dst_bytes, u32 src);

// Returns true if the str1 starts with str2.
b32 string_starts_with(i8 const* str1, i8 const* str2);

// Tokenizes a string in place. Returns the next token on success, 0 otherwise.
i8* string_token(i8* dst, i32 delim, i8** next);

// Returns the u32 representation of str on success, 0 otherwise.
u32 string_to_u32(i8 const* str);

// Converts the string to uppercase. Returns the length of the string in bytes.
u32 string_upper(i8* dst, u32 dst_byte, i8 const* src);


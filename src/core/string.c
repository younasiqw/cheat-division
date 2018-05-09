#include "core\string.h"

u32 string_append(i8* dst, u32 dst_bytes, i8 const* src)
{
	u32 i = 0;
	u32 j = 0;

	while (dst[i])
	{
		++i;
	}

	--dst_bytes;

	while (i < dst_bytes && src[j])
	{
		dst[i++] = src[j++];
	}

	dst[i] = 0;

	return i;
}

u32 string_bytes(i8 const* str)
{
	u32 i = 0;

	while (str[i])
	{
		++i;
	}

	return i;
}

u32 string_combine(i8* dst, u32 dst_bytes, i8 const** src, u32 src_num)
{
	u32 i = 0;
	u32 j = 0;
	u32 n = 0;

	--dst_bytes;

	while (i < dst_bytes && n < src_num)
	{
		if (src[n][j] == 0)
		{
			j = 0;
			++n;
		}
		else
		{
			dst[i++] = src[n][j++];
		}
	}

	dst[i] = 0;

	return i;
}

i32 string_compare(i8 const* s1, i8 const* s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}

	return *(u8*)s1 - *(u8*)s2;
}

u32 string_copy(i8* dst, u32 dst_bytes, i8 const* src)
{
	u32 i = 0;

	--dst_bytes;
	while (i < dst_bytes && src[i])
	{
		dst[i++] = src[i];
	}

	dst[i] = 0;

	return i;
}

b32 string_ends_with(i8 const* str1, i8 const* str2)
{
	u32 s1b = string_bytes(str1);
	u32 s2b = string_bytes(str2);
	return (s1b >= s2b) && (string_compare(str1 + s1b - s2b, str2) == 0);
}

void string_from_u32(i8* dst, u32 dst_bytes, u32 src)
{
	i8 buf[STR_U32_LEN];

	u32 n = 0;
	do
	{
		buf[n++] = (src % 10) + 48;
		src /= 10;
	} while (src > 0);

	--n;
	--dst_bytes;

	u32 p = 0;
	while (p < dst_bytes && p <= n)
	{
		dst[p++] = buf[n - p];
	}

	dst[p] = 0;
}

b32 string_starts_with(i8 const* str1, i8 const* str2)
{
	while (*str2)
	{
		if (*str1++ != *str2++)
		{
			return false;
		}
	}

	return true;
}

i8* string_token(i8* dst, i32 delim, i8** next)
{
	if (dst == 0)
	{
		dst = *next;
	}

	while (*dst != 0 && *dst == delim)
	{
		++dst;
	}

	if (*dst == 0)
	{
		return 0;
	}

	i8* tok = dst;
	while (*dst != 0 && *dst != delim)
	{
		dst += 1;
	}

	if (dst != 0)
	{
		*dst = 0;
		++dst;
	}

	*next = dst;

	return tok;
}

u32 string_to_u32(i8 const* str)
{
	u32 val = 0;
	for (u32 i = 0; str[i]; ++i)
	{
		val *= 10;
		val += str[i] - 48;
	}

	return val;
}

u32 string_upper(i8* dst, u32 dst_bytes, i8 const* src)
{
	--dst_bytes;

	u32 i = 0;
	while (i < dst_bytes && src[i])
	{
		i8 c = src[i];
		dst[i++] = c >= 'a' && c <= 'z' ? c - 32 : c;
	}

	dst[i] = 0;
	
	return i;
}
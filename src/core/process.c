#include "process.h"
#include "string.h"
#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <stdlib.h>

static HANDLE g_process;
static HWND g_window;
static DWORD g_id;

void process_close(void)
{
	if (g_process)
	{
		CloseHandle(g_process);
		g_process = 0;
	}

	if (g_window)
	{
		g_window = 0;
	}

	g_id = 0;
}

u64 process_follow(i8 const* pattern, i8 const* mask, u32 off)
{
	u64 addr = process_scan(pattern, mask);
	if (addr == 0)
	{
		return 0;
	}

	u32 offset = process_read_u32(addr + off);
	return (addr + offset + off + 4);
}

u64 process_get_base(i8* name)
{
	return (u64)GetModuleHandle(0);
}

void* process_get_handle(void)
{
	return g_process;
}

void* process_get_window(void)
{
	return g_window;
}

u32 process_get_id(void)
{
	return g_id;
}

b32 process_is_open(void)
{
	DWORD ec;
	return GetExitCodeProcess(g_process, &ec) && ec == STILL_ACTIVE;
}

b32 process_open(void)
{
	g_id = GetCurrentProcessId();
	g_process = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, g_id);
	if (g_process)
	{
		return true;
	}

	process_close();

	return false;
}

b32 process_read(u64 base, void* dst, u32 bytes)
{
	if (ReadProcessMemory(g_process, (void*)base, dst, bytes, 0) == FALSE)
	{
		memset(dst, 0, bytes);
		return false;
	}

	return true;
}

b32 process_read_str(u64 base, i8* dst, u32 bytes)
{
	if (process_read(base, dst, bytes) == false)
	{
		return false;
	}

	u32 end = bytes - 1;
	u32 i = 0;

	while (dst[i] && i < end)
	{
		++i;
	}

	while (i < end)
	{
		dst[i++] = 0;
	}
	
	dst[end] = 0;

	return true;
}

u64 process_scan(void const* src, i8 const* msk)
{
	u32 bytes = 0;
	while (msk[bytes])
	{
		++bytes;
	}

	u8 const *s1 = src;

	u64 addr = 0;
	MEMORY_BASIC_INFORMATION mbi;

	u8 *buf = malloc(128000000);
	if (buf == 0)
	{
		return 0;
	}

	while (VirtualQueryEx(g_process, (void*)addr, &mbi, sizeof(mbi)))
	{
		addr += mbi.RegionSize;

		if (mbi.RegionSize < bytes)
		{
			continue;
		}

		if (mbi.State != MEM_COMMIT)
		{
			continue;
		}

		if (mbi.Protect != PAGE_EXECUTE_READ)
		{
			continue;
		}

		u64 left = mbi.RegionSize;
		while (left > 0)
		{
			u64 size = left > 128000000 ? 128000000 : left;

			SIZE_T read;
			if (ReadProcessMemory(g_process, mbi.BaseAddress, buf, size, &read))
			{
				u64 end = read > bytes ? size - bytes : 0;

				for (u64 i = 0; i < end; ++i)
				{
					b32 match = true;

					for (u64 j = 0; j < bytes; ++j)
					{
						if (msk[j] == 'x' && buf[i + j] != s1[j])
						{
							match = false;
							break;
						}
					}

					if (match)
					{
						free(buf);
						return (u64)mbi.BaseAddress + i;
					}
				}
			}

			left = left - size;
		}
	}

	free(buf);

	return 0;
}

b32 process_write(u64 base, void const* src, u32 bytes)
{
	return WriteProcessMemory(g_process, (LPVOID)base, (LPVOID)src, bytes, 0);
}
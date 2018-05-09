#include "core/bind.h"
#include "core/config.h"
#include "core/string.h"
#include <Windows.h>

// Maximum number of keybinds.
#define MAX_BINDS 64

// Key-name mapping entry.
typedef struct KeyName
{
	i8* key;
	u32 vk;
} KeyName;

// Keybind state.
typedef struct State
{
	Bind bind;
	b32 down;
	b32 press;
} State;

// Key-name mapping table.
static KeyName CONFIG_KEYS[] = {
	{ "ADD",        VK_ADD },
	{ "BACKSLASH",	VK_SEPARATOR },
	{ "BACKSPACE",	VK_BACK },
	{ "CAPITAL",	VK_CAPITAL },
	{ "CAPS",		VK_CAPITAL },
	{ "CAPSLOCK",	VK_CAPITAL },
	{ "CLEAR",		VK_CLEAR },
	{ "COMMA",		VK_OEM_COMMA },
	{ "DECIMAL",	VK_DECIMAL },
	{ "DELETE",		VK_DELETE },
	{ "DIVIDE",		VK_DIVIDE },
	{ "DOWN",		VK_DOWN },
	{ "END",		VK_END },
	{ "ESC",		VK_ESCAPE },
	{ "ESCAPE",		VK_ESCAPE },
	{ "F1",			VK_F1 },
	{ "F10",		VK_F10 },
	{ "F11",		VK_F11 },
	{ "F12",		VK_F12 },
	{ "F2",			VK_F2 },
	{ "F3",			VK_F3 },
	{ "F4",			VK_F4 },
	{ "F5",			VK_F5 },
	{ "F6",			VK_F6 },
	{ "F7",			VK_F7 },
	{ "F8",			VK_F8 },
	{ "F9",			VK_F9 },
	{ "HOME",		VK_HOME },
	{ "INSERT",		VK_INSERT },
	{ "LBUTTON",	VK_LBUTTON },
	{ "LEFT",		VK_LEFT },
	{ "MBUTTON",	VK_MBUTTON },
	{ "MULTIPLY",	VK_MULTIPLY },
	{ "MINUS",		VK_OEM_MINUS },
	{ "NUM0",		VK_NUMPAD0 },
	{ "NUM1",		VK_NUMPAD1 },
	{ "NUM2",		VK_NUMPAD2 },
	{ "NUM3",		VK_NUMPAD3 },
	{ "NUM4",		VK_NUMPAD4 },
	{ "NUM5",		VK_NUMPAD5 },
	{ "NUM6",		VK_NUMPAD6 },
	{ "NUM7",		VK_NUMPAD7 },
	{ "NUM8",		VK_NUMPAD8 },
	{ "NUM9",		VK_NUMPAD9 },
	{ "NUMLOCK",	VK_NUMLOCK },
	{ "OEM1",		VK_OEM_1 },
	{ "OEM2",		VK_OEM_2 },
	{ "OEM3",		VK_OEM_3 },
	{ "OEM4",		VK_OEM_4 },
	{ "OEM5",		VK_OEM_5 },
	{ "OEM6",		VK_OEM_6 },
	{ "OEM7",		VK_OEM_7 },
	{ "OEM8",		VK_OEM_8 },
	{ "PAGEDOWN",	VK_NEXT },
	{ "PAGEUP",		VK_PRIOR },
	{ "PAUSE",		VK_PAUSE },
	{ "PERIOD",		VK_OEM_PERIOD },
	{ "PLUS",		VK_OEM_PLUS },
	{ "PRINT",		VK_PRINT },
	{ "RBUTTON",	VK_RBUTTON },
	{ "RETURN",		VK_RETURN },
	{ "RIGHT",		VK_RIGHT },
	{ "SCROLL",		VK_SCROLL },
	{ "SELECT",		VK_SELECT },
	{ "SEPARATOR",	VK_SEPARATOR },
	{ "SPACE",		VK_SPACE },
	{ "SUBTRACT",	VK_SUBTRACT },
	{ "TAB",		VK_TAB },
	{ "UP",			VK_UP },
	{ "XBUTTON1",	VK_XBUTTON1 },
	{ "XBUTTON2",	VK_XBUTTON2 },
};

static State g_state[MAX_BINDS];
static u32 g_num_binds;

static b32 window_is_key_down(u32 key)
{
	return GetAsyncKeyState(key) & 0x8000;
}

static b32 bind_check(Bind const* b)
{
	if (b->k)
	{
		return
			(b->c == 0 || window_is_key_down(VK_CONTROL)) &&
			(b->a == 0 || window_is_key_down(VK_MENU)) &&
			(b->s == 0 || window_is_key_down(VK_SHIFT)) &&
			window_is_key_down(b->k);
	}

	return
		(b->c && window_is_key_down(VK_CONTROL)) ||
		(b->a && window_is_key_down(VK_MENU)) ||
		(b->s && window_is_key_down(VK_SHIFT));
}

b32 bind_is_down(u32 b)
{
	if (b < MAX_BINDS)
	{
		return g_state[b].down;
	}

	return false;
}

b32 bind_is_press(u32 b)
{
	if (b < MAX_BINDS)
	{
		return g_state[b].press;
	}

	return false;
}

u32 bind_register(i8 const* key, i8 const* val)
{
	if (g_num_binds >= MAX_BINDS)
	{
		return MAX_BINDS;
	}

	i8 buf[64] = { 0 };
	config_get_str(buf, sizeof(buf), "Keys", key, val);
	string_upper(buf, ARRAYSIZE(buf), buf);

	Bind b = { 0 };

	i8* ctx = buf;
	for (i8* token = string_token(buf, '+', &ctx); token; token = string_token(NULL, '+', &ctx))
	{
		b32 found = false;

		if (string_compare(token, "ALT") == 0)
		{
			b.a = true;
		} else if (string_compare(token, "CTRL") == 0)
		{
			b.c = true;
		} else if (string_compare(token, "SHIFT") == 0)
		{
			b.s = true;
		} else
		{
			for (u32 i = 0; i < ARRAYSIZE(CONFIG_KEYS); ++i)
			{
				if (string_compare(token, CONFIG_KEYS[i].key) == 0)
				{
					b.k = CONFIG_KEYS[i].vk;
					found = true;
					break;
				}
			}
		}

		if (found == false && string_bytes(token) == 1)
		{
			b.k = token[0];
		}
	}

	u32 i = g_num_binds++;
	g_state[i].bind = b;

	return i;
}

void bind_update(void)
{
	u32 n = g_num_binds;
	for (u32 i = 0; i < n; ++i)
	{
		State* s = g_state + i;

		b32 down = bind_check(&s->bind);

		if (down != false && s->down == false)
		{
			s->press = true;
		} else
		{
			s->press = false;
		}

		s->down = down;
	}
}

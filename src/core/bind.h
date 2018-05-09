#pragma once
#include "core/types.h"

// Keybinding.
typedef struct Bind
{
	u32 k;
	u32 c;
	u32 s;
	u32 a;
} Bind;

// Returns true if the bind is down.
b32 bind_is_down(u32 b);

// Returns true if the bind is pressed.
b32 bind_is_press(u32 b);

// Registers a keybind. Returns the index of the bind.
u32 bind_register(i8 const* key, i8 const* val);

// Updates keybind state.
void bind_update(void);

#include "core/types.h"

// Returns an RGBA color in u32 format.
#define RGBA(r, g, b, a) (((u32)((u8)(r))) | ((u32)((u8)(g)) << 8) | ((u32)((u8)(b)) << 16) | ((u32)((u8)(a)) << 24))

// Common colors.
#define COLOR_WHITE RGBA(255, 255, 255, 255)
#define COLOR_GRAY RGBA(255, 255, 255, 150)
#define COLOR_BLACK RGBA(0, 0, 0, 255)
#define COLOR_PINK RGBA(255, 0, 255, 255)
#define COLOR_GREEN RGBA(0, 255, 0, 255)
#define COLOR_RED RGBA(255, 0, 0, 255)
#define COLOR_CYAN RGBA(0, 255, 255, 255) 

// Creates the renderer.
void renderer_create(void* device, void* context);

// Destroys the renderer.
void renderer_destroy(void);

// Draws an area directly from the renderer texture.
void renderer_draw_area(i32 dx, i32 dy, i32 dw, i32 dh, i32 sx, i32 sy, i32 sw, i32 sh, u32 color);

// Draws an ESP box.
void renderer_draw_esp(i32 x, i32 y, i32 w, i32 h, u32 color);

// Draws a health bar.
void renderer_draw_hp(i32 x, i32 y, i32 w, i32 h, u32 color, f32 hp);

// Draws a rectangle.
void renderer_draw_rect(i32 x, i32 y, i32 w, i32 h, u32 color);

// Draws text.
void renderer_draw_text(i32 x, i32 y, u32 color, i8 *src);

// Begins rendering a frame.
void renderer_frame_init(void* rtv, u32 res_x, u32 res_y);

// Finishes rendering a frame.
void renderer_frame_post(void);

// Draws a formatted string.
void renderer_printf(i32 x, i32 y, u32 color, i8 const * fmt, ...);

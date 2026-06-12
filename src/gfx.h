#ifndef GFX_H
#define GFX_H

#include <stdint.h>

/* Framebuffer geometry. 32 bits per pixel, XRGB8888 (0x00RRGGBB). */
#define FB_WIDTH  800
#define FB_HEIGHT 600

/* Build an XRGB8888 pixel from 8-bit components. */
#define RGB(r, g, b) \
    (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/* Base address of the framebuffer (used to configure ramfb). */
uint32_t *gfx_framebuffer(void);

void gfx_clear(uint32_t color);
void gfx_put_pixel(int x, int y, uint32_t color);
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_draw_rect(int x, int y, int w, int h, uint32_t color);
void gfx_vgradient(int x, int y, int w, int h, uint32_t top, uint32_t bottom);

/* Text rendering with the built-in 8x8 font. `scale` enlarges each glyph. */
void gfx_draw_char(int x, int y, char c, uint32_t fg, int scale);
void gfx_draw_string(int x, int y, const char *s, uint32_t fg, int scale);

#endif /* GFX_H */

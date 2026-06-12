/* gfx.c - software rendering into a linear XRGB8888 framebuffer. */

#include <stddef.h>

#include "gfx.h"
#include "font8x8.h"

/*
 * The framebuffer lives in BSS. ramfb reads it directly from guest RAM each
 * frame, so its link-time address is also the physical address we hand to
 * QEMU. 16-byte alignment keeps row accesses tidy.
 */
static uint32_t framebuffer[FB_WIDTH * FB_HEIGHT] __attribute__((aligned(16)));

uint32_t *gfx_framebuffer(void)
{
    return framebuffer;
}

void gfx_put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) {
        return;
    }
    framebuffer[(size_t)y * FB_WIDTH + (size_t)x] = color;
}

void gfx_clear(uint32_t color)
{
    for (size_t i = 0; i < (size_t)FB_WIDTH * FB_HEIGHT; ++i) {
        framebuffer[i] = color;
    }
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            gfx_put_pixel(x + dx, y + dy, color);
        }
    }
}

void gfx_draw_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int dx = 0; dx < w; ++dx) {
        gfx_put_pixel(x + dx, y, color);
        gfx_put_pixel(x + dx, y + h - 1, color);
    }
    for (int dy = 0; dy < h; ++dy) {
        gfx_put_pixel(x, y + dy, color);
        gfx_put_pixel(x + w - 1, y + dy, color);
    }
}

/* Linear top-to-bottom gradient between two colors. */
void gfx_vgradient(int x, int y, int w, int h, uint32_t top, uint32_t bottom)
{
    int tr = (int)((top >> 16) & 0xff), tg = (int)((top >> 8) & 0xff), tb = (int)(top & 0xff);
    int br = (int)((bottom >> 16) & 0xff), bg = (int)((bottom >> 8) & 0xff), bb = (int)(bottom & 0xff);

    for (int dy = 0; dy < h; ++dy) {
        int num = (h > 1) ? dy : 0;
        int den = (h > 1) ? (h - 1) : 1;
        int r = tr + (br - tr) * num / den;
        int g = tg + (bg - tg) * num / den;
        int b = tb + (bb - tb) * num / den;
        uint32_t color = RGB(r, g, b);
        for (int dx = 0; dx < w; ++dx) {
            gfx_put_pixel(x + dx, y + dy, color);
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint32_t fg, int scale)
{
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) {
        uc = '?';
    }
    if (scale < 1) {
        scale = 1;
    }
    const uint8_t *glyph = font8x8_basic[uc];
    for (int row = 0; row < 8; ++row) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; ++col) {
            if (bits & (1u << col)) {
                gfx_fill_rect(x + col * scale, y + row * scale, scale, scale, fg);
            }
        }
    }
}

void gfx_draw_string(int x, int y, const char *s, uint32_t fg, int scale)
{
    if (scale < 1) {
        scale = 1;
    }
    int cx = x;
    int cy = y;
    for (; *s != '\0'; ++s) {
        if (*s == '\n') {
            cx = x;
            cy += 8 * scale;
            continue;
        }
        gfx_draw_char(cx, cy, *s, fg, scale);
        cx += 8 * scale;
    }
}

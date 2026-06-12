/* kernel.c - Lost Eagle OS entry point.
 *
 * Brings up serial logging, configures the ramfb framebuffer, and renders a
 * simple "gaming system" boot screen with a bouncing sprite.
 */

#include <stdint.h>
#include <stdbool.h>

#include "uart.h"
#include "gfx.h"
#include "ramfb.h"

/* Background gradient endpoints (top -> bottom of the whole screen). */
#define BG_TOP    RGB(12, 16, 48)
#define BG_BOTTOM RGB(2, 2, 10)

/* Layout. */
#define TITLE_H 80
#define PLAY_X  40
#define PLAY_Y  104
#define PLAY_W  (FB_WIDTH - 80)
#define PLAY_H  (FB_HEIGHT - PLAY_Y - 96)
#define BARS_H  60
#define BARS_Y  (FB_HEIGHT - BARS_H - 24)

#define SPRITE  36

/* Crude busy-wait; there is no timer wired up yet. */
static void delay(volatile uint64_t count)
{
    while (count--) {
        __asm__ volatile("nop");
    }
}

/* Colour of the full-screen background gradient at an absolute row y. */
static uint32_t bg_at(int y)
{
    int tr = 12, tg = 16, tb = 48;
    int br = 2, bg = 2, bb = 10;
    int den = FB_HEIGHT - 1;
    int r = tr + (br - tr) * y / den;
    int g = tg + (bg - tg) * y / den;
    int b = tb + (bb - tb) * y / den;
    return RGB(r, g, b);
}

/* Repaint a rectangle of background gradient (used to erase the moving sprite). */
static void erase_bg(int x, int y, int w, int h)
{
    for (int dy = 0; dy < h; ++dy) {
        gfx_fill_rect(x, y + dy, w, 1, bg_at(y + dy));
    }
}

/* Classic SMPTE-style colour bars. */
static void draw_color_bars(int x, int y, int w, int h)
{
    static const uint32_t bars[7] = {
        RGB(255, 255, 255), RGB(255, 255, 0), RGB(0, 255, 255),
        RGB(0, 255, 0),     RGB(255, 0, 255), RGB(255, 0, 0),
        RGB(0, 0, 255),
    };
    int bw = w / 7;
    for (int i = 0; i < 7; ++i) {
        gfx_fill_rect(x + i * bw, y, bw, h, bars[i]);
    }
}

/* Draw the unchanging parts of the scene exactly once. */
static void draw_static_scene(void)
{
    gfx_vgradient(0, 0, FB_WIDTH, FB_HEIGHT, BG_TOP, BG_BOTTOM);

    /* Title bar. */
    gfx_fill_rect(0, 0, FB_WIDTH, TITLE_H, RGB(18, 22, 60));
    gfx_fill_rect(0, TITLE_H, FB_WIDTH, 3, RGB(255, 215, 0));
    gfx_draw_string(24, 14, "LOST EAGLE OS", RGB(255, 215, 0), 5);
    gfx_draw_string(26, 56, "RISC-V Gaming System", RGB(120, 200, 255), 2);

    /* Play-area frame. */
    gfx_draw_rect(PLAY_X, PLAY_Y, PLAY_W, PLAY_H, RGB(90, 100, 170));
    gfx_draw_rect(PLAY_X + 1, PLAY_Y + 1, PLAY_W - 2, PLAY_H - 2, RGB(50, 56, 110));

    gfx_draw_string(PLAY_X + 12, PLAY_Y + PLAY_H - 24,
                    "BOOT OK  -  FRAMEBUFFER LIVE", RGB(160, 255, 160), 2);

    /* Colour-bar test pattern. */
    draw_color_bars(0, BARS_Y, FB_WIDTH, BARS_H);
    gfx_draw_string(8, FB_HEIGHT - 18, "ramfb XRGB8888 800x600",
                    RGB(200, 200, 200), 2);
}

static void draw_sprite(int x, int y)
{
    gfx_fill_rect(x, y, SPRITE, SPRITE, RGB(255, 90, 90));
    gfx_draw_rect(x, y, SPRITE, SPRITE, RGB(255, 210, 210));
    gfx_fill_rect(x + 8, y + 8, 8, 8, RGB(255, 255, 255)); /* a little highlight */
}

void kmain(uint64_t hartid, uint64_t fdt)
{
    uart_init();
    uart_puts("\n=== Lost Eagle OS ===\n");
    uart_puts("RISC-V (rv64) S-mode kernel on QEMU 'virt'\n");
    uart_puts("boot hart : ");
    uart_put_dec(hartid);
    uart_puts("\nfdt addr  : ");
    uart_put_hex(fdt);
    uart_putc('\n');

    uint32_t *fb = gfx_framebuffer();
    uart_puts("fb addr   : ");
    uart_put_hex((uint64_t)(uintptr_t)fb);
    uart_putc('\n');

    if (ramfb_init(fb, FB_WIDTH, FB_HEIGHT)) {
        uart_puts("ramfb     : initialised (800x600 XRGB8888)\n");
    } else {
        uart_puts("ramfb     : NOT available (run QEMU with -device ramfb)\n");
    }

    draw_static_scene();

    /* Bouncing-sprite bounds, kept inside the play area and above the status
     * line so partial redraws never touch the static content. */
    int min_x = PLAY_X + 6;
    int max_x = PLAY_X + PLAY_W - 6 - SPRITE;
    int min_y = PLAY_Y + 6;
    int max_y = PLAY_Y + PLAY_H - 30 - SPRITE;

    int x = min_x + 40, y = min_y + 40;
    int prev_x = x, prev_y = y;
    int vx = 4, vy = 3;

    uart_puts("entering render loop\n");
    for (;;) {
        erase_bg(prev_x, prev_y, SPRITE, SPRITE);
        draw_sprite(x, y);
        prev_x = x;
        prev_y = y;

        x += vx;
        y += vy;
        if (x <= min_x || x >= max_x) {
            vx = -vx;
            x += vx;
        }
        if (y <= min_y || y >= max_y) {
            vy = -vy;
            y += vy;
        }

        delay(1500000);
    }
}

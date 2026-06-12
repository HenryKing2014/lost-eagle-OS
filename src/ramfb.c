/* ramfb.c - set up the QEMU ramfb display via fw_cfg. */

#include "ramfb.h"
#include "fw_cfg.h"

/* DRM fourcc for XRGB8888 ('X','R','2','4'). */
#define DRM_FORMAT_XRGB8888 0x34325258u

/* Layout of the "etc/ramfb" fw_cfg item. All fields are big-endian. */
struct ramfb_cfg {
    uint64_t addr;
    uint32_t fourcc;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
} __attribute__((packed));

bool ramfb_init(uint32_t *fb, uint32_t width, uint32_t height)
{
    if (!fw_cfg_probe()) {
        return false;
    }

    uint16_t select;
    uint32_t size;
    if (!fw_cfg_find_file("etc/ramfb", &select, &size)) {
        return false;
    }

    struct ramfb_cfg cfg;
    cfg.addr = __builtin_bswap64((uint64_t)(uintptr_t)fb);
    cfg.fourcc = __builtin_bswap32(DRM_FORMAT_XRGB8888);
    cfg.flags = __builtin_bswap32(0);
    cfg.width = __builtin_bswap32(width);
    cfg.height = __builtin_bswap32(height);
    cfg.stride = __builtin_bswap32(width * 4u);

    fw_cfg_dma_write(select, &cfg, sizeof(cfg));
    return true;
}

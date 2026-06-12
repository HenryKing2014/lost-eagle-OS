#ifndef RAMFB_H
#define RAMFB_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Configure QEMU's "ramfb" display device to scan out a framebuffer that
 * lives in guest RAM.  Requires `-device ramfb` on the QEMU command line.
 *
 * `fb` must point at width*height pixels of XRGB8888 storage.
 * Returns false if the ramfb device is not available.
 */
bool ramfb_init(uint32_t *fb, uint32_t width, uint32_t height);

#endif /* RAMFB_H */

#ifndef FW_CFG_H
#define FW_CFG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Driver for the QEMU fw_cfg device using the DMA interface.
 *
 * On the RISC-V 'virt' machine the fw_cfg registers live at 0x10100000.
 * fw_cfg lets the guest read configuration "files" published by QEMU and,
 * for some entries (such as "etc/ramfb"), write back to them.
 */

/* Initialise and verify the fw_cfg device. Returns false if not present. */
bool fw_cfg_probe(void);

/*
 * Look up a named file in the fw_cfg directory.
 * On success returns true and fills *select (the fw_cfg selector key) and
 * *size (file size in bytes).
 */
bool fw_cfg_find_file(const char *name, uint16_t *select, uint32_t *size);

/* DMA-write `len` bytes from `data` into the fw_cfg item `select`. */
void fw_cfg_dma_write(uint16_t select, const void *data, uint32_t len);

#endif /* FW_CFG_H */

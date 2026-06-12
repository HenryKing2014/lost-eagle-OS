/* fw_cfg.c - QEMU fw_cfg DMA driver for the RISC-V 'virt' machine. */

#include "fw_cfg.h"

#define FW_CFG_BASE    0x10100000UL
#define FW_CFG_REG_DMA (FW_CFG_BASE + 0x10) /* 64-bit, big-endian */

/* Control bits for the DMA access structure. */
#define FW_CFG_DMA_CTL_ERROR  0x01
#define FW_CFG_DMA_CTL_READ   0x02
#define FW_CFG_DMA_CTL_SKIP   0x04
#define FW_CFG_DMA_CTL_SELECT 0x08
#define FW_CFG_DMA_CTL_WRITE  0x10

/* Well-known fw_cfg selector keys. */
#define FW_CFG_SIGNATURE 0x0000
#define FW_CFG_FILE_DIR  0x0019

/* All multi-byte fields in these structures are big-endian. */
/* Natural layout already matches the wire format (no padding), so this does
 * not need to be packed - and keeping it unpacked gives an aligned `control`. */
struct fw_cfg_dma_access {
    uint32_t control;
    uint32_t length;
    uint64_t address;
};

struct fw_cfg_file {
    uint32_t size;     /* big-endian */
    uint16_t select;   /* big-endian */
    uint16_t reserved;
    char     name[56];
};

static volatile uint64_t *const fw_cfg_dma_reg =
    (volatile uint64_t *)FW_CFG_REG_DMA;

static bool str_eq(const char *a, const char *b)
{
    while (*a && (*a == *b)) {
        ++a;
        ++b;
    }
    return *a == *b;
}

/* Kick off one DMA access and spin until QEMU reports completion. */
static bool fw_cfg_dma_run(struct fw_cfg_dma_access *acc)
{
    /* QEMU updates acc->control from "outside" the C abstract machine, so the
     * field must be read through a volatile pointer or the compiler will hoist
     * it out of the loop and spin forever. */
    volatile uint32_t *control = &acc->control;
    __sync_synchronize();
    *fw_cfg_dma_reg = __builtin_bswap64((uint64_t)(uintptr_t)acc);
    for (;;) {
        uint32_t ctl = *control; /* field is stored big-endian */
        if (ctl == 0) {
            return true; /* completed successfully */
        }
        if (__builtin_bswap32(ctl) & FW_CFG_DMA_CTL_ERROR) {
            return false;
        }
    }
}

/*
 * Read `len` bytes of the currently selected item into `buf`. When
 * `do_select` is true the item `select` is selected first (which also
 * rewinds it to offset 0); otherwise reading continues from the current
 * offset of the previously selected item.
 */
static bool fw_cfg_read(uint16_t select, void *buf, uint32_t len, bool do_select)
{
    struct fw_cfg_dma_access acc;
    uint32_t ctl = FW_CFG_DMA_CTL_READ;
    if (do_select) {
        ctl |= FW_CFG_DMA_CTL_SELECT | ((uint32_t)select << 16);
    }
    acc.control = __builtin_bswap32(ctl);
    acc.length = __builtin_bswap32(len);
    acc.address = __builtin_bswap64((uint64_t)(uintptr_t)buf);
    return fw_cfg_dma_run(&acc);
}

bool fw_cfg_probe(void)
{
    char sig[4] = {0};
    if (!fw_cfg_read(FW_CFG_SIGNATURE, sig, sizeof(sig), true)) {
        return false;
    }
    return sig[0] == 'Q' && sig[1] == 'E' && sig[2] == 'M' && sig[3] == 'U';
}

bool fw_cfg_find_file(const char *name, uint16_t *select, uint32_t *size)
{
    uint32_t count_be = 0;
    if (!fw_cfg_read(FW_CFG_FILE_DIR, &count_be, sizeof(count_be), true)) {
        return false;
    }
    uint32_t count = __builtin_bswap32(count_be);

    for (uint32_t i = 0; i < count; ++i) {
        struct fw_cfg_file entry = {0};
        if (!fw_cfg_read(0, &entry, sizeof(entry), false)) {
            return false;
        }
        if (str_eq(entry.name, name)) {
            *select = __builtin_bswap16(entry.select);
            *size = __builtin_bswap32(entry.size);
            return true;
        }
    }
    return false;
}

void fw_cfg_dma_write(uint16_t select, const void *data, uint32_t len)
{
    struct fw_cfg_dma_access acc;
    uint32_t ctl = FW_CFG_DMA_CTL_WRITE | FW_CFG_DMA_CTL_SELECT |
                   ((uint32_t)select << 16);
    acc.control = __builtin_bswap32(ctl);
    acc.length = __builtin_bswap32(len);
    acc.address = __builtin_bswap64((uint64_t)(uintptr_t)data);
    (void)fw_cfg_dma_run(&acc);
}

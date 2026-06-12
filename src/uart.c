/* uart.c - minimal driver for the NS16550A UART on the QEMU 'virt' machine. */

#include "uart.h"

#define UART0_BASE 0x10000000UL

/* 16550 register offsets (each register is one byte wide here). */
#define UART_THR 0 /* Transmit Holding Register (write, DLAB=0) */
#define UART_RHR 0 /* Receive Holding Register  (read,  DLAB=0) */
#define UART_DLL 0 /* Divisor Latch Low         (DLAB=1) */
#define UART_IER 1 /* Interrupt Enable Register (DLAB=0) */
#define UART_DLM 1 /* Divisor Latch High        (DLAB=1) */
#define UART_FCR 2 /* FIFO Control Register (write) */
#define UART_LCR 3 /* Line Control Register */
#define UART_LSR 5 /* Line Status Register */

#define LCR_DLAB 0x80 /* Divisor latch access bit */
#define LCR_8N1  0x03 /* 8 data bits, no parity, 1 stop bit */
#define FCR_ENABLE_FIFO 0x01
#define FCR_CLEAR_FIFOS 0x06
#define LSR_THR_EMPTY 0x20 /* Transmit holding register empty */

static volatile uint8_t *const uart = (volatile uint8_t *)UART0_BASE;

void uart_init(void)
{
    uart[UART_IER] = 0x00;                 /* disable interrupts */
    uart[UART_LCR] = LCR_DLAB;             /* enable divisor latch */
    uart[UART_DLL] = 0x03;                 /* 38400 baud (QEMU ignores rate) */
    uart[UART_DLM] = 0x00;
    uart[UART_LCR] = LCR_8N1;              /* 8N1, latch off */
    uart[UART_FCR] = FCR_ENABLE_FIFO | FCR_CLEAR_FIFOS;
}

void uart_putc(char c)
{
    while ((uart[UART_LSR] & LSR_THR_EMPTY) == 0) {
        /* wait until the transmit holding register is empty */
    }
    uart[UART_THR] = (uint8_t)c;
}

void uart_puts(const char *s)
{
    for (; *s != '\0'; ++s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s);
    }
}

void uart_put_hex(uint64_t value)
{
    static const char digits[] = "0123456789abcdef";
    uart_puts("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uart_putc(digits[(value >> shift) & 0xf]);
    }
}

void uart_put_dec(uint64_t value)
{
    char buf[21];
    int i = 0;
    if (value == 0) {
        uart_putc('0');
        return;
    }
    while (value > 0 && i < (int)sizeof(buf)) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i > 0) {
        uart_putc(buf[--i]);
    }
}

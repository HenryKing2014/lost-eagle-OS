#ifndef UART_H
#define UART_H

#include <stdint.h>

/* NS16550A UART on the QEMU 'virt' machine, mapped at 0x10000000. */
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_hex(uint64_t value);
void uart_put_dec(uint64_t value);

#endif /* UART_H */

#ifndef uart_defs_h
#define uart_defs_h

#include <stdint.h>

// UART register bits
#define UART_LCR_DLAB   (0x80)
#define UART_LCR_NOPAR  (0x00)
#define UART_LCR_1STOP  (0x00)
#define UART_LCR_8BITS  (0x03)
#define UART_IER_EI     (0x07)
#define UART_FCR_EN     (0x01)
#define UART_FCR_CLR    (0x06)

// Interrupt controller contants
#define VIC_UART0     (0x00000040)
#define VIC_UART1     (0x00000080)
#define VIC_Channel_UART0       (6)
#define VIC_Channel_UART1       (7)

// Constants to setup and access the VIC
#define serINVALID_QUEUE  ((xQueueHandle) 0)
#define serNO_BLOCK       ((portTickType) 0)

#endif // ifndef uart_defs_h

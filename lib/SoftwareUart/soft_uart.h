#ifndef SOFT_UART_H_
#define SOFT_UART_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"

#define SOFT_RX_BUFFER_SIZE 500
#define RX_PIN PD2 // INT0 pin (External interrupt)

void soft_uart_init(uint32_t baud);
char soft_uart_read(void);
uint16_t soft_uart_read_count(void);

#endif /* SOFT_UART_H_ */
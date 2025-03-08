#ifndef UART_HAL_H_
#define UART_HAL_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"

#define RX_BUFFER_SIZE 128

void uart_init(uint32_t baud, uint8_t high_speed);
void uart_send_byte(char c);
void uart_send_array(char *c, uint16_t len);
void uart_send_string(char *c);
uint16_t uart_read_count(void);
uint8_t uart_read(void);

#endif /* UART_HAL_H_ */
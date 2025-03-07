#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#include "config.h"

extern "C"
{
#include <uart_hal.h>
}

int main(void)
{
    uint8_t start[] = "Program Start\n\r";

    uint8_t inputString[200];
    uint8_t cur_pointer = 0;
    uint8_t data;

    DDRD |= 0xF0; // 0b11110000
    uart_init(9600, 0);

    sei();
    uart_send_string(start);

    while (1)
    {
        if (uart_read_count() > 0)
        {
            data = uart_read();
            if (data == 'j')
            {
                inputString[cur_pointer] = '\0';
                uart_send_string(inputString);
                cur_pointer = 0;
            }
            else
            {
                inputString[cur_pointer] = data;
                cur_pointer += 1;
            }
        }
    }
}
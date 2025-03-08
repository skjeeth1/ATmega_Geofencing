#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"

#include <uart_hal.h>

uint8_t receiveDataComplete = false;

void get_uart_data(char *);

int main(void)
{
    char start[] = "Program Start\n\r";

    char inputString[RX_BUFFER_SIZE];

    DDRD |= 0xF0; // 0b11110000
    uart_init(9600, 0);

    sei();
    uart_send_string(start);

    char signal[] = "$GPGLL";
    char signal_header[6];

    while (true)
    {
        get_uart_data(inputString);
        _delay_ms(1000);

        if (receiveDataComplete)
        {
            strncpy(signal_header, inputString, 6);
            signal_header[6] = '\0';
            if (!strcmp(signal_header, signal))
            {
                uart_send_string(inputString);
            }
        }
    }
}

void get_uart_data(char *inputString)
{
    static uint8_t cur_pointer = 0;
    char data;

    receiveDataComplete = false;

    while (uart_read_count() > 0)
    {
        data = uart_read();
        if (data == '\n')
        {
            inputString[cur_pointer] = '\0';
            receiveDataComplete = true;
            cur_pointer = 0;
        }
        else
        {
            inputString[cur_pointer] = data;
            cur_pointer += 1;
        }
    }
}
#include "soft_uart.h"

volatile uint8_t receiving = 0;
volatile uint8_t rx_byte_shift = 0;
volatile uint8_t rx_byte_count = 0;
volatile char rx_byte_data = 0;
volatile uint8_t rx_ready = 0;

volatile static char soft_rx_buffer[SOFT_RX_BUFFER_SIZE] = {0};
volatile static uint16_t soft_rx_count = 0;

ISR(INT0_vect)
{
    if (!receiving)
    {
        receiving = 1;
        rx_byte_count = 0;
        rx_byte_shift = 0;
        TCNT0 = 0;               // Reset Timer0
        TIMSK0 |= (1 << OCIE0A); // Enable Timer0 Compare Match Interrupt
    }
}

ISR(TIMER0_COMPA_vect)
{
    // static uint8_t tx_shift = 0;
    volatile static uint16_t rx_write_pos = 0;

    // RX Handling
    if (receiving)
    {
        if (rx_byte_count == 8)
        {                                      // Stop bit time
            rx_byte_data = rx_byte_shift >> 1; // Store the final received byte
            rx_ready = 1;
            receiving = 0;
            rx_byte_count = 0;
            TIMSK0 &= ~(1 << OCIE0A); // Disable Timer0 interrupt after reception

            soft_rx_buffer[rx_write_pos] = rx_byte_data;
            soft_rx_count++;
            rx_write_pos++;
            if (rx_write_pos >= SOFT_RX_BUFFER_SIZE)
            {
                rx_write_pos = 0;
            }
            return;
        }

        // Shift in RX bits
        if (PIND & (1 << RX_PIN))
        {
            rx_byte_shift |= (1 << rx_byte_count);
        }
        rx_byte_count++;
    }
}

void soft_uart_init(uint32_t baud)
{
    uint8_t prescaler = 8;

    TCCR0A = (1 << WGM01);                               // CTC mode
    TCCR0B = (1 << CS01);                                // Prescaler 8
    OCR0A = (uint8_t)((F_CPU / (baud * prescaler)) - 1); // Set compare match value

    sei();

    EICRA |= (1 << ISC01); // Falling edge on INT0
    EIMSK |= (1 << INT0);  // Enable INT0
}

uint16_t soft_uart_read_count(void)
{
    return soft_rx_count;
}

char soft_uart_read()
{
    static uint16_t rx_read_pos = 0;
    char data = 0;

    data = soft_rx_buffer[rx_read_pos];
    rx_read_pos++;
    soft_rx_count--;
    if (rx_read_pos >= SOFT_RX_BUFFER_SIZE)
    {
        rx_read_pos = 0;
    }
    return data;
}

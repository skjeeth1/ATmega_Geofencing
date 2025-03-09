#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>

#include <SoftwareSerial.h>
#include <Arduino.h>

#include "config.h"

extern "C"
{
#include <uart_hal.h>
}

uint8_t receiveDataComplete = false;

SoftwareSerial gps_serial(6, 5);
char receivedString[200];    // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

void get_uart_data(char *);
void getGPSdata();

int main(void)
{
    char start[] = "Program Start\n\r";

    char inputString[RX_BUFFER_SIZE];

    DDRD |= 0xF0; // 0b11110000
    uart_init(9600, 0);

    pinMode(6, INPUT);
    pinMode(5, OUTPUT);
    gps_serial.begin(9600);

    sei();
    uart_send_string(start);

    char signal[] = "$GPGLL";
    char signal_header[6];

    char LAT[11];
    char LON[11];

    while (true)
    {
        get_uart_data(inputString);
        getGPSdata();

        if (receiveDataComplete)
        {
            // strncpy(signal_header, inputString, 6);
            // signal_header[6] = '\0';
            // if (!strcmp(signal_header, signal))
            // {
            //     uart_send_string(inputString);
            // }
        }

        if (stringComplete)
        {
            strncpy(signal_header, receivedString, 6);
            signal_header[6] = '\0';
            if (!strcmp(signal_header, signal))
            {
                strncpy(LAT, receivedString + 7, 10);
                LAT[11] = '\0';
                // int LATperiod = LAT.indexOf('.');
                // int LATzero = LAT.indexOf('0');
                // if (LATzero == 0)
                // {
                //     LAT = LAT.substring(1);
                // }

                strncpy(LON, receivedString + 20, 10);
                LON[11] = '\0';
                // String LON = receivedString.substring(20, 31);
                // int LONperiod = LON.indexOf('.');
                // int LONTzero = LON.indexOf('0');
                // if (LONTzero == 0)
                // {
                //     LON = LON.substring(1);
                // }`

                // Serial.println(LAT);
                // Serial.println(LON);

                // location.x = LAT.toDouble();
                // location.y = LON.toDouble();

                uart_send_string(LAT);
                uart_send_byte('\n');
                uart_send_string(LON);
                uart_send_byte('\n');
            }
        }

        _delay_ms(1000);
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

void getGPSdata()
{
    stringComplete = false;
    static uint8_t cur_pointer1 = 0;
    while (gps_serial.available())
    {
        // get the new byte:
        char inChar = (char)gps_serial.read();

        if (inChar == '\n')
        {
            receivedString[cur_pointer1] = '\0';
            stringComplete = true;
            cur_pointer1 = 0;
        }
        else
        {
            receivedString[cur_pointer1] = inChar;
            cur_pointer1 += 1;
        }
    }
}
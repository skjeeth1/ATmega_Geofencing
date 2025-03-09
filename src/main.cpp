#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// #include <SoftwareSerial.h>
// #include <Arduino.h>

#include "config.h"

extern "C"
{
#include <uart_hal.h>
#include <soft_uart.h>
#include <ray_cast.h>
}

struct Point
{
    double lat, lon;
};

void get_uart_data(char *);
void getGPSdata();
bool geofence(Point, Point, int);

char SerialData[RX_BUFFER_SIZE];
bool receiveDataComplete = false;

char GPSdata[RX_BUFFER_SIZE]; // a String to hold incoming data
bool receivedGPSdata = false; // whether the string is complete

int main(void)
{
    char start[] = "Program Start\n\r";

    DDRD |= 0xF0; // 0b11110000
    uart_init(9600, 0);

    DDRD &= ~(1 << RX_PIN);
    PORTD |= (1 << RX_PIN); // Enable pull-up for RX

    soft_uart_init(9600);

    sei();
    uart_send_string(start);

    char signal[] = "$GPGLL";
    char signal_header[6];

    char LAT[13];
    char LON[13];

    Point cur_location = {0, 0};
    Point origin = {0, 0};

    while (true)
    {
        // get_uart_data(SerialData);
        getGPSdata();

        if (receivedGPSdata)
        {
            strncpy(signal_header, GPSdata, 6);
            signal_header[6] = '\0';
            if (!strcmp(signal_header, signal))
            {
                strncpy(LAT, GPSdata + 7, 12);
                LAT[12] = '\0';

                strncpy(LON, GPSdata + 22, 12);
                LON[12] = '\0';

                uart_send_string(LAT);
                uart_send_byte('\n');
                uart_send_string(LON);
                uart_send_byte('\n');

                cur_location.lat = strtod(LAT, NULL);
                cur_location.lon = strtod(LON, NULL);

                bool status = geofence(cur_location, origin, 500);
                uart_send_byte(status ? 'y' : 'n');
            }
        }

        _delay_ms(1000);
    }
}

void get_uart_data(char *inputString)
{
    static uint16_t cur_pointer = 0;
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

bool geofence(Point gps, Point origin, int distance)
// Function to check if the GPS location is within the geofence
{
    double x = gps.lat - origin.lat;
    double y = gps.lon - origin.lon;
    double distanceSquared = x * x + y * y;
    return distanceSquared <= distance * distance;
}

void getGPSdata()
{
    static uint16_t cur_pointer1 = 0;
    char data;

    receivedGPSdata = false;

    while (soft_uart_read_count() > 0)
    {
        data = soft_uart_read();
        if (data == '\n')
        {
            GPSdata[cur_pointer1] = '\0';
            receivedGPSdata = true;
            cur_pointer1 = 0;
        }
        else
        {
            GPSdata[cur_pointer1] = data;
            cur_pointer1 += 1;
        }
    }
}
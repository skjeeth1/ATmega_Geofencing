#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#include <uart_hal.h>
#include <soft_uart.h>
// #include <ray_cast.h>

typedef struct
{
    volatile unsigned long int x;
    volatile unsigned long int y;
} vec;

void get_uart_data(char *);
void getGPSdata();
uint8_t geofence(vec, vec, double);
int SIMInit(void);
void error(void);
void receiveSMS();
void parseMessage(char *message);
void set_origin();
void set_distance(int distance);
void send_message(char *message);
void outside_check();
void inside_check();
vec vsub(vec a, vec b);
double vmag(vec a);

#define LED_PIN PB0

#define LED_PINo PD5
#define LED_PINi PD6

#define PHONE_NUMBER "+916282591266"

char SerialData[RX_BUFFER_SIZE];
bool receiveDataComplete = false;

char GPSdata[RX_BUFFER_SIZE]; // a String to hold incoming data
bool receivedGPSdata = false; // whether the string is complete

vec cur_location = {50.0, 50.0};
// vec origin = {0832.7088, 7654.2633};
vec origin = {0, 0};

int geofence_dist = 100;

int main(void)
{
    char start[] = "Program Start\n\r";

    DDRD |= 0xF0; // 0b11110000

    uart_init(9600, 0);
    soft_uart_init(9600);

    DDRD &= ~(1 << RX_PIN);
    PORTD |= (1 << RX_PIN); // Enable pull-up for RX

    DDRD |= (1 << PD1);
    DDRD &= ~(1 << PD0);
    // PORTD |= (1 << RX_PIN); // Enable pull-up for RX

    DDRB |= (1 << LED_PIN);
    PORTB &= ~(1 << LED_PIN);

    DDRD |= (1 << LED_PINo);
    PORTD &= ~(1 << LED_PINo);

    DDRD |= (1 << LED_PINi);
    PORTD &= ~(1 << LED_PINi);

    sei();
    uart_send_string(start);

    char signal[] = "$GPGLL";
    char signal_header[6];

    volatile char LAT[6];
    volatile char LON[6];

    volatile char LAT1[6];
    volatile char LON1[6];

    volatile uint8_t inside = 1;
    volatile uint8_t status = 0;

    // vec vsq[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}, {2.5, 2.5}, {7.5, 0.1}, {7.5, 7.5}, {2.5, 7.5}};

    // polygon_t sq = {4, vsq};      /* outer square */
    // polygon_t sq_hole = {8, vsq}; /* outer and inner square, ie hole */

    // vec b = {15, 15};
    // vec c = {10, 5}; /* on edge */
    // vec d = {5, 5};

    // int SIMstatus = SIMInit();
    // if (SIMstatus)
    // {
    //     return 1;
    // }

    while (1)
    {
        getGPSdata();
        receiveSMS();

        if (receivedGPSdata)
        {
            // error();
            strncpy(signal_header, GPSdata, 6);
            signal_header[6] = '\0';
            if (!strcmp(signal_header, signal))
            {
                strncpy(LAT, GPSdata + 7, 5);
                LAT[5] = '\0';

                strncpy(LON, GPSdata + 21, 5);
                LON[5] = '\0';

                uart_send_string(LAT);
                uart_send_byte('\n');
                uart_send_string(LON);
                uart_send_byte('\n');

                cur_location.x = strtol(LAT, NULL, 10);
                cur_location.y = strtol(LON, NULL, 10);

                sprintf(LAT1, "%ld\n", cur_location.x);
                sprintf(LON1, "%ld\n", cur_location.y);

                uart_send_string(LAT1);
                uart_send_string(LON1);

                status = geofence(cur_location, origin, geofence_dist);
                if (status != inside)
                {
                    char mess[] = "Device has left the compound!";
                    if (status == true)
                        inside_check();
                    else
                        outside_check();
                    send_message(mess);
                }

                inside = status;
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

uint8_t geofence(vec gps, vec origin, double distance)
// Function to check if the GPS location is within the geofence
{
    vec dist = vsub(gps, origin);
    return distance > vmag(dist);
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

int SIMInit()
{
    char receive[10];
    char success[] = "OK";

    uart_send_string("AT\n");

    while (1)
    {
        get_uart_data(receive);

        if (receiveDataComplete)
        {
            if (strcmp(receive, success))
            {
                error();
                return 1;
            }
            break;
        }
    }

    uart_send_string("AT+CMGF=1\n"); // Configure Text Mode
    _delay_ms(1000);
    uart_send_string("AT+CMGS=\"+916282591266\"\n");
    _delay_ms(1000);
    uart_send_string("Device is Armed!");
    _delay_ms(1000);
    uart_send_byte(26); // Terminate char
    _delay_ms(1000);

    uart_send_string("AT+CNMI=1,2,0,0,0\n");
    _delay_ms(1000);

    return 0;
}

void receiveSMS()
{
    char receivedData[RX_BUFFER_SIZE];
    get_uart_data(receivedData);

    char *num_start;
    char sender[20], message[160];

    if (receiveDataComplete)
    {
        uart_send_string("RECEIVED MESSAGE: ");
        uart_send_string(receivedData);
        uart_send_byte('\n');
        // Find the phone number in the response
        num_start = strchr(receivedData, '"'); // First quote
        if (!num_start)
            return;
        num_start++; // Move past first quote

        char *num_end = strchr(num_start, '"'); // Find closing quote
        if (!num_end)
            return;

        // Extract phone number
        strncpy(sender, num_start, num_end - num_start);
        sender[num_end - num_start] = '\0'; // Null-terminate

        uart_send_string("SENDER: ");
        uart_send_string(sender);
        uart_send_byte('\n');
        if (strcmp(sender, PHONE_NUMBER) != 0)
            return; // Ignore unknown number
    }
    else
    {
        return;
    }

    while (1)
    {
        get_uart_data(message);

        if (receiveDataComplete)
        {
            uart_send_string("MESSAGE: ");
            uart_send_string(message);
            uart_send_byte('\n');
            parseMessage(message);
            break;
        }
    }
}

void parseMessage(char *message)
{
    if (strcmp(message, "SET_ORIGIN") == 0)
    {
        set_origin();
    }
    else if (strncmp(message, "SET_DISTANCE ", 13) == 0)
    {
        int new_distance = strtod(message + 13, NULL); // Extract integer after "SET_DISTANCE "
        set_distance(new_distance);
    }
}

void set_origin()
{
    uart_send_string("SET ORIGIN!");
    uart_send_byte('\n');
    origin.x = cur_location.x;
    origin.y = cur_location.y;

    char blah[10];
    sprintf(blah, "%d\n", (int)origin.x);
    uart_send_string(blah);
    sprintf(blah, "%d\n", (int)origin.y);
    uart_send_string(blah);
}

void set_distance(int distance)
{
    uart_send_string("SET DISTANCE!\n");
    uart_send_string("PREVIOUS DISTANCE: ");

    char blah[10];
    sprintf(blah, "%d", (int)geofence_dist);
    uart_send_string(blah);
    uart_send_byte('\n');

    geofence_dist = distance;

    uart_send_string("NEW DISTANCE: ");
    sprintf(blah, "%d", (int)geofence_dist);
    uart_send_string(blah);
    uart_send_byte('\n');
}

void send_message(char *message)
{
    uart_send_string("AT+CMGS=\"+916282591266\"\n");
    _delay_ms(1000);
    uart_send_string(message);
    _delay_ms(1000);
    uart_send_byte(26);
}

void error()
{
    PORTB |= (1 << LED_PIN);
    _delay_ms(1000);
    PORTB &= ~(1 << LED_PIN);
}

void outside_check()
{
    PORTD |= (1 << LED_PINo);
    _delay_ms(1000);
    PORTD &= ~(1 << LED_PINo);
}

void inside_check()
{
    PORTD |= (1 << LED_PINi);
    _delay_ms(1000);
    PORTD &= ~(1 << LED_PINi);
}

vec vsub(vec a, vec b)
{
    vec c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    return c;
}

double vmag(vec a)
{
    double sum = a.x * a.x + a.y * a.y;
    return sqrt(sum);
}
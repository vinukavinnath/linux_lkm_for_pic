/*
 * File:   main.c
 * Author: vinuka vinnath
 *
 * Created on March 26, 2025, 9:34 PM
 */

// LED Blinking Firmware for PIC18F4550
// Configured for serial communication and LED control

#include <xc.h>
#include <stdint.h>

// Configuration Bits
#pragma config FOSC = XT_XT     // Crystal oscillator 
#pragma config PWRT = OFF       // Power-up Timer disabled
#pragma config BOR = OFF        // Brown-out Reset disabled
#pragma config WDT = OFF        // Watchdog Timer disabled
#pragma config LVP = OFF        // Low Voltage Programming disabled
#pragma config MCLRE = ON       // MCLR pin enabled

// Define LED pin (assuming connected to RC0)
#define LED_PIN   PORTCbits.RC0

// Function Prototypes
void initialize(void);
void delay_ms(unsigned int ms);
void processSerialData(void);

void main(void) {
    // Initialize microcontroller
    initialize();
    
    while(1) {
        // Wait for serial data and process
        processSerialData();
    }
}

// Initialize microcontroller peripherals
void initialize(void) {
    // Configure port directions
    TRISC = 0x00;    // Set PORTC as output for LED
    PORTC = 0x00;    // Clear PORTC

    // Configure UART for serial communication
    // Assuming 9600 baud rate, 8-bit, no parity
    TXSTAbits.SYNC = 0;   // Asynchronous mode
    RCSTAbits.SPEN = 1;   // Serial port enabled
    TXSTAbits.TXEN = 1;   // Transmit enabled
    RCSTAbits.CREN = 1;   // Receiver enabled

    // Baud rate calculation for 9600 at 8MHz
    SPBRG = 12;  // Formula: (Fosc / (16 * Baud Rate)) - 1
}

// Simple delay function
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 123; j++);  // Approximate delay
}

// Process incoming serial data
void processSerialData(void) {
    // Check if data is available
    if(PIR1bits.RCIF) {
        uint8_t receivedData = RCREG;
        
        // Simple protocol: 
        // '1' turns LED on
        // '0' turns LED off
        switch(receivedData) {
            case '1':
                LED_PIN = 1;  // Turn LED on
                break;
            case '0':
                LED_PIN = 0;  // Turn LED off
                break;
        }
    }
}
/*
 * PIC16F887 LED Control Firmware
 * Target: easyPIC7 development board
 * Compiler: XC8
 * 
 * This firmware controls an LED on PORTC through UART commands.
 * When "1" is received, the LED turns ON.
 * When "0" is received, the LED turns OFF.
 */

// Configuration bits
#pragma config FOSC = HS        // High-Speed Crystal/Resonator
#pragma config WDTE = OFF       // Watchdog Timer disabled
#pragma config PWRTE = ON       // Power-up Timer enabled
#pragma config MCLRE = ON       // MCLR pin enabled
#pragma config CP = OFF         // Program memory code protection disabled
#pragma config CPD = OFF        // Data memory code protection disabled
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config IESO = ON        // Internal/External Switchover enabled
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor enabled
#pragma config LVP = OFF        // Low-Voltage Programming disabled

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// Define clock frequency - adjust according to your crystal
#define _XTAL_FREQ 8000000  // 8MHz crystal

// Define LED pin
#define LED_PIN RC0          // LED connected to RC0 (adjust as needed)
// But LED must be connected D0 pin for proper functionality

// UART functions
void UART_Init(void);
void UART_Tx(uint8_t data);
uint8_t UART_Rx(void);
bool UART_DataReady(void);

// Main function
void main(void)
{
    // Set up PORTC for output (LED pin)
    TRISC &= ~(1 << 0);  // Set RC0 as output
    PORTC &= ~(1 << 0);  // Initialize LED as OFF
    
    // Initialize UART
    UART_Init();
    
    // Main loop
    while(1)
    {
        // Check if data received
        if(UART_DataReady())
        {
            uint8_t cmd = UART_Rx();
            
            // Process command
            if(cmd == '1')
            {
                PORTC |= (1 << 0);    // Turn LED ON
                UART_Tx('1');         // Acknowledge
            }
            else if(cmd == '0')
            {
                PORTC &= ~(1 << 0);   // Turn LED OFF
                UART_Tx('0');         // Acknowledge
            }
        }
    }
}

// Initialize UART at 9600 baud
void UART_Init(void)
{
    // Configure UART transmitter
    TXSTAbits.TXEN = 1;    // Enable transmitter
    TXSTAbits.SYNC = 0;    // Asynchronous mode
    TXSTAbits.BRGH = 1;    // High-speed baud rate
    
    // Configure UART receiver
    RCSTAbits.SPEN = 1;    // Enable serial port
    RCSTAbits.CREN = 1;    // Enable continuous receive
    
    // Set baud rate to 9600 for 8MHz crystal
    // SPBRG = ((_XTAL_FREQ/16)/baud_rate) - 1
    SPBRG = 51;            // For 9600 baud with 8MHz crystal
    
    // Configure pins - these are usually TX on RC6 and RX on RC7
    TRISCbits.TRISC6 = 0;  // TX pin as output
    TRISCbits.TRISC7 = 1;  // RX pin as input
}

// Transmit a byte via UART
void UART_Tx(uint8_t data)
{
    while(!TXSTAbits.TRMT);  // Wait for transmit buffer to be empty
    TXREG = data;            // Send data
}

// Check if UART data is ready to be read
bool UART_DataReady(void)
{
    return PIR1bits.RCIF;   // Return RCIF flag status
}

// Receive a byte via UART
uint8_t UART_Rx(void)
{
    while(!PIR1bits.RCIF);  // Wait until data is available
    
    // Check for errors
    if(RCSTAbits.OERR)
    {
        // Handle overrun error
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    
    return RCREG;  // Return received data
}
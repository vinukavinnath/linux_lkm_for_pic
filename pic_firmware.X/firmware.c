
/*
 * PIC18F4550 Firmware for LED Control
 * Communication via UART
 */

#include <xc.h>

// Configuration bits
#pragma config FOSC = HS        // External crystal oscillator
#pragma config PWRT = ON        // Power-up timer enabled
#pragma config BOR = ON         // Brown-out reset enabled
#pragma config WDT = OFF        // Watchdog timer disabled
#pragma config PBADEN = OFF     // PORTB<4:0> pins are digital I/O
#pragma config LVP = OFF        // Low-voltage programming disabled
#pragma config MCLRE = ON       // MCLR pin enabled

// Define CPU Frequency (adjust as needed)
#define _XTAL_FREQ 20000000     // 20 MHz crystal

// Commands
#define LED_ON_CMD 0x01
#define LED_OFF_CMD 0x00
#define BLINK_CMD 0x02

// Function prototypes
void initialize(void);
void process_command(unsigned char cmd);
void initialize_uart(void);
unsigned char uart_receive(void);
void uart_transmit(unsigned char data);

// Global variables
unsigned char blinking = 0;

void main(void) {
    unsigned char command;
    
    // Initialize hardware
    initialize();
    
    // Set RB0 as output for LED
    TRISBbits.TRISB0 = 0;
    
    // Initially turn off LED
    LATBbits.LATB0 = 0;
    
    // Main loop
    while(1) {
        // Check if data is available via UART
        if(PIR1bits.RCIF) {
            command = uart_receive();
            process_command(command);
        }
        
        // Handle blinking if enabled
        if(blinking) {
            LATBbits.LATB0 = !LATBbits.LATB0;  // Toggle LED
            __delay_ms(500);                   // 500ms delay
        }
    }
}

void initialize(void) {
    // Initialize UART
    initialize_uart();
    
    // Enable global interrupts
    INTCONbits.GIE = 1;
    
    // Enable peripheral interrupts
    INTCONbits.PEIE = 1;
}

void initialize_uart(void) {
    // Configure UART for 9600 baud rate (for 20MHz crystal)
    // BRGH=1, BRG16=1, SPBRG=520 for 9600 baud
    
    // Configure RX and TX pins
    TRISCbits.TRISC6 = 1;    // RC6 (TX) is input initially
    TRISCbits.TRISC7 = 1;    // RC7 (RX) is input
    
    // Configure UART
    TXSTAbits.BRGH = 1;      // High speed baud rate
    BAUDCONbits.BRG16 = 1;   // 16-bit baud rate generator
    
    SPBRGH = 0x02;           // Set baud rate to 9600
    SPBRG = 0x08;            // (SPBRGH:SPBRG = 520 for 9600 baud at 20MHz)
    
    RCSTAbits.SPEN = 1;      // Enable serial port
    TXSTAbits.SYNC = 0;      // Asynchronous mode
    TXSTAbits.TXEN = 1;      // Enable transmitter
    RCSTAbits.CREN = 1;      // Enable receiver
    
    TRISCbits.TRISC6 = 0;    // RC6 (TX) as output
    
    // Enable UART receive interrupt
    PIE1bits.RCIE = 1;
}

unsigned char uart_receive(void) {
    // Wait for data to be received
    while(!PIR1bits.RCIF);
    
    // Return received data
    return RCREG;
}

void uart_transmit(unsigned char data) {
    // Wait for transmit buffer to be empty
    while(!TXSTAbits.TRMT);
    
    // Send data
    TXREG = data;
}

void process_command(unsigned char cmd) {
    switch(cmd) {
        case LED_ON_CMD:
            blinking = 0;        // Disable blinking
            LATBbits.LATB0 = 1;  // Turn LED on
            break;
            
        case LED_OFF_CMD:
            blinking = 0;        // Disable blinking
            LATBbits.LATB0 = 0;  // Turn LED off
            break;
            
        case BLINK_CMD:
            blinking = 1;        // Enable blinking
            break;
            
        default:
            // Unknown command - do nothing
            break;
    }
    
    // Send acknowledgment
    uart_transmit(cmd);
}
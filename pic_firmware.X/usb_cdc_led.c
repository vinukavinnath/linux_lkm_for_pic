// usb_cdc_led.c - Main source file for USB CDC LED control
#include <xc.h>
#include "usb_config.h"
#include "usb_device.h"
#include "usb_device_cdc.h"

// Configuration bits for PIC18F4550
#pragma config PLLDIV = 5       // 20 MHz crystal with 5 prescaler for 4MHz
#pragma config CPUDIV = OSC1_PLL2 // CPU system clock postscaler 
#pragma config USBDIV = 2       // Clock source from 96MHz PLL/2
#pragma config FOSC = HSPLL_HS  // HS oscillator, PLL enabled
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor disabled
#pragma config IESO = OFF       // Internal/External Oscillator Switchover disabled
#pragma config PWRT = ON        // Power-up Timer enabled
#pragma config BOR = ON         // Brown-out Reset enabled
#pragma config BORV = 3         // Brown-out Voltage set to 2.1V
#pragma config VREGEN = ON      // USB Voltage Regulator enabled
#pragma config WDT = OFF        // Watchdog Timer disabled
#pragma config WDTPS = 32768    // Watchdog Timer Postscale
#pragma config CCP2MX = ON      // CCP2 multiplexed with RC1
#pragma config PBADEN = OFF     // PORTB<4:0> pins are configured as digital I/O
#pragma config LPT1OSC = OFF    // Timer1 configured for higher power operation
#pragma config MCLRE = ON       // MCLR pin enabled
#pragma config STVREN = ON      // Stack full/underflow will cause reset
#pragma config LVP = OFF        // Single-Supply ICSP disabled
#pragma config XINST = OFF      // Extended Instruction Set disabled
#pragma config CP0 = OFF        // Program memory block not code-protected
#pragma config CP1 = OFF        // Program memory block not code-protected
#pragma config CP2 = OFF        // Program memory block not code-protected
#pragma config CP3 = OFF        // Program memory block not code-protected
#pragma config CPB = OFF        // Boot block not code-protected
#pragma config CPD = OFF        // Data EEPROM not code-protected
#pragma config WRT0 = OFF       // Program memory block not write-protected
#pragma config WRT1 = OFF       // Program memory block not write-protected
#pragma config WRT2 = OFF       // Program memory block not write-protected
#pragma config WRT3 = OFF       // Program memory block not write-protected
#pragma config WRTC = OFF       // Configuration registers not write-protected
#pragma config WRTB = OFF       // Boot block not write-protected
#pragma config WRTD = OFF       // Data EEPROM not write-protected
#pragma config EBTR0 = OFF      // Program memory block not protected from table reads
#pragma config EBTR1 = OFF      // Program memory block not protected from table reads
#pragma config EBTR2 = OFF      // Program memory block not protected from table reads
#pragma config EBTR3 = OFF      // Program memory block not protected from table reads
#pragma config EBTRB = OFF      // Boot block not protected from table reads

// Define the LED pin (assuming RD0, adjust as needed)
#define LED_PIN  LATDbits.LATD0
#define LED_TRIS TRISDbits.TRISD0

// Constants
#define USB_BUFFER_SIZE 64
#define LED_ON_COMMAND  'O'
#define LED_OFF_COMMAND 'F'
#define LED_STATUS_COMMAND 'S'

// USB buffer
unsigned char usbReadBuffer[USB_BUFFER_SIZE];
unsigned char usbWriteBuffer[USB_BUFFER_SIZE];

// Function prototypes
void initSystem(void);
void processUSBCommands(void);
void USBDeviceTasks(void);

void main(void) {
    // Initialize the system
    initSystem();
    
    // Main loop
    while(1) {
        // Check if device is configured
        if(USBDeviceState < CONFIGURED_STATE) {
            continue;  // Skip the rest of the loop if not configured
        }
        
        // Process USB CDC commands
        processUSBCommands();
        
        // Required USB tasks
        USBDeviceTasks();
    }
}

void initSystem(void) {
    // Configure oscillator
    OSCCON = 0x70;  // Set to 8MHz internal oscillator
    
    // Configure LED pin as output and turn it off initially
    LED_TRIS = 0;  // Configure as output
    LED_PIN = 0;   // Initially off
    
    // Initialize USB module
    USBDeviceInit();
    
    // Enable interrupts
    INTCONbits.PEIE = 1;  // Enable peripheral interrupts
    INTCONbits.GIE = 1;   // Enable global interrupts
}

void processUSBCommands(void) {
    // Check if data is available to read
    if(!USBUSARTIsTxTrfReady()) return;
    
    // Get number of bytes received from host
    BYTE numBytesRead = getsUSBUSART(usbReadBuffer, USB_BUFFER_SIZE);
    
    // Process received data if available
    if(numBytesRead > 0) {
        // Process each command
        for(BYTE i = 0; i < numBytesRead; i++) {
            switch(usbReadBuffer[i]) {
                case LED_ON_COMMAND:
                    LED_PIN = 1;  // Turn LED on
                    usbWriteBuffer[0] = 'N';  // Send confirmation
                    putUSBUSART(usbWriteBuffer, 1);
                    break;
                    
                case LED_OFF_COMMAND:
                    LED_PIN = 0;  // Turn LED off
                    usbWriteBuffer[0] = 'F';  // Send confirmation
                    putUSBUSART(usbWriteBuffer, 1);
                    break;
                    
                case LED_STATUS_COMMAND:
                    // Send back the current status of the LED
                    usbWriteBuffer[0] = LED_PIN ? '1' : '0';
                    putUSBUSART(usbWriteBuffer, 1);
                    break;
                    
                default:
                    // Unknown command
                    break;
            }
        }
    }
    
    CDCTxService();
}

// USB Callback functions needed by the USB library
void USBCBSuspend(void) {
    // USB bus suspend - typically used to enter low power mode
}

void USBCBWakeFromSuspend(void) {
    // Wake from suspend
}

BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size) {
    switch(event) {
        case EVENT_TRANSFER:
            return TRUE;
            
        case EVENT_SOF:
            return TRUE;
            
        case EVENT_SUSPEND:
            USBCBSuspend();
            return TRUE;
            
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            return TRUE;
            
        case EVENT_CONFIGURED:
            return TRUE;
            
        case EVENT_SET_DESCRIPTOR:
            return TRUE;
            
        case EVENT_EP0_REQUEST:
            USBCheckCDCRequest();
            return TRUE;
            
        case EVENT_BUS_ERROR:
            return TRUE;
            
        case EVENT_TRANSFER_TERMINATED:
            return TRUE;
            
        default:
            return FALSE;
    }
}
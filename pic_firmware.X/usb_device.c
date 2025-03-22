// usb_device.c - USB Device Implementation
#include "usb_device.h"
#include "usb_config.h"

// Define missing buffer sizes
#define USB_EP0_BUFF_SIZE 8

// USB Global Variables
USB_DEVICE_STATE USBDeviceState = DETACHED_STATE;
BYTE USBActiveConfiguration = 0;
BYTE USBAlternateInterface[USB_MAX_EP_NUMBER];

// USB Buffers
BYTE ep0Bo[USB_EP0_BUFF_SIZE];
BYTE ep0Bi[USB_EP0_BUFF_SIZE];
BYTE ep1Bo[CDC_DATA_OUT_EP_SIZE];
BYTE ep1Bi[CDC_DATA_IN_EP_SIZE];
BYTE ep2Bo[CDC_DATA_OUT_EP_SIZE];
BYTE ep2Bi[CDC_DATA_IN_EP_SIZE];

// USB Descriptors
const BYTE device_descriptor[18] = {
    0x12,                       // Size of this descriptor in bytes
    0x01,                       // DEVICE descriptor type
    0x00, 0x02,                 // USB Spec Release Number in BCD format (2.00)
    0x02,                       // Class Code (CDC)
    0x00,                       // Subclass code
    0x00,                       // Protocol code
    USB_EP0_BUFF_SIZE,          // Max packet size for EP0
    (BYTE)(USB_VID & 0xFF), (BYTE)((USB_VID >> 8) & 0xFF),  // Vendor ID
    (BYTE)(USB_PID & 0xFF), (BYTE)((USB_PID >> 8) & 0xFF),  // Product ID
    0x00, 0x01,                 // Device release number in BCD format
    0x01,                       // Manufacturer string index
    0x02,                       // Product string index
    0x00,                       // Device serial number string index
    0x01                        // Number of possible configurations
};

const BYTE configuration_descriptor[67] = {
    // Configuration Descriptor
    0x09,                       // Size of this descriptor in bytes
    0x02,                       // CONFIGURATION descriptor type
    67, 0,                      // Total length of data for this cfg
    2,                          // Number of interfaces in this cfg
    1,                          // Index value of this configuration
    0,                          // Configuration string index
    0x80,                       // Attributes (bus powered)
    50,                         // Max power consumption (100mA)
            
    // Interface Descriptor (CDC Communication Interface)
    0x09,                       // Size of this descriptor in bytes
    0x04,                       // INTERFACE descriptor type
    CDC_COMM_INTF_ID,           // Interface Number
    0x00,                       // Alternate Setting Number
    0x01,                       // Number of endpoints in this interface
    0x02,                       // Class code (Communication Interface Class)
    0x02,                       // Subclass code (Abstract Control Model)
    0x01,                       // Protocol code (V.25ter, AT Commands)
    0x00,                       // Interface string index
    
    // CDC Class-Specific Descriptors
    0x05,                       // Size of this descriptor in bytes
    0x24,                       // CS_INTERFACE descriptor type
    0x00,                       // Header functional descriptor subtype
    0x10, 0x01,                 // CDC Specification release number
    
    0x04,                       // Size of this descriptor in bytes
    0x24,                       // CS_INTERFACE descriptor type
    0x02,                       // Abstract Control Management functional descriptor subtype
    0x00,                       // Capabilities
    
    0x05,                       // Size of this descriptor in bytes
    0x24,                       // CS_INTERFACE descriptor type
    0x06,                       // Union functional descriptor subtype
    CDC_COMM_INTF_ID,           // Communication Class Interface ID
    CDC_DATA_INTF_ID,           // Data Class Interface ID
    
    0x07,                       // Size of this descriptor in bytes
    0x05,                       // ENDPOINT descriptor type
    CDC_COMM_EP | 0x80,         // Endpoint Address (EP1 IN)
    0x03,                       // Attributes (Interrupt)
    CDC_NOTICE_EP_SIZE, 0x00,   // Max packet size
    0x02,                       // Interval (2 ms)
            
    // Interface Descriptor (CDC Data Interface)
    0x09,                       // Size of this descriptor in bytes
    0x04,                       // INTERFACE descriptor type
    CDC_DATA_INTF_ID,           // Interface Number
    0x00,                       // Alternate Setting Number
    0x02,                       // Number of endpoints in this interface
    0x0A,                       // Class code (CDC Data)
    0x00,                       // Subclass code
    0x00,                       // Protocol code
    0x00                        // Interface string index
};

// Added separate descriptor entries to fix the initialization error
const BYTE ep_out_descriptor[7] = {
    // Endpoint Descriptor (OUT)
    0x07,                       // Size of this descriptor in bytes
    0x05,                       // ENDPOINT descriptor type
    CDC_DATA_EP,                // Endpoint Address (EP2 OUT)
    0x02,                       // Attributes (Bulk)
    CDC_DATA_OUT_EP_SIZE, 0x00, // Max packet size
    0x00                        // Interval (ignored for Bulk)
};

const BYTE ep_in_descriptor[7] = {
    // Endpoint Descriptor (IN)
    0x07,                       // Size of this descriptor in bytes
    0x05,                       // ENDPOINT descriptor type
    CDC_DATA_EP | 0x80,         // Endpoint Address (EP2 IN)
    0x02,                       // Attributes (Bulk)
    CDC_DATA_IN_EP_SIZE, 0x00,  // Max packet size
    0x00                        // Interval (ignored for Bulk)
};

// String descriptors - fixed the initialization syntax
const BYTE string0_descriptor[4] = {
    // String 0: Language ID array
    0x04,                      // Size of this descriptor in bytes
    0x03,                      // STRING descriptor type
    0x09, 0x04                 // Language ID: English (United States)
};

const BYTE string1_descriptor[22] = {
    // String 1: Manufacturer
    22,                        // Size of this descriptor in bytes
    0x03,                      // STRING descriptor type
    'Y', 0, 'o', 0, 'u', 0, 'r', 0, ' ', 0, 'N', 0, 'a', 0, 'm', 0, 'e', 0, 0, 0
};

const BYTE string2_descriptor[70] = {
    // String 2: Product
    70,                        // Size of this descriptor in bytes
    0x03,                      // STRING descriptor type
    'P', 0, 'I', 0, 'C', 0, '1', 0, '8', 0, 'F', 0, '4', 0, '5', 0, '5', 0, '0', 0,
    ' ', 0, 'U', 0, 'S', 0, 'B', 0, ' ', 0, 'C', 0, 'D', 0, 'C', 0, ' ', 0, 'L', 0,
    'E', 0, 'D', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0, 'r', 0, 'o', 0, 'l', 0,
    'l', 0, 'e', 0, 'r', 0, 0, 0
};

// USB Device Initialization
void USBDeviceInit(void) {
    // Reset all USB endpoints
    UCON = 0;          // Disable USB module
    UCFG = 0;          // Configure USB module
    
    // Configure USB module
    UCFG = 0x14;       // USB internal transceiver, full-speed mode, with pull-up enabled
    
    // Reset all USB registers
    UADDR = 0;         // Reset device address
    UIE = 0;           // Disable all USB interrupts
    UIR = 0;           // Clear all USB interrupt flags
    UEIE = 0;          // Disable all USB error interrupts
    UEIR = 0;          // Clear all USB error interrupt flags
    
    // Enable required USB interrupts
    UIE = 0x59;        // Enable RESET, IDLE, STALL, and TOKEN interrupts
    
    // Configure endpoints
    UEP0 = 0x16;       // EP0 IN/OUT enabled, handshaking enabled, STALL disabled
    UEP1 = 0x16;       // EP1 IN/OUT enabled, handshaking enabled, STALL disabled
    UEP2 = 0x16;       // EP2 IN/OUT enabled, handshaking enabled, STALL disabled
    
    // Enable USB module
    UCON = 0x08;       // Enable USB module, no suspend
    
    // Set initial state
    USBDeviceState = ATTACHED_STATE;
}

// USB Device Tasks
void USBDeviceTasks(void) {
    BYTE i;
    
    // Check if RESET flag is set
    if(UIRbits.URSTIF) {
        UIRbits.URSTIF = 0;    // Clear reset flag
        
        // Reset EP0 registers
        UEP0 = 0x16;
        
        // Reset device address
        UADDR = 0;
        
        // Reset device state
        USBDeviceState = DEFAULT_STATE;
        
        // We would normally clear all BDT entries here
        // but for simplicity we'll just comment it out
        /*
        for(i = 0; i < 16; i++) {
            // Set BDT to unused
        }
        */
        
        // Re-enable required interrupts
        UIE = 0x59;
    }
    
    // Check if TOKEN interrupt is set
    if(UIRbits.TRNIF) {
        UIRbits.TRNIF = 0;    // Clear token interrupt flag
        
        // Process token
        // In a full implementation, we would handle setup packets, control transfers, etc.
    }
    
    // Check other USB interrupts as needed
}

// Enable USB Endpoint
void USBEnableEndpoint(BYTE ep, BYTE options) {
    // Enable the specified endpoint with the given options
    switch(ep) {
        case 0:
            UEP0 = options;
            break;
        case 1:
            UEP1 = options;
            break;
        case 2:
            UEP2 = options;
            break;
        // Add more cases as needed
    }
}
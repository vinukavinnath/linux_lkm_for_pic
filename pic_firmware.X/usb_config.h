// usb_config.h - USB Configuration Header
#ifndef USB_CONFIG_H
#define USB_CONFIG_H

// USB Device Configuration
#define USB_SUPPORT_DEVICE
#define USB_ENABLE_ALL_HANDLERS

// USB CDC Configuration
#define USB_USE_CDC

// Define Device Vendor and Product IDs
#define USB_VID 0x04D8      // Microchip VID
#define USB_PID 0x000A      // Arbitrary PID (you may need to change this)

// Device Descriptor Configuration
#define USB_MAX_EP_NUMBER 3
#define USB_NUM_STRING_DESCRIPTORS 3

// USB Endpoints Configuration
#define CDC_COMM_INTF_ID 0
#define CDC_DATA_INTF_ID 1
#define CDC_COMM_EP     1
#define CDC_DATA_EP     2

// USB Buffer Sizes
#define CDC_DATA_IN_EP_SIZE 64
#define CDC_DATA_OUT_EP_SIZE 64
#define CDC_NOTICE_EP_SIZE 8

// Allow Dynamic EP Buffer Allocation
#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
#define USB_POLLING
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
#define USB_SPEED_OPTION USB_FULL_SPEED
#define USB_ENABLE_STATUS_STAGE_TIMEOUTS

#define USB_STATUS_STAGE_TIMEOUT (BYTE)45 // Timeout in ms

// Device Name Configuration
#define USB_PRODUCT_STRING "PIC18F4550 USB CDC LED Controller"
#define USB_MANUFACTURER_STRING "Your Name"

// Define ping-pong mode options
#define USB_PING_PONG__NO_PING_PONG         0x00
#define USB_PING_PONG__EP0_OUT_ONLY         0x01
#define USB_PING_PONG__FULL_PING_PONG       0x02
#define USB_PING_PONG__ALL_BUT_EP0          0x03

// Define pullup options
#define USB_PULLUP_ENABLE  0x01
#define USB_PULLUP_DISABLE 0x00

// Define transceiver options
#define USB_INTERNAL_TRANSCEIVER 0x00
#define USB_EXTERNAL_TRANSCEIVER 0x01

// Define speed options
#define USB_FULL_SPEED 0x00
#define USB_LOW_SPEED  0x01

#endif // USB_CONFIG_H
// usb_device.h - USB Device Header
#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <xc.h>

// USB States
typedef enum {
    DETACHED_STATE = 0,
    ATTACHED_STATE,
    POWERED_STATE,
    DEFAULT_STATE,
    ADDRESS_STATE,
    CONFIGURED_STATE
} USB_DEVICE_STATE;

// USB Events
typedef enum {
    EVENT_NONE = 0,
    EVENT_TRANSFER,
    EVENT_SOF,
    EVENT_SUSPEND,
    EVENT_RESUME,
    EVENT_CONFIGURED,
    EVENT_SET_DESCRIPTOR,
    EVENT_EP0_REQUEST,
    EVENT_BUS_ERROR,
    EVENT_TRANSFER_TERMINATED
} USB_EVENT;

// Common Type Definitions
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;
typedef enum { FALSE = 0, TRUE } BOOL;

// Function Prototypes
void USBDeviceInit(void);
void USBDeviceTasks(void);
void USBEnableEndpoint(BYTE ep, BYTE options);
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size);

// External Variables
extern USB_DEVICE_STATE USBDeviceState;

#endif // USB_DEVICE_H
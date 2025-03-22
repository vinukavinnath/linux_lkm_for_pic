// usb_device_cdc.h - USB CDC Class Header
#ifndef USB_DEVICE_CDC_H
#define USB_DEVICE_CDC_H

#include "usb_device.h"

// CDC Function Prototypes
void USBCheckCDCRequest(void);
BOOL USBUSARTIsTxTrfReady(void);
BYTE getsUSBUSART(char *buffer, BYTE len);
void putUSBUSART(char *data, BYTE length);
void CDCTxService(void);

#endif // USB_DEVICE_CDC_H
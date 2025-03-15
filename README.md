# USB-Based Bulb Control Using PIC18F4550 & Linux Kernel Module

## Overview

This project demonstrates the development of a **custom Linux kernel module** and **firmware for PIC18F4550** to control a **bulb** via **USB communication**. The project implements **low-level USB communication** by directly accessing **USB controller registers**, ensuring efficient data transfer between the **Linux system** and the **PIC18F4550 microcontroller**. The communication is achieved without using any existing USB libraries, providing a deeper understanding of **USB protocols** and **device driver development**.

### Key Features
- **Linux Kernel Module**: Implements raw USB control commands to interface with the **PIC18F4550** microcontroller for turning the bulb on/off.
- **PIC18F4550 Firmware**: Processes incoming USB commands (on/off) from the Linux system to control the state of the bulb.
- **No External Libraries**: Avoids using USB libraries, instead accessing USB controller registers directly to manage communication.
- **Device File Interface**: Provides a simple **character device file** (`/dev/usb_bulb`) for easy interaction from the user space, enabling bulb control via **command-line interface**.
- **Modular Design**: Both the Linux kernel module and the firmware are modular, making them easily extendable for future modifications and use cases.

## Technologies

- **C (Linux Kernel Module)**: For implementing USB control and device file interactions.
- **Embedded C (PIC18F4550 Firmware)**: For programming the **PIC18F4550** microcontroller to handle USB communication and bulb control.
- **Linux Kernel Development**: Understanding and interacting with USB subsystem and USB device drivers.
- **USB Communication**: Low-level USB protocol and direct register access for device control.
- **MPLAB X IDE**: For compiling and flashing the firmware onto the **PIC18F4550**.
- **Makefile**: For compiling and building the Linux kernel module.


## Prerequisites

Before you begin, ensure you have the following:

- **Linux OS** (Ubuntu, Fedora, or similar for development and testing).
- **PIC18F4550 microcontroller** hardware setup.
- **MPLAB X IDE**: For compiling and flashing the **PIC18F4550 firmware**.
- **USB Setup**: A USB cable connecting your **PIC18F4550** to the Linux system for communication.
- **Root Access**: Required for loading/unloading kernel modules and interacting with device files.




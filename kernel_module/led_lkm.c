/**
 * led_control.c - Linux kernel module for LED control via serial communication
 * For PIC18F4550 connected via serial port
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("LED Control Driver for PIC18F4550");
MODULE_VERSION("1.0");

#define DEVICE_NAME "led_control"
#define SERIAL_PORT 0x3F8 /* Default COM1 port address */
#define LED_ON_CMD 0x01
#define LED_OFF_CMD 0x00
#define BLINK_CMD 0x02

static int major;
static char message[256];
static int device_opened = 0;

/* Serial port functions */
static inline void serial_outb(unsigned char value, unsigned int port)
{
    outb(value, SERIAL_PORT + port);
}

static inline unsigned char serial_inb(unsigned int port)
{
    return inb(SERIAL_PORT + port);
}

static void serial_init(void)
{
    /* Disable all interrupts */
    serial_outb(0x00, UART_IER);

    /* Set baud rate to 9600 */
    serial_outb(0x80, UART_LCR); /* Set DLAB on */
    serial_outb(0x0C, UART_DLL); /* Set baud rate divisor */
    serial_outb(0x00, UART_DLM); /* Set baud rate divisor */

    /* 8 data bits, 1 stop bit, no parity */
    serial_outb(0x03, UART_LCR);

    /* Enable FIFO, clear them, with 14-byte threshold */
    serial_outb(0xC7, UART_FCR);

    /* DTR active, RTS active */
    serial_outb(0x03, UART_MCR);

    /* Clear the port */
    inb(SERIAL_PORT + UART_LSR);
    inb(SERIAL_PORT + UART_RX);
    inb(SERIAL_PORT + UART_IIR);
    inb(SERIAL_PORT + UART_MSR);
}

static void serial_send(unsigned char data)
{
    int i = 0;

    /* Wait for transmitter to be ready */
    while ((serial_inb(UART_LSR) & UART_LSR_THRE) == 0 && i < 100)
    {
        udelay(100);
        i++;
    }

    /* Send the data */
    serial_outb(data, UART_TX);
}

/* Device operations */
static int device_open(struct inode *inode, struct file *file)
{
    if (device_opened)
        return -EBUSY;

    device_opened++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    device_opened--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    return 0; /* Not implemented */
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    int i;

    /* Get the data from user space */
    if (length > 255)
        length = 255;

    if (copy_from_user(message, buffer, length))
        return -EFAULT;

    message[length] = '\0';

    /* Process the command */
    if (strncmp(message, "ON", 2) == 0)
    {
        serial_send(LED_ON_CMD);
        printk(KERN_INFO "LED Control: LED ON command sent\n");
    }
    else if (strncmp(message, "OFF", 3) == 0)
    {
        serial_send(LED_OFF_CMD);
        printk(KERN_INFO "LED Control: LED OFF command sent\n");
    }
    else if (strncmp(message, "BLINK", 5) == 0)
    {
        serial_send(BLINK_CMD);
        printk(KERN_INFO "LED Control: LED BLINK command sent\n");
    }
    else
    {
        printk(KERN_INFO "LED Control: Unknown command\n");
    }

    return length;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release};

static int __init led_control_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0)
    {
        printk(KERN_ALERT "LED Control: Failed to register character device\n");
        return major;
    }

    printk(KERN_INFO "LED Control: Module loaded with major number %d\n", major);
    printk(KERN_INFO "LED Control: Create device with 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);

    serial_init();
    printk(KERN_INFO "LED Control: Serial port initialized\n");

    return 0;
}

static void __exit led_control_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "LED Control: Module unloaded\n");
}

module_init(led_control_init);
module_exit(led_control_exit);
/*
 * PIC LED Control - Linux Kernel Module
 *
 * This module creates a character device that interfaces with the PIC16F887
 * to control an LED via the serial port.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define DEVICE_NAME "pic_led"
#define CLASS_NAME "pic"
#define TTY_DEVICE "/dev/ttyS0"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("User");
MODULE_DESCRIPTION("Kernel module to control PIC LED via serial port");
MODULE_VERSION("0.1");

static int major_number;
static struct class *pic_led_class = NULL;
static struct device *pic_led_device = NULL;
static struct tty_struct *tty = NULL;
static struct file *tty_file = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release};

// Remove this custom structure definition
// struct tty_file_private
// {
//     struct tty_struct *tty;
// };

static int initialize_serial(void)
{
    // Open the TTY device
    tty_file = filp_open(TTY_DEVICE, O_RDWR, 0);
    if (IS_ERR(tty_file))
    {
        printk(KERN_ALERT "PIC LED: Failed to open TTY device %s\n", TTY_DEVICE);
        return PTR_ERR(tty_file);
    }

    // Access TTY through the file's private data using the kernel's structure
    if (!tty_file->private_data)
    {
        printk(KERN_ALERT "PIC LED: No private data in TTY file\n");
        filp_close(tty_file, NULL);
        return -ENODEV;
    }

    // Use the kernel's existing tty_file_private structure
    tty = ((struct tty_file_private *)tty_file->private_data)->tty;

    if (!tty)
    {
        printk(KERN_ALERT "PIC LED: No TTY found for the device\n");
        filp_close(tty_file, NULL);
        return -ENODEV;
    }

    return 0;
}

static int __init pic_led_init(void)
{
    int ret;

    // Register the character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
    {
        printk(KERN_ALERT "PIC LED: Failed to register a major number\n");
        return major_number;
    }

    // Register the device class - updated for newer kernel API
    pic_led_class = class_create(CLASS_NAME);
    if (IS_ERR(pic_led_class))
    {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "PIC LED: Failed to register device class\n");
        return PTR_ERR(pic_led_class);
    }

    // Register the device driver
    pic_led_device = device_create(pic_led_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(pic_led_device))
    {
        class_destroy(pic_led_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "PIC LED: Failed to create the device\n");
        return PTR_ERR(pic_led_device);
    }

    // Initialize serial port
    ret = initialize_serial();
    if (ret < 0)
    {
        device_destroy(pic_led_class, MKDEV(major_number, 0));
        class_destroy(pic_led_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return ret;
    }

    printk(KERN_INFO "PIC LED: Module loaded successfully\n");
    return 0;
}

static void __exit pic_led_exit(void)
{
    if (tty_file)
    {
        filp_close(tty_file, NULL);
    }

    device_destroy(pic_led_class, MKDEV(major_number, 0));
    class_destroy(pic_led_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "PIC LED: Module unloaded successfully\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "PIC LED: Device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "PIC LED: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    char rx_buffer[2];
    int ret;

    if (*offset > 0)
        return 0;

    if (!tty)
    {
        printk(KERN_ALERT "PIC LED: TTY not available\n");
        return -ENOSYS;
    }

    // In modern kernels, we don't directly use tty->ops->read
    // Instead, we'll use a simpler approach - just return the last state
    // For real implementation, you'd need to set up proper TTY line discipline

    // For simplicity, we'll just return a fixed value for now
    rx_buffer[0] = '0'; // Default state
    rx_buffer[1] = '\0';

    // Copy to user space
    ret = copy_to_user(buffer, rx_buffer, 1);
    if (ret)
    {
        printk(KERN_ALERT "PIC LED: Failed to send data to user\n");
        return -EFAULT;
    }

    *offset = 1;
    return 1;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    char tx_buffer[2];
    int ret, written;

    if (len == 0)
        return 0;

    if (!tty)
    {
        printk(KERN_ALERT "PIC LED: TTY not available\n");
        return -ENOSYS;
    }

    // Copy from user space
    ret = copy_from_user(tx_buffer, buffer, 1);
    if (ret)
    {
        printk(KERN_ALERT "PIC LED: Failed to receive data from user\n");
        return -EFAULT;
    }

    // Send command to PIC via serial port
    if (tx_buffer[0] == '0' || tx_buffer[0] == '1')
    {
        // Use tty_write instead of ops->write
        written = tty_write_room(tty);
        if (written < 1)
        {
            printk(KERN_ALERT "PIC LED: No room to write to TTY\n");
            return -ENOSPC;
        }

        ret = tty_insert_flip_string(tty->port, tx_buffer, 1);
        if (ret != 1)
        {
            printk(KERN_ALERT "PIC LED: Error writing to serial port\n");
            return -EIO;
        }

        tty_flip_buffer_push(tty->port);

        printk(KERN_INFO "PIC LED: Sent command %c to PIC\n", tx_buffer[0]);
        return 1;
    }

    printk(KERN_ALERT "PIC LED: Invalid command (must be '0' or '1')\n");
    return -EINVAL;
}

module_init(pic_led_init);
module_exit(pic_led_exit);
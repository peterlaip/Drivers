#include <linux/module.h>   
#include <linux/kernel.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h>  
#include <linux/gpio.h>     
#include <linux/err.h>      
#include <linux/delay.h>    
#include <linux/device.h>   

#define DEVICE_NAME "fan_driver" 
#define CLASS_NAME  "fan"      
#define GPIO_FAN1 17            
#define GPIO_FAN2 22             

static int major;
static struct class* fan_class = NULL;      
static struct device* fan_device = NULL;    

static int fan_driver_open(struct inode* inode, struct file* file) {
    return 0;
}

static int fan_driver_release(struct inode* inode, struct file* file) {
    return 0;
}


static ssize_t fan_driver_write(struct file* file, const char __user* buf, size_t count, loff_t* offset) {
    char command[256];

    // Check if the input data is too long
    if (count > sizeof(command) - 1) {
        return -EINVAL;
    }

    if (copy_from_user(command, buf, count)) {
        return -EFAULT;
    }

    command[count] = '\0';

    int fan_number;
    char state;

    // Scan the command
    if (sscanf(command, "%d %c", &fan_number, &state) != 2) {
        return -EINVAL;
    }

    printk(KERN_INFO "fan_driver: fan_number=%d, state=%c\n", fan_number, state);

    if (fan_number == 1) {
        gpio_set_value(GPIO_FAN1, (state == '1') ? 1 : 0);
    }
    else if (fan_number == 2) {
        gpio_set_value(GPIO_FAN2, (state == '1') ? 1 : 0);
    }
    else {
        return -EINVAL;
    }

    return count;
}


static struct file_operations fops = {
    .open = fan_driver_open,
    .release = fan_driver_release,
    .write = fan_driver_write,
};


static int __init fan_driver_init(void) {
    int result;

   
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "fan_driver: failed to register a major number\n");
        return major;
    }

    // Create a class for the device
    fan_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(fan_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ERR "fan_driver: failed to create device class\n");
        return PTR_ERR(fan_class);
    }

    // Create the device file under /dev
    fan_device = device_create(fan_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(fan_device)) {
        class_destroy(fan_class);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ERR "fan_driver: failed to create the device\n");
        return PTR_ERR(fan_device);
    }

    if (!gpio_is_valid(GPIO_FAN1) || !gpio_is_valid(GPIO_FAN2)) {
        device_destroy(fan_class, MKDEV(major, 0));
        class_destroy(fan_class);
        unregister_chrdev(major, DEVICE_NAME);
        return -ENODEV;
    }

    // Request control of the GPIO pins
    result = gpio_request(GPIO_FAN1, "fan1");
    if (result) {
        device_destroy(fan_class, MKDEV(major, 0));
        class_destroy(fan_class);
        unregister_chrdev(major, DEVICE_NAME);
        return result;
    }

    result = gpio_request(GPIO_FAN2, "fan2");
    if (result) {
        gpio_free(GPIO_FAN1);
        device_destroy(fan_class, MKDEV(major, 0));
        class_destroy(fan_class);
        unregister_chrdev(major, DEVICE_NAME);
        return result;
    }

  
    gpio_direction_output(GPIO_FAN1, 0);
    gpio_direction_output(GPIO_FAN2, 0);

    printk(KERN_INFO "fan_driver: initialized\n");
    return 0;
}


static void __exit fan_driver_exit(void) {
    gpio_set_value(GPIO_FAN1, 0);
    gpio_set_value(GPIO_FAN2, 0);
    gpio_free(GPIO_FAN1);
    gpio_free(GPIO_FAN2);

   
    device_destroy(fan_class, MKDEV(major, 0));
    class_unregister(fan_class);
    class_destroy(fan_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "fan_driver: exited\n");
}

module_init(fan_driver_init);
module_exit(fan_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAI");
MODULE_DESCRIPTION("A driver for controlling a relay via GPIO 17, 22");

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>           // for struct file_operations, register_chrdev()
#include <linux/uaccess.h>      // for copy_to_user()
#include <linux/i2c.h>          // for i2c_get_adapter(), i2c_put_adapter(), struct i2c_client...
#include <linux/delay.h>        // for msleep()
#include <linux/device.h>       // for struct class, struct device, class_create()...

#define DEVICE_NAME "mq3_driver"
#define CLASS_NAME  "mq3"
#define ADS1115_ADDR 0x48

// ADS1115 registers
#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG 0x01

// ADS1115 config register bits
#define ADS1115_OS_SINGLE 0x8000
#define ADS1115_MUX_SINGLE_0 0x4000
#define ADS1115_PGA_4_096V 0x0200
#define ADS1115_MODE_SINGLE 0x0100
#define ADS1115_DR_128SPS 0x0080
#define ADS1115_COMP_MODE_TRAD 0x0000
#define ADS1115_COMP_POL_ACTVLOW 0x0000
#define ADS1115_COMP_LAT_NONLAT 0x0000
#define ADS1115_COMP_QUE_DISABLE 0x0003

static int major;
static struct i2c_client* client;
static struct class* mq3_class = NULL;   
static struct device* mq3_device = NULL; 

static int mq3_driver_open(struct inode* inode, struct file* file) {
    return 0;
}

static int mq3_driver_release(struct inode* inode, struct file* file) {
    return 0;
}

static ssize_t mq3_driver_read(struct file* file, char __user* buf, size_t count, loff_t* ppos) {
    u16 config = ADS1115_OS_SINGLE | ADS1115_MUX_SINGLE_0 | ADS1115_PGA_4_096V |
        ADS1115_MODE_SINGLE | ADS1115_DR_128SPS | ADS1115_COMP_MODE_TRAD |
        ADS1115_COMP_POL_ACTVLOW | ADS1115_COMP_LAT_NONLAT | ADS1115_COMP_QUE_DISABLE;
    int ret;
    u16 adc_value;
    char kernel_buf[20];

    // Write config and start conversion
    ret = i2c_smbus_write_word_swapped(client, ADS1115_REG_CONFIG, config);
    if (ret < 0) {
        return ret;
    }

    // Wait for conversion to complete 
    msleep(8);

    // Read conversion result
    ret = i2c_smbus_read_word_swapped(client, ADS1115_REG_CONVERSION);
    if (ret < 0) {
        return ret;
    }
    adc_value = ret;

    ret = snprintf(kernel_buf, sizeof(kernel_buf), "%d\n", adc_value);
    if (ret < 0) {
        return ret;
    }

    if (copy_to_user(buf, kernel_buf, ret)) {
        return -EFAULT;
    }

    return ret;
}

static struct file_operations fops = {
    .open = mq3_driver_open,
    .release = mq3_driver_release,
    .read = mq3_driver_read,
};

static int __init mq3_driver_init(void) {
    struct i2c_adapter* adapter;
    struct i2c_board_info board_info = {
        I2C_BOARD_INFO("ads1115", ADS1115_ADDR)
    };


    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "mq3_driver: failed to register a major number\n");
        return major;
    }

 
    mq3_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mq3_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ERR "mq3_driver: failed to create device class\n");
        return PTR_ERR(mq3_class);
    }


    mq3_device = device_create(mq3_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mq3_device)) {
        class_destroy(mq3_class);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ERR "mq3_driver: failed to create the device\n");
        return PTR_ERR(mq3_device);
    }

    // Get the I2C adapter
    adapter = i2c_get_adapter(1);  // Assuming I2C bus 1, change if necessary
    if (!adapter) {
        device_destroy(mq3_class, MKDEV(major, 0));
        class_destroy(mq3_class);
        unregister_chrdev(major, DEVICE_NAME);
        return -ENODEV;
    }

    // Create a new I2C client for the device
    client = i2c_new_client_device(adapter, &board_info);
    i2c_put_adapter(adapter);

    if (IS_ERR(client)) {
        device_destroy(mq3_class, MKDEV(major, 0));
        class_destroy(mq3_class);
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(client);
    }

    printk(KERN_INFO "mq3_driver: device initialized successfully\n");
    return 0;
}

static void __exit mq3_driver_exit(void) {
    if (client) {
        i2c_unregister_device(client);
    }

 
    device_destroy(mq3_class, MKDEV(major, 0));
    class_unregister(mq3_class);
    class_destroy(mq3_class);
    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "mq3_driver: device exited successfully\n");
}

module_init(mq3_driver_init);
module_exit(mq3_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAI");
MODULE_DESCRIPTION("MQ3 Driver");

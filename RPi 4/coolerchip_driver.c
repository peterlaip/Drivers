#include <linux/module.h>   
#include <linux/kernel.h>   
#include <linux/gpio.h>     
#include <linux/fs.h>      
#include <linux/uaccess.h> 

#define DEVICE_NAME "coolerchip_driver" 
#define CLASS_NAME "coolerchip"         

static int majorNumber;                
static struct class* coolerchipClass = NULL;   
static struct device* coolerchipDevice = NULL;

#define GPIO_COOLER 27 


static ssize_t coolerchip_write(struct file* file, const char* buffer, size_t len, loff_t* offset) {
    char command;

    if (copy_from_user(&command, buffer, len))
    {
        return -EFAULT; 
    }

    // Set the GPIO value based on the command ('1' for ON, '0' for OFF)
    gpio_set_value(GPIO_COOLER, command == '1' ? 1 : 0);

    return len; 
}

static struct file_operations fops = {
    .write = coolerchip_write,
};


static int __init coolerchip_driver_init(void) {
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "CoolerChipDriver failed to register a major number\n");
        return majorNumber; 
    }

    coolerchipClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(coolerchipClass))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return PTR_ERR(coolerchipClass); 
    }

    coolerchipDevice = device_create(coolerchipClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(coolerchipDevice))
    {
        class_destroy(coolerchipClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return PTR_ERR(coolerchipDevice); 
    }

    gpio_request(GPIO_COOLER, "sysfs");
    gpio_direction_output(GPIO_COOLER, 0); 
    gpio_export(GPIO_COOLER, false); 

    printk(KERN_INFO "CoolerChipDriver: initialized\n");
    return 0; 
}

static void __exit coolerchip_driver_exit(void) {
    gpio_unexport(GPIO_COOLER); 
    gpio_free(GPIO_COOLER);    

    device_destroy(coolerchipClass, MKDEV(majorNumber, 0));
    class_unregister(coolerchipClass);
    class_destroy(coolerchipClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "CoolerChipDriver: exiting\n");
}


module_init(coolerchip_driver_init);
module_exit(coolerchip_driver_exit);

MODULE_LICENSE("GPL");          
MODULE_AUTHOR("LAI");      
MODULE_DESCRIPTION("A driver for controlling a relay via GPIO 27");        

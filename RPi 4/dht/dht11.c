#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
// #include <mach/gpio.h>
//#include <asm-generic/uaccess.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/err.h>

//LED is connected to this GPIO
#define PIN (21)
#define DEVICE_MAJOR 232
#define DEVICE_NAME "dht11"
// #define PIN  EXYNOS4_GPX0(6)

typedef unsigned char  U8;
unsigned char buf[6];
//DEFINE_SPINLOCK(lock);
unsigned char check_flag;

int data_in(void)
{
        gpio_direction_input(PIN);
        return gpio_get_value(PIN);
}
void gpio_out(int value)   //set gpio is output
{
    gpio_direction_output(PIN,value);
}

void humidity_read_data(void)
{
        int i=0,j=0;
        int num;
    unsigned char flag=0;
        unsigned char data=0;
        gpio_out(0);
        mdelay(30);
        gpio_out(1);
        udelay(40);
        if(data_in() == 0)
        {
                while(!gpio_get_value(PIN))
                {
                        udelay(5);
                        i++;
                        if(i>20)
                        {
                                printk("error data!\n");
                                break;
                }
                }
                i=0;
                while(gpio_get_value(PIN))
                {
                udelay(5);
                i++;
                if(i>20)
                {
                                printk("error data!\n");
                                break;
                        }
                }
        }
        for(i = 0;i < 5;i++)
        {
                for(num = 0;num < 8;num++)
                {
                        j = 0;
                        while( !gpio_get_value(PIN) )
                        {
                                udelay(10);
                                j++;
                                if(j > 40)
                                break;
                        }
                        flag = 0x0;
                        udelay(28);
                        if( gpio_get_value(PIN) )
                        {
                                flag = 0x01;
                        }
                        j = 0;
                        while( gpio_get_value(PIN) )
                        {
                                udelay(10);
                                j++;
                                if(j > 60)
                                break;
                        }
                        data<<=1;
                        data|=flag;
                }
                buf[i] = data;
        }
        buf[5] = buf[0] + buf[1] + buf[2] + buf[3];
        if(buf[4] == buf[5])
        {
                check_flag = 0xff;
                printk("humidity check pass\n");
                printk("humidity=[%d],temp=[%d]\n", buf[0], buf[2]);
        }
        else
        {
                check_flag = 0x0;
                printk("humidity check fail\n");
        }
}

static ssize_t humidiy_read(struct file *file, char* buffer, size_t size, loff_t *off)
{
        int ret;
        local_irq_disable();
        humidity_read_data();
        local_irq_enable();
        if(check_flag == 0xff)
                {
                        ret = copy_to_user(buffer,buf,sizeof(buf));
                        if(ret < 0)
                        {
                                printk("copy to user err\n");
                                return -EAGAIN;
            }
            else
            {
                return 0;
                        }
         }
         else
         {
                return -EAGAIN;
                 }
}

static int humidiy_open(struct inode *inode, struct file *file)
{
    printk("open in kernel\n");
    return 0;
}

static int humidiy_release(struct inode *inode, struct file *file)
{
    printk("humidity release\n");
    return 0;
}

static struct file_operations humidity_dev_fops={
    owner               :       THIS_MODULE,
    open                :       humidiy_open,
        read            :       humidiy_read,
        release         :       humidiy_release,
};
static struct class *dht_class;

static int __init humidity_dev_init(void)
{
        /*
         * Register device
         */
        int     ret;

        ret = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &humidity_dev_fops);
        if (ret < 0) {
                printk(KERN_INFO "%s: registering device %s with major %d failed with %d\n",
                       __func__, DEVICE_NAME, DEVICE_MAJOR, DEVICE_MAJOR );
                return(ret);
        }
        printk("DHT11 driver register success!\n");

        dht_class = class_create(THIS_MODULE, "dht11");
        if (IS_ERR(dht_class))
        {
                printk(KERN_WARNING "Can't make node %d\n", DEVICE_MAJOR);
                return PTR_ERR(dht_class);
        }

    device_create(dht_class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);

        printk("DHT11 driver make node success!\n");

        // Reserve gpios
        if( gpio_request( PIN, DEVICE_NAME ) < 0 )      // request pin 2
        {
                printk( KERN_INFO "%s: %s unable to get TRIG gpio\n", DEVICE_NAME, __func__ );
                ret = -EBUSY;
                return(ret);
        }

        // Set gpios directions
        if( gpio_direction_output( PIN, 1) < 0 )        // Set pin 2 as output with default value 0
        {
                printk( KERN_INFO "%s: %s unable to set TRIG gpio as output\n", DEVICE_NAME, __func__ );
                ret = -EBUSY;
                return(ret);
        }

    return 0;
}

static void __exit humidity_dev_exit(void)
{
    gpio_free(PIN);
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

module_init(humidity_dev_init);
module_exit(humidity_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WWW");

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/of_gpio.h>


#define TIMER_CNT    1             /*设备号个数*/
#define TIMER_NAME  "timer" 
#define LED_OPEN     1          
#define LED_CLOSE    0            
#define CLOSE_CMD   (_IO(0xFE,0x1))
#define OPEN_CMD    (_IO(0xFE,0x2))
#define SET_PERIOD  (_IO(0xFE,0x3))

struct timer_dev{
    dev_t          devid ;      /*设备号*/
    struct cdev    cdev;
    struct class  *class;
    struct device *devices;
    int major;
    int minor;
    struct device_node *nd;    /*设备节点*/
    int           led_gpio;    /*key所使用的编号*/
    spinlock_t    lock;        /*定义自悬锁变量，保护数据*/
    struct timer_list timer;   /*定义定时器*/
    int           timeperiod;  /*定时器周期，ms*/
};

struct timer_dev  timerdev;    /*定時器设备*/

static int led_init(void)      /*读取设备树文件，初始化Led*/
{
    int ret;
    timerdev.nd = of_find_node_by_path("/gpio-led");
    if(!timerdev.nd) return -EINVAL;
    
    timerdev.led_gpio = of_get_named_gpio(timerdev.nd, "led-gpio",0);
    if(timerdev.led_gpio < 0) return -EINVAL;

    gpio_request(timerdev.led_gpio,"led");
    ret = gpio_direction_output(timerdev.led_gpio,1);
    if(ret < 0 ){
        printk("set gpio error\r\n");
        return -EINVAL;
    }
    return 0;
}

static int timer_open(struct inode *inode,struct file *filp) //初始化led，设置定时周期
{
    int ret;
    filp->private_data = &timerdev;/*设置私有数据*/
	timerdev.timeperiod =1000;  /*定时周期： 1000ms*/
    ret = led_init();
    return ret;
}
//                          ioctl函数
static long timer_unlocked_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
    struct timer_dev *dev = filp->private_data;
    int timer_period;
    unsigned long flags;
    switch(cmd){
        case OPEN_CMD:   //打开定时器
            spin_lock_irqsave(&dev->lock, flags);
            timer_period = dev->timeperiod;
            spin_unlock_irqrestore(&dev->lock, flags);
            mod_timer( &dev->timer, jiffies +msecs_to_jiffies(timer_period) );
            break;
        case CLOSE_CMD: //关闭定时器
            del_timer(&dev->timer);
            break;
        case SET_PERIOD://设置定时器定时时间
            spin_lock_irqsave(&dev->lock, flags);
            timer_period = arg;
            dev->timeperiod   = arg;
            spin_unlock_irqrestore(&dev->lock, flags);
            mod_timer( &dev->timer, jiffies +msecs_to_jiffies(timer_period) );
            break;
        default:
            break;
    }
    return 0;
}
static int timer_release(struct inode *inode,struct file *filp)
{
	return 0;
}
static struct file_operations timer_fops={
    .owner=           THIS_MODULE,
    .open =           timer_open,
    .unlocked_ioctl = timer_unlocked_ioctl,
    .release=         timer_release
};

void function_timer(unsigned long arg)
{
    struct timer_dev *dev = (struct timer_dev *)arg; //将地址编号强制转换为该结构提地址，
    static int sta = 1;
    int timerperiod;
    unsigned long flags;
    sta = !sta;/* 每次都取反,实现 LED 灯反转 */
    gpio_set_value(dev->led_gpio, sta);
    /* 重启定时器 */
    spin_lock_irqsave(&dev->lock, flags);
    timerperiod = dev->timeperiod;
    spin_unlock_irqrestore(&dev->lock, flags);
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod));

}

//////////////////////////////模块初始化，卸载函数。////////////////////////////
static int __init timer_init(void)
{
    spin_lock_init(&timerdev.lock);
    /*创建设备号*/
    if(timerdev.major){    /*如果定义了设备号*/
        timerdev.devid = MKDEV(timerdev.major,0);  //确定设备号
        register_chrdev_region(timerdev.devid,TIMER_CNT,
                        TIMER_NAME);  /*注册设备号*/
    }else {
        alloc_chrdev_region(&timerdev.devid,0,TIMER_CNT,
                        TIMER_NAME);  /*申请设备号*/
        register_chrdev_region(timerdev.devid,TIMER_CNT,
                        TIMER_NAME);  /*注册设备号*/               
        timerdev.major = MAJOR(timerdev.devid);
        timerdev.minor = MINOR(timerdev.devid);
    }
    printk("timerdev major = %d, minor = %d \r\n",
                   timerdev.major,timerdev.minor);
    timerdev.cdev.owner = THIS_MODULE;  /*cdev结构体初始化*/
    cdev_init(&timerdev.cdev,&timer_fops);
    cdev_add( &timerdev.cdev,timerdev.devid,TIMER_CNT);  /*添加字符设备*/
    /*创建设备节点*/
    timerdev.class = class_create(THIS_MODULE,TIMER_NAME);/*创建类*/
    if(IS_ERR(timerdev.class)){
        return PTR_ERR(timerdev.class);
    }
    timerdev.devices = device_create(timerdev.class,NULL ,   /*创建设备*/
                    timerdev.devid,NULL,TIMER_NAME);
    if(IS_ERR(timerdev.devices)){
        return PTR_ERR(timerdev.devices);
    }

   /* 初始化 timer,设置定时器处理函数,还未设置周期,不会激活定时器 */
    init_timer(&timerdev.timer);
    timerdev.timer.function = function_timer;
    timerdev.timer.data = (unsigned long)&timerdev;  //传递的是地址的编号
    return 0;
}

static void __exit timer_exit(void)
{

    unregister_chrdev_region(timerdev.devid,1);//释放掉设备号
    cdev_del(&timerdev.cdev);
    class_destroy(timerdev.class);
    device_destroy(timerdev.class,timerdev.devid);
    gpio_free(timerdev.led_gpio);
    del_timer(&timerdev.timer);
}
module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liupeng Peng");


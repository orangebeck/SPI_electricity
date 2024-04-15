#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>

#include <linux/platform_device.h>

#include <linux/spi/spi.h>

#include <linux/err.h>
#include "include/AD7190/AD7190.h"

struct device_node *electricity_device_node;
struct spi_device *electricity_device_spi;
static dev_t electricity_devno;
#define DEVNO_START 0
#define DEVNO_NUM 1
#define DEVNO_NAME "spi_electricity"

struct cdev electricity_cdev;
struct class *electricity_class;
struct device *electricity_device;

static int electricity_open(struct inode *inode, struct file *file)
{
    int temp;
    printk("electricity_open \n");
    AD7190_init_rdy_pin( of_get_named_gpio(electricity_device_node, "spi_rdy_state", 0));

    AD7190_init(electricity_device_spi);
    
    //单次进行数据转换
    AD7190_set_power(electricity_device_spi,1);
    //AD7190_range_setup(electricity_device_spi, 0, AD7190_CONF_GAIN_1);
    AD7190_channel_select(electricity_device_spi, AD7190_CH_TEMP_SENSOR);
    temp = AD7190_single_conversion(electricity_device_spi);
    printk("the temp is %d\n",temp);

    return 0;
}
static ssize_t electricity_read(struct file *file, char __user *buf, size_t cnt, loff_t *loff)
{
    printk("electricity_read \n");
    return 0;
}
static ssize_t electricity_write(struct file *file, const char __user *buf, size_t cnt, loff_t *loff)
{
    printk("electricity_write \n");

    return 0;
}
static int electricity_release(struct inode *inode, struct file *file)
{
    gpio_free(86);
    return 0;
}

static struct file_operations electricity_cdev_ops = {
    .owner = THIS_MODULE,
    .open = electricity_open,
    .read = electricity_read,
    .write = electricity_write,
    .release = electricity_release,
};

static int electricity_probe(struct spi_device *spi)
{
    int ret = -1;

    pr_emerg("\t electricity match successed \n");

    /*获得spi节点*/
    electricity_device_node = of_find_node_by_path("/soc/spi@44009000/spi_electricity@0");
    if (electricity_device_node == NULL)
    {
        pr_emerg("\t get spi_electricity@0 failed! \n");
    }

    /*初始化spi*/
    electricity_device_spi = spi;
    electricity_device_spi->mode = SPI_MODE_3;
    electricity_device_spi->max_speed_hz = AD7190_SPI_CLK; // 规定最大spi速度为5MHz
    spi_setup(electricity_device_spi);

    /*注册cdev设备*/
    ret = alloc_chrdev_region(&electricity_devno, DEVNO_START, DEVNO_NUM, DEVNO_NAME);
    if (ret < 0)
    {
        printk("fail to alloc electricity_spi\n");
        goto alloc_err;
    }

    cdev_init(&electricity_cdev, &electricity_cdev_ops);
    ret = cdev_add(&electricity_cdev, electricity_devno, DEVNO_NUM);
    if (ret < 0)
    {
        printk("fail to add electricity dev\n");
        goto add_err;
    }

    electricity_class = class_create(THIS_MODULE, DEVNO_NAME);

    electricity_device = device_create(electricity_class, NULL, electricity_devno, NULL, DEVNO_NAME);

    return 0;
add_err:
    unregister_chrdev_region(electricity_devno, DEVNO_NUM);

alloc_err:
    return -1;
}

static int electricity_remove(struct spi_device *spi)
{
    printk("remove electricity_driver\n");
    device_destroy(electricity_class, electricity_devno);
    class_destroy(electricity_class);
    cdev_del(&electricity_cdev);
    unregister_chrdev_region(electricity_devno, DEVNO_NUM);
    return 0;
}

struct spi_device_id electricity_device_id[] = {
    {"fire,spi_electricity"},
    {}};

struct of_device_id electricity_of_device_id[] = {
    {.compatible = "fire,spi_electricity"},
    {}};

struct spi_driver electricity_driver = {
    .probe = electricity_probe,
    .remove = electricity_remove,
    .id_table = electricity_device_id,
    .driver = {
        .name = "spi_electricity",
        .owner = THIS_MODULE,
        .of_match_table = electricity_of_device_id,
    },
};

static int __init electricity_spi_init(void)
{
    int ret;
    pr_info("electricity_driver_init!\n");
    ret = spi_register_driver(&electricity_driver);
    return ret;
}

static void __exit electricity_spi_exit(void)
{
    pr_info("electricity_driver_exit!\n");
    spi_unregister_driver(&electricity_driver);
}

module_init(electricity_spi_init);
module_exit(electricity_spi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou weijie");
MODULE_DESCRIPTION("This is a spi module about AD7190");
MODULE_ALIAS("electricy spi");
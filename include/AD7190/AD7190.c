#include "AD7190.h"
#include <linux/errno.h>
#include <linux/string.h>

static int rdy_pin = -1;
static int cs_pin = -1;

int AD7190_init_rdy_pin(int pin_num){
    rdy_pin = pin_num;
    gpio_request(rdy_pin,"rdy");
    gpio_direction_input(rdy_pin);
    return rdy_pin;
}

int AD7190_init_cs_pin(int pin_num){
    cs_pin = pin_num;
    gpio_request(cs_pin,"cs");
    gpio_direction_output(cs_pin,1);
    return rdy_pin;
}

/***************************************************************************//**
 * @brief 等待rdy引脚变化，采用的是轮询形式进行，每次1ms
 *
 * @return none.
*******************************************************************************/
void AD7190_wait_rdy_go_low(void)
{
  unsigned long timeOutCnt = 0xFFFFFFFF;
	int count = 0;	//增加拉低计数，如果10ms以上就代表AD7190转换完成

	gpio_direction_output(cs_pin,0);	//进入的时候将片选置0，等待rdy
  while(timeOutCnt--)
  {
    mdelay(1);
		if (gpio_get_value(rdy_pin) == 0)
		{
			count ++;
			if (count >= 10)
			{
				break;
			}
		}
  }

	gpio_direction_output(cs_pin,1);
  if (timeOutCnt != 0)
  {
    printk("rdy go low!\n");
  }
  
}

int AD7190_set_register_value(struct spi_device *spi_device, unsigned char register_addr, unsigned int register_value, unsigned char bytes_number)
{
    int ret = -1;
    unsigned char write_commend[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char *data_pointer = (unsigned char *)&register_value;
    unsigned char bytes_nr = bytes_number;

    struct spi_message message;
    struct spi_transfer *transfer;

    write_commend[0] = AD7190_COMM_WRITE | AD7190_COMM_ADDR(register_addr);
    while (bytes_nr > 0)
    {
        write_commend[bytes_nr] = *data_pointer;
        data_pointer++;
        bytes_nr--;
    }

    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL); // kzalloc申请了一段内存并将其初始化，如果使用了kmalloc就会因为没有初始化报错
    transfer->tx_buf = write_commend;
    transfer->len = bytes_number + 1;
    spi_message_init(&message);
    spi_message_add_tail(transfer, &message);

    ret = spi_sync(spi_device, &message);
    kfree(transfer);
    if (ret != 0)
    {
        printk(KERN_ERR "Error: %d \n", ret);
        return -1;
    }
    return 0;
}

unsigned int AD7190_get_register_value(struct spi_device *spi_device, unsigned char register_addr, unsigned char bytes_number)
{
    int ret = -1;
    unsigned char register_word[] = {0x00, 0x00, 0x00, 0x00};
    unsigned char addr = 0;
    unsigned int buffer = 0;
    int i = 0;

    struct spi_message message;
    struct spi_transfer *transfer;

    addr = AD7190_COMM_READ | AD7190_COMM_ADDR(register_addr);

    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    transfer->tx_buf = &addr;
    transfer->len = 1;
    spi_message_init(&message);
    spi_message_add_tail(transfer, &message);

    ret = spi_sync(spi_device, &message);
    kfree(transfer);
    if (ret != 0)
    {
        printk("spi sync error!\n");
        return -1;
    }

    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    transfer->rx_buf = register_word;
    transfer->len = bytes_number;
    spi_message_init(&message);
    spi_message_add_tail(transfer, &message);

    ret = spi_sync(spi_device, &message);
    kfree(transfer);
    if (ret != 0)
    {
        printk("spi sync error!\n");
        return -1;
    }
    else
    {
        for (i = 0; i < bytes_number; i++)
        {
            buffer = (buffer << 8) + register_word[i];
        }
    }

    return buffer;
}

int AD7190_reset(struct spi_device *spi_device)
{
    int ret = -1;
    unsigned char register_word[7] = {
        0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    struct spi_message message;
    struct spi_transfer *transfer;

    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL); // kzalloc申请了一段内存并将其初始化，如果使用了kmalloc就会因为没有初始化报错
    transfer->tx_buf = register_word;
    transfer->len = 7;
    spi_message_init(&message);
    spi_message_add_tail(transfer, &message);

    ret = spi_sync(spi_device, &message);
    kfree(transfer);
    if (ret != 0)
    {
        printk(KERN_ERR "Error: %d \n", ret);
    }
    if (ret < 0)
    {
        printk("AD7190_reset error\n");
    }
    return ret;
}

int AD7190_init(struct spi_device *spi_device)
{
    unsigned int ret = 0;
    AD7190_reset(spi_device);
    mdelay(1);
    ret = AD7190_get_register_value(spi_device, AD7190_REG_ID, 1);
    printk("AD7190:0x%X\n", ret);
    if ((ret & AD7190_ID_MASK) == ID_AD7190)
    {
        printk("AD7190 connect successfully\n");
        ret = 0;
    }
    else
    {
        printk("AD7190 connect error\n");
        ret = -1;
    }
    return ret;
}

/*
 *@brief 设置芯片空闲模式/省电模式
 *@param mode - 选择空闲模式或者省电模式
 *                                   Example: 0 - 省电模式
 *                                                       1 - 空闲模式
 *
 *@return 0 正常
 */
int AD7190_set_power(struct spi_device *spi_device, int mode)
{
    unsigned int old_power_mode = 0;
    unsigned int new_power_mode = 0;
    int ret;

    old_power_mode = AD7190_get_register_value(spi_device, AD7190_REG_MODE, 3);
    old_power_mode &= ~(AD7190_MODE_SEL(0x07));

    new_power_mode = old_power_mode | AD7190_MODE_SEL((mode * (AD7190_MODE_IDLE)) | (!mode * (AD7190_MODE_IDLE)));
    
    new_power_mode &= ~(AD7190_MODE_CLKSRC(0x3));
    new_power_mode |= (AD7190_MODE_CLKSRC(AD7190_CLK_EXT_MCLK1_2));
    
    ret = AD7190_set_register_value(spi_device, AD7190_REG_MODE, new_power_mode, 3);
    if (ret < 0)
    {
        printk("AD7190_set_power error\n");
        return -1;
    }
    return 0;
}

/*
 *@brief 选择通道使能
 *@param channel - 选择一个通道
 *
 *@return 0 正常
 */
int AD7190_channel_select(struct spi_device *spi_device, unsigned short channel)
{
    int ret = -1;
    unsigned int old_reg_value = 0x0;
    unsigned int new_reg_value = 0x0;

    old_reg_value = AD7190_get_register_value(spi_device, AD7190_REG_CONF, 3);
    old_reg_value &= ~(AD7190_CONF_CHAN(0xff));
    new_reg_value = old_reg_value | AD7190_CONF_CHAN(channel);

    ret = AD7190_set_register_value(spi_device, AD7190_REG_CONF, new_reg_value, 3);
    if (ret < 0)
    {
        printk("AD7190_channel_select error\n");
        return -1;
    }
    return 0;
}

void AD7190_multi_channel_select(struct spi_device *spi_device, unsigned short channel)
{
    unsigned int oldRegValue = 0x0;
    unsigned int newRegValue = 0x0;

    oldRegValue = AD7190_get_register_value(spi_device, AD7190_REG_CONF, 3);
    oldRegValue &= ~(AD7190_CONF_CHAN(0xFF));
    newRegValue = oldRegValue | AD7190_CONF_CHAN(channel);
    AD7190_set_register_value(spi_device, AD7190_REG_CONF, newRegValue, 3);
}

/*
 *@brief 校准制定通道
 *@param mode - 校准模式
 *@param channel - 选择一个通道去校准
 *
 *@return 0 正常
 */
void AD7190_calibrate(struct spi_device *spi_device, unsigned char mode, unsigned char channel)
{
    unsigned int old_reg_value = 0x0;
    unsigned int new_reg_value = 0x0;

    AD7190_channel_select(spi_device, channel);
    old_reg_value = AD7190_get_register_value(spi_device, AD7190_REG_MODE, 3);
    old_reg_value &= ~(AD7190_MODE_SEL(0x7));

    new_reg_value = old_reg_value | AD7190_MODE_SEL(mode);

    AD7190_set_register_value(spi_device, AD7190_REG_MODE, new_reg_value, 3);
    AD7190_wait_rdy_go_low();
}

/***************************************************************************/
/**
 * @brief 设置斩波使能
 *
 * @param chop - chop setting
 *               Example: 0 - Disable
 *                        1 - enable
 *
 * @return none.
 *******************************************************************************/
void AD7190_chop_setting(struct spi_device *spi_device, unsigned char chop)
{
    int ret;
    unsigned int old_reg_value = 0x0;
    unsigned int new_reg_value = 0x0;

    old_reg_value = AD7190_get_register_value(spi_device, AD7190_REG_CONF, 3);
    if (chop == 1)
    {
        new_reg_value = old_reg_value | AD7190_CONF_CHOP;
    }
    else
    {
        new_reg_value = old_reg_value & (~AD7190_CONF_CHOP);
    }

    ret = AD7190_set_register_value(spi_device, AD7190_REG_CONF, new_reg_value, 3);
    if (ret < 0)
    {
        printk("AD7190_chop_setting error\n");
    }
}

/***************************************************************************/
/**
 * @brief 设置clk
 *
 * @param clk - clk setting
 *
 * @return none.
 *******************************************************************************/
void AD7190_clk_setting(struct spi_device *spi_device, unsigned char clk)
{
    int ret;
    unsigned int old_reg_value = 0x0;
    unsigned int new_reg_value = 0x0;

    old_reg_value = AD7190_get_register_value(spi_device, AD7190_REG_MODE, 3);

    new_reg_value = old_reg_value & (~(AD7190_MODE_CLKSRC(0x3)));
    if (clk != 0)
    {
        new_reg_value = old_reg_value | AD7190_MODE_CLKSRC(clk);
    }

    ret = AD7190_set_register_value(spi_device, AD7190_REG_MODE, new_reg_value, 3);
    if (ret < 0)
    {
        printk("AD7190_clk_setting error\n");
    }
}

/***************************************************************************/
/**
 * @brief 设置滤波器输出速率
 *
 * @param freq - freq setting
 *
 * @return none.
 *******************************************************************************/
void AD7190_filter_freq_setting(struct spi_device *spi_device, unsigned char freq)
{
    int ret;
    unsigned int old_reg_value = 0x0;
    unsigned int new_reg_value = 0x0;

    old_reg_value = AD7190_get_register_value(spi_device, AD7190_REG_MODE, 3);

    new_reg_value = old_reg_value & (~AD7190_MODE_RATE(0x3FF));
    
    new_reg_value = old_reg_value | AD7190_MODE_RATE(freq);

    ret = AD7190_set_register_value(spi_device, AD7190_REG_MODE, new_reg_value, 3);
    if (ret < 0)
    {
        printk("AD7190_clk_setting error\n");
    }
}

/***************************************************************************/ 
/**
  * @brief 选择转机极性及ADC输入范围
  *
  * @param polarity - Polarity select bit.
                      Example: 0 - bipolar operation is selected.
                               1 - unipolar operation is selected.
 * @param range - Gain select bits. These bits are written by the user to select
                  the ADC input range.
  *
  * @return none.
 *******************************************************************************/
void AD7190_range_setup(struct spi_device *spi_device, unsigned char polarity, unsigned char range)
{
    unsigned int oldRegValue = 0x0;
    unsigned int newRegValue = 0x0;

    oldRegValue = AD7190_get_register_value(spi_device, AD7190_REG_CONF, 3);
    oldRegValue &= ~(AD7190_CONF_UNIPOLAR | AD7190_CONF_GAIN(0x7));
    newRegValue = oldRegValue | (polarity * AD7190_CONF_UNIPOLAR) | AD7190_CONF_GAIN(range) | AD7190_CONF_BUF;
    AD7190_set_register_value(spi_device, AD7190_REG_CONF, newRegValue, 3);
}

/***************************************************************************/
/**
 * @brief Returns the result of a single conversion.
 *
 * @return regData - Result of a single analog-to-digital conversion.
 *******************************************************************************/
void AD7190_single_conversion(struct spi_device *spi_device)
{
    unsigned int command = 0x0;
    unsigned int regData = 0x0;

    command = AD7190_MODE_SEL(AD7190_MODE_SINGLE) | AD7190_MODE_CLKSRC(AD7190_CLK_EXT_MCLK1_2) | AD7190_MODE_RATE(0x060);
    AD7190_set_register_value(spi_device, AD7190_REG_MODE, command, 3);
}
                                                            
/***************************************************************************/
/**
 * @brief setting continuous read data enable or disable
 *
 * @param cread - continuous read data
 *                 Example: 0 - Disable
 *                          1 - enable
 *
 * @return none.
 *******************************************************************************/
void AD7190_continuous_readdata(struct spi_device *spi_device, unsigned char cread)
{
    int ret = -1;
    unsigned char registerWord = 0;

    struct spi_message message;
    struct spi_transfer *transfer;

    if (cread == 1)
    {
        registerWord = 0x5C;
    }
    else
    {   
        AD7190_wait_rdy_go_low();
        registerWord = 0x58;
    }

    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL); // kzalloc申请了一段内存并将其初始化，如果使用了kmalloc就会因为没有初始化报错
    transfer->tx_buf = &registerWord;
    transfer->len = 1;
    spi_message_init(&message);
    spi_message_add_tail(transfer, &message);

    ret = spi_sync(spi_device, &message);
    kfree(transfer);
    if (ret != 0)
    {
        printk(KERN_ERR "Error: %d \n", ret);
    }
}

/***************************************************************************/
/**
 * @brief Returns the average of several conversion results.
 *
 * @return samplesAverage - The average of the conversion results.
 *******************************************************************************/
unsigned int AD7190_continuous_read_average(struct spi_device *spi_device, unsigned char sampleNumber)
{
    unsigned int samplesAverage = 0x0;
    unsigned char count = 0x0;
    unsigned int command = 0x0;

    command = AD7190_MODE_SEL(AD7190_MODE_CONT) | AD7190_MODE_CLKSRC(AD7190_CLK_INT) | AD7190_MODE_RATE(0x060);
    AD7190_set_register_value(spi_device, AD7190_REG_MODE, command, 3);
    for (count = 0; count < sampleNumber; count++)
    {
       AD7190_wait_rdy_go_low();
        samplesAverage += AD7190_get_register_value(spi_device, AD7190_REG_DATA, 3);
    }
    samplesAverage = samplesAverage / sampleNumber;

    return samplesAverage;
}

/***************************************************************************/
/**
 * @brief Read data from temperature sensor and converts it to Celsius degrees.
 *
 * @return temperature - Celsius degrees.
 *******************************************************************************/
unsigned int  AD7190_temperature_read(struct spi_device *spi_device)
{
    unsigned char temperature = 0x0;
    unsigned int dataReg = 0x0;

    AD7190_range_setup(spi_device, 0, AD7190_CONF_GAIN_1);
    AD7190_channel_select(spi_device, AD7190_CH_TEMP_SENSOR);
    dataReg = AD7190_single_conversion(spi_device);
    printk("dataReg is %d\n",dataReg);
    dataReg -= 0x800000;
    dataReg /= 2815; // Kelvin Temperature
    dataReg -= 273;  // Celsius Temperature
    temperature = (unsigned int)dataReg;

    return temperature;
}

// void ad7190_unipolar_multichannel_conf(struct spi_device *spi_device)
// {
//     //unsigned int command = 0x0;
//     unsigned int temp = 0;
//     // chop enable
//     AD7190_chop_setting(spi_device, 1);

//     /* Calibrates channel AIN1(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_ZERO, AD7190_CH_AIN1P_AINCOM);
//     /* Calibrates channel AIN2(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_ZERO, AD7190_CH_AIN2P_AINCOM);
//     /* Calibrates channel AIN3(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_ZERO, AD7190_CH_AIN3P_AINCOM);
//     /* Calibrates channel AIN4(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_ZERO, AD7190_CH_AIN4P_AINCOM);

//     /* Selects unipolar operation and ADC's input range to +-Vref/1. */
//     AD7190_range_setup(spi_device, 1, AD7190_CONF_GAIN_1);
//     /* Calibrates channel AIN1(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_FULL, AD7190_CH_AIN1P_AINCOM);
//     /* Calibrates channel AIN2(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_FULL, AD7190_CH_AIN2P_AINCOM);
//     /* Calibrates channel AIN3(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_FULL, AD7190_CH_AIN3P_AINCOM);
//     /* Calibrates channel AIN4(+) - AINCOM(-). */
//     AD7190_calibrate(spi_device, AD7190_MODE_CAL_INT_FULL, AD7190_CH_AIN4P_AINCOM);

//     AD7190_multi_channel_select(spi_device, 0xF0);

//     // /* Performs a conversion. */
//     // command = AD7190_MODE_SEL(AD7190_MODE_CONT) | AD7190_MODE_DAT_STA |
//     //           AD7190_MODE_CLKSRC(AD7190_CLK_EXT_MCLK1_2) | AD7190_MODE_RATE(768);
//     // AD7190_set_register_value(spi_device, AD7190_REG_MODE, command, 3);

//     temp = AD7190_temperature_read(spi_device);
//     printk("temp = %d\n", temp);
// }

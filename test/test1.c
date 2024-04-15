#ifndef HARDWARE_AD7190_C_
#define HARDWARE_AD7190_C_
#include < AD7190.h>
#define AD7190_SPI_DEVICE_NAME "spi10" /* SPI 设备名称 */
// 和myspi.c里面驱动一致
struct rt_spi_device *spi_dev_ad7190; /* SPI 设备句柄 */
// 初始复位
void AD7190_Reset(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 复位 */
    struct rt_spi_message msg;
    rt_uint8_t sendbuf[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    msg.send_buf = sendbuf;
    msg.recv_buf = RT_NULL;
    msg.length = 16;
    msg.cs_take = 1;
    msg.cs_release = 1;
    msg.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg);
    rt_thread_mdelay(10); // 必须延迟2ms以上，再进行ad7190操作
    rt_kprintf("AD7190 reset ok\n");
}
// 读取ID值
void AD7190_ID_RD(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x60; //  ID_Reg_Read_CMD 命令， 通信寄存器写入 0110 0000
    rt_uint8_t ID_Reg = 0x00;
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = RT_NULL;
    msg2.recv_buf = &ID_Reg;
    msg2.length = 1;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 ID REG: %x\n", ID_Reg);
}
// 设置配置寄存器
void AD7190_ConfigReg_Set(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x10;                 //  ConfigReg 写入 命令， 通信寄存器写入 0001 0000
    rt_uint8_t ConfigReg[3] = {0x80, 0x02, 0x16}; // 配置寄存器写入 1000 0000、0000 0010、0001 0110 / 0x02通道选择位
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = ConfigReg;
    msg2.recv_buf = RT_NULL;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1); //
    rt_kprintf("AD7190 ConfigREG Set OK!\n");
}
// 配置寄存器更换通道
void AD7190_ConfigReg_Switch(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x10;                 //  ConfigReg 写入 命令0001 0000
    rt_uint8_t ConfigReg[3] = {0x80, 0x01, 0x17}; // 配置寄存器写入 1000 0000、0000 0001、0001 0111 / 0x01通道选择位 128增益
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = ConfigReg;
    msg2.recv_buf = RT_NULL;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1); //
    rt_kprintf("AD7190 ConfigREG Set OK!\n");
}
// 读取配置信息
void AD7190_ConfigReg_Read(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x50; //  ConfigReg 写入 命令 | 0x40, 次高位写1，读取  0101 0000
    rt_uint8_t ConfigReg[3];
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = RT_NULL;
    msg2.recv_buf = ConfigReg;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 ConfigREG read: %02X,%02X,%02X\n", ConfigReg[0], ConfigReg[1], ConfigReg[2]);
}
// 设置模式寄存器
void AD7190_ModeReg_Set(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 操作模式选择寄存器 */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x08;               // 0000 1000   ModeReg 写入 命令0X08,0x08,0x04,0x03
    rt_uint8_t ModeReg[3] = {0x08, 0x04, 0x03}; // 模式寄存器写入值0000 1000、0000 0100、0000 0011
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = ModeReg;
    msg2.recv_buf = RT_NULL;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 ModeREG Set OK!\n");
}

// 校准//calibration校准 内部零电平校准 0x88、内置满量程校准0xA8、系统零电平校准0xC8、系统满量程校准0xE8
void AD7190_Calibration(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x08;               // 0000 1000   ModeReg 写入 命令0X08,0x08,0x04,0x03
    rt_uint8_t ModeReg[3] = {0x88, 0x04, 0x03}; // 模式寄存器写入值0000 1000、0000 0100、0000 0011
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = ModeReg;
    msg2.recv_buf = RT_NULL;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 ModeREG Set OK!\n");
}
// 模式寄存器读取
void AD7190_ModeReg_Read(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x48; // 0100 1000  ModeReg 写入 命令  0X08,0x08,0x04,0x03
    rt_uint8_t ModeReg[3];
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = RT_NULL;
    msg2.recv_buf = ModeReg;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 ModeREG read: %02X,%02X,%02X\n", ModeReg[0], ModeReg[1], ModeReg[2]);
}
// 设置模式寄存器
void AD7190_GPOCONReg_Set(rt_uint8_t bpsw)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x28; // 0010 1000
    rt_uint8_t GPOCONReg = 0x00;  // GPOCON寄存器写入值
    if (bpsw == 1)
        GPOCONReg = 0x40; // 电桥打开
    else
        GPOCONReg = 0x00; //
    GPOCONReg = GPOCONReg | (bpsw << 2);
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = &GPOCONReg;
    msg2.recv_buf = RT_NULL;
    msg2.length = 1;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1);
    rt_kprintf("AD7190 GPOCON Set OK!\n");
}
// AD7190初始化
void AD7190_Init(void)
{
    AD7190_Reset();                       // 复位
    AD7190_ID_RD();                       // ID寄存器
    AD7190_ConfigReg_Set();               // 配置寄存器
    AD7190_Calibration();                 // 校准
    AD7190_ModeReg_Set();                 // 模式寄存器
    AD7190_GPOCONReg_Set(AD7190_BPSW_ON); // GPOCON寄存器（使能通用数字输出）
}
// 读取AD7190的数据寄存器
rt_uint32_t AD7190_DataReg_RD(void)
{
    spi_dev_ad7190 = (struct rt_spi_device *)rt_device_find(AD7190_SPI_DEVICE_NAME); //
    /* 发送命令读取ID */
    struct rt_spi_message msg1, msg2;
    rt_uint8_t CommRegCMD = 0x58; // 0101 1000  DATAReg 读取 命令
    rt_uint8_t DataReg[3];
    rt_uint32_t lw_ADC_value;
    msg1.send_buf = &CommRegCMD;
    msg1.recv_buf = RT_NULL;
    msg1.length = 1;
    msg1.cs_take = 1;
    msg1.cs_release = 0;
    msg1.next = &msg2;
    msg2.send_buf = RT_NULL;
    msg2.recv_buf = DataReg;
    msg2.length = 3;
    msg2.cs_take = 0;
    msg2.cs_release = 1;
    msg2.next = RT_NULL;
    rt_spi_transfer_message(spi_dev_ad7190, &msg1); //
    rt_kprintf("AD7190 DataREG read: %02X,%02X,%02X\n", DataReg[0], DataReg[1], DataReg[2]);
    lw_ADC_value = ((rt_uint32_t)DataReg[0] << 16) + ((rt_uint32_t)DataReg[1] << 8) + (rt_uint32_t)DataReg[2];
    // rt_kprintf("ADC=%d\n",lw_ADC_value);
    return lw_ADC_value;
} //=====================------------------------------
#endif /* HARDWARE_AD7190_C_ */

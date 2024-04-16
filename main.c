#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "include/AD7190/AD7190.h"

int main(int argc, char *argv[])
{
    unsigned int ret;
    struct AD7190 AD7190 = {
        .channel = AD7190_CH_AIN1P_AINCOM,
        .chop = 1,
        .clk = 0,
        .continuous = 0,
        .frequency = 480,
        .gain = AD7190_CONF_GAIN_1
    }
    printf("spi_tiny test\n");

    /*打开文件*/
    int fd = open("/dev/spi_electricitydrv", O_RDWR);
    if(fd < 0)
    {
        printf("open file : %s failed !\n", argv[0]);
        return -1;
    }

    write(fd,&AD7190,sizeof(struct AD7190));
    /*读取数据*/
    ret = read(fd,&AD7190,sizeof(struct AD7190));
    if(ret < 0)
    {
        printf("write file error! \n");
        close(fd);
        /*判断是否关闭成功*/
    }

    printf("data is %d\n",AD7190.data);
    /*关闭文件*/
    ret = close(fd);
    if(ret < 0)
    {
        printf("close file error! \n");
    }

    return 0;
}
KERNEL_DIR=../../test_linux入门/ebf_linux_kernel_mp157_depth1/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

ifneq ($(KERNELRELEASE),)
# kbuild部分的makefile
include Kbuild
endif

all:
	$(MAKE) EXTRA_CFLAGS=-fno-pic  -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE)  -C $(KERNEL_DIR) M=$(CURDIR) clean
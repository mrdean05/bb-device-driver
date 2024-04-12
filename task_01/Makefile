obj-m := hello_world_mod.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERNEL_PATH=/home/mr_dean/Desktop/embedded_linux_dev/bb_linux/linux-5.10.168-ti-rt-r69/
HOST_KERNEL_PATH=/lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD) help
host:
	make -C $(HOST_KERNEL_PATH) M=$(shell pwd) modules

host_clean:
	make -C $(HOST_KERNEL_PATH) M=$(PWD) clean
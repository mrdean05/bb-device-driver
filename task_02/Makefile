obj-m := character_device.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERNEL_PATH=/lib/modules/5.10.168/build
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
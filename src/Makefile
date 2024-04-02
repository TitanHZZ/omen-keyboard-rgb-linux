obj-m += hp-omen-wmi.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNELDIR) M=$(PWD)/src modules

clean:
	make -C $(KERNELDIR) M=$(PWD)/src clean

.PHONY: clean
obj-m := hello.o
KERDIR := /usr/src/kernels/3.10.0-693.21.1.el7.x86_64/
CURRENT_PATH := $(shell pwd)
build := kernel_modules

kernel_modules:
	$(MAKE) -C $(KERDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERDIR) M=$(CURRENT_PATH) clean


# SPDX-License-Identifier: GPL-2.0
# Please keep these build lists sorted!

# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
# If we running without kernel build system
# core driver code
	obj-m += dmabuf-exporter.o
	obj-m += dmabuf-importer.o
else
	#BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	BUILDSYSTEM_DIR ?= /home/new/baiy/kernel-head/linux-headers-5.3.0-28-generic
	PWD:=$(shell pwd)
all:
	# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules
	gcc -o dmabuf-test dmabuf-user.c

clean:
	# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
	rm -rf dma-test
endif

# SPDX-License-Identifier: GPL-2.0
# Please keep these build lists sorted!

# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
# If we running without kernel build system
# core driver code
	obj-m += dmabuf-exporter.o
	obj-m += dmabuf-importer.o
	obj-m += module-refcount.o
else
	#BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	BUILDSYSTEM_DIR ?= /home/inno/baiy/linux-headers-4.15.0-154-generic
	PWD:=$(shell pwd)
all:
	# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules
	gcc -o dmabuf-prod dmabuf-prod.c task.c -lpthread
	gcc -o dmabuf-cons dmabuf-cons.c task.c -lpthread

clean:
	# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
	rm -rf dmabuf-prod dmabuf-cons
endif

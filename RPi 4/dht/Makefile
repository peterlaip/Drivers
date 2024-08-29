# Makefile for the DHT11 kernel module

# Object file name
obj-m += dht11.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean the build files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


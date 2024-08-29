# Define the kernel build directory and current working directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Find all driver source files matching *_driver.c and add them as kernel module targets
DRIVERS := $(wildcard *_driver.c)
OBJ_MODULES := $(patsubst %.c,%.o,$(DRIVERS))

# Find all user-space source files excluding driver files (e.g., *_control.c)
USER_PROGS := $(wildcard *_control.c)

# Add driver objects to obj-m for kernel module compilation
obj-m += $(OBJ_MODULES)

# Default target to build all modules and user-space programs
all: modules $(USER_PROGS:.c=)

# Build kernel modules
modules:
	make -C $(KDIR) M=$(PWD) modules

# Clean kernel modules and user-space programs
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(USER_PROGS:.c=)

# Install all drivers found
install: $(patsubst %.o,install-%,$(OBJ_MODULES))

# Uninstall all drivers found
uninstall: $(patsubst %.o,uninstall-%,$(OBJ_MODULES))

# Reload all drivers found
reload: uninstall install

# Compile all user-space programs
%: %.c
	gcc -o $@ $<

# Install a specific driver
install-%:
	sudo insmod $*.ko

# Uninstall a specific driver
uninstall-%:
	sudo rmmod $*


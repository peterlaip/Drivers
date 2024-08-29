
obj-m += fan_driver.o
obj-m += coolerchip_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f fan_control coolerchip_control

fan_control: fan_control.c
	gcc -o fan_control fan_control.c

coolerchip_control: coolerchip_control.c
	gcc -o coolerchip_control coolerchip_control.c


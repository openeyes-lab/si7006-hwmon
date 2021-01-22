si7006-hwmon-objs := si7006.o

obj-m += si7006-hwmon.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo cp si7006-hwmon.ko /lib/modules/$(shell uname -r)/kernel/drivers/hwmon
	sudo depmod -a

load:
	sudo insmod si7006-hwmon.ko

reload:
	sudo rmmod si7006-hwmon.ko
	sudo insmod si7006-hwmon.ko

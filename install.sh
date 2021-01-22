#!/bin/bash

echo "Running script $0 with params $1"

if [ -z "$1" ]; then
	sudo apt update
	sudo apt upgrade -y
	echo "Checking headers installation"
	dpkg-query -s raspberrypi-kernel-headers
	if [ $? -eq 1 ]
	then
		sudo apt install -y raspberrypi-kernel-headers
	fi
fi

# build overlay dtbo
if dtc -@ -b 0 -I dts -O dtb -o si7006-hwmon.dtbo dts/si7006-hwmon.dts ; then
	sudo chown root:root si7006-hwmon.dtbo
	sudo mv si7006-hwmon.dtbo /boot/overlays
else
	echo "fail to compile dts"
	exit
fi

if grep -q "dtoverlay=si7006-hwmon" /boot/config.txt ; then
	echo "confi.txt already prepared"
else
	echo "dtoverlay=si7006-hwmon" | sudo tee -a /boot/config.txt
fi

cd build

make
if [ $? -ne 0 ]; then
    echo "Failed to make"
    echo -1
fi

make install
if [ $? -ne 0 ]; then
    echo "Failed to install"
    echo -1
fi

rm *.o
rm *.mod
rm modules.order
rm Module.symvers
rm *.mod.c
rm *.ko
rm .*.cmd

echo "si7006-hwmon correctly installed: reboot to make effective"

# Linux driver si7006-hwmon

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](http://www.gnu.org/licenses/lgpl-3.0)

This repository contains Linux drivers for hardware monitor module implemented
on OPEN-EYES-RPI devices from OPEN-EYES S.r.L.
This driver is licensed under the Gnu Public License.
This driver is tested under the Linux 5.X kernels.

For more information about OPEN-EYES-RPI devices visit http://www.open-eyes.it

The Si7006-hwmon Linux driver dialogues with the Silicon Labs chip Si7006 
https://www.silabs.com/documents/public/data-sheets/Si7006-A20.pdf
mounted on the OPEN-EYES-RPI multifunction access system based on Rasberry compute module CM3

The sensor measures temperature and humidity on the board.

## Manual installation

### Build instructions

Prepare Raspberry for build:
```
sudo apt update
sudo apt upgrade
sudo apt-get install raspberrypi-kernel-headers git
```
Download from git:
```
git clone https://github.com/openeyes-lab/si7006-hwmon.git
```
build driver
```
cd si7006-hwmon/build
make
make install
```

### Implement device Tree overlay

source file : dts/si7006-hwmon.dts

compile dts file and copy into /boot/overlays directory
```
dtc -@ -b 0 -I dts -O dtb -o si7006-hwmon.dtbo dts/si7006-hwmon.dts
```
change compiled file owner and move it into /boot/overlays directory
```
sudo chown root:root si7006-hwmon.dtbo
sudo mv si7006-hwmon.dtbo /boot/overlays
```
add this line
```
dtoverlay=si7006-hwmon
```
into the file /boot/config.txt

reboot

## Automatic install/uninstall

After cloning the repository;
to install driver execute:
```
cd si7006-hwmon
bash install.sh
```
to uninstall execute:
```
cd si7006-hwmon
bash uninstall.sh
```

# Interface involved

The Si7006 sensor answers on the address 0x40 of the I2C bus.

# Filesys

HWMON is created into /sys/class/hwmon/hwmon0...x directory

# Reference

## HWMON
https://www.kernel.org/doc/html/latest/hwmon/hwmon-kernel-api.html
https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface

# lm-sensors

Install:
```
sudo apt install lm-sensors
```

get value with command:
```
sensors
```

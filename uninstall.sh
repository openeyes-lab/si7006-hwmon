sudo rm /boot/overlays/si7006-hwmon.dtbo

sudo sed -i -e "/si7006/d" /boot/config.txt

sudo rm /lib/modules/$(uname -r)/kernel/drivers/hwmon/si7006-hwmon.ko

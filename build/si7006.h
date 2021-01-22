/*
 * si7006.h - Part of OPEN-EYES-II products, Linux kernel modules for hardware
 * monitoring
 *
 * Author:
 * Massimiliano Negretti <massimiliano.negretti@open-eyes.it> 2020-07-12
 *
 * Include file of sd109-hwmon Linux driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _SI7006_H
#define _SI7006_H

#define SI7006_NUM_REGS                                 256
#define ID_SI7006			                                  0x06
#define SI7006_NUM_CH_TEMP                              1

/* Si7006 register addresses */
#define SI7006_MEAS_REL_HUMIDITY_MASTER_MODE            0xE5
#define SI7006_MEAS_REL_HUMIDITY_NO_MASTER_MODE         0xF5
#define SI7006_MEAS_TEMP_MASTER_MODE                    0xE3
#define SI7006_MEAS_TEMP_NO_MASTER_MODE                 0xF3
#define SI7006_READ_OLD_TEMP                            0xE0
#define SI7006_RESET                                    0xFE
#define SI7006_WRITE_HUMIDITY_TEMP_CONTR                0xE6
#define SI7006_READ_HUMIDITY_TEMP_CONTR                 0xE7
#define SI7006_WRITE_HEATER_CONTR                       0x51
#define SI7006_READ_HEATER_CONTR                        0x11
#define SI7006_READ_ID_LOW_0                            0xFA
#define SI7006_READ_ID_LOW_1                            0x0F
#define SI7006_READ_ID_HIGH_0                           0xFC
#define SI7006_READ_ID_HIGH_1                           0xC9
#define SI7006_FIRMWARE_0                               0x84
#define SI7006_FIRMWARE_1                               0xB8

struct si7006_private {
	struct i2c_client	     *client;
  struct mutex           update_lock;
	/* Temperature registers */
	bool                   temperature_valid;
	long                   max_temperature;
	long                   temperature;
	long                   min_temperature;
	unsigned long          temperature_updated;
	/* Humidity registers */
	bool                   humidity_valid;
	long                   max_humidity;
	long                   humidity;
	long                   min_humidity;
	unsigned long          humidity_updated;
};

#endif /* _SI7006_H */

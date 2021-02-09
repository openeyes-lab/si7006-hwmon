/*
 * si7006.c - Part of OPEN-EYES-II products, Linux kernel modules for hardware
 * monitoring
 * This driver handles the Si7006 temperature and humidity sensor.
 * Author:
 * Massimiliano Negretti <massimiliano.negretti@open-eyes.it> 2020-07-12
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

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/hwmon.h>
#include "si7006.h"

const struct regmap_config si7006_regmap_config = {
	.max_register = SI7006_NUM_REGS - 1,
};

/**
 * @brief HWMON function to get temperature
 * @param [in] dev struct device pointer
 * @param [in] data struct si7006_private pointer
 * @param [out] val temperature value
 * @return 0 if success
 * @details Returns the temperature measured from Si7006 sensor.
 */
static int si7006_get_master_temperature(struct device *dev,
				      struct si7006_private *data, long *val)
{
	char buf[6];
	int  error;
	int code;
	long long temp;

	/* Put the 2-byte command into the buffer */
	buf[0] = SI7006_MEAS_TEMP_MASTER_MODE;

	/* Send the command */
	error = i2c_master_send(data->client, buf, 1);
	if (error < 0)
				 return error;

	/* Receive the 2-byte result */
	error = i2c_master_recv(data->client, buf, 2);
	if (error < 0)
		return error;

	code = buf[1] + buf[0]*256;
	temp = ((long long)(code)*175720)/65536-46850;
	*val = (long)temp;

	return 0;
}

/**
 * @brief HWMON function to get humidity
 * @param [in] dev struct device pointer
 * @param [in] data struct si7006_private pointer
 * @param [out] val humidity value
 * @return 0 if success
 * @details Returns the humidity measured from Si7006 sensor.
 */
static int si7006_get_master_humidity(struct device *dev,
				      struct si7006_private *data, long *val)
{
	char buf[6];
	int  error;
	int code;
	long long temp;

	/* Put the 1-byte command into the buffer */
	buf[0] = SI7006_MEAS_REL_HUMIDITY_MASTER_MODE;

	/* Send the command */
	error = i2c_master_send(data->client, buf, 1);
	if (error < 0)
				 return error;

	/* Receive the 2-byte result */
	error = i2c_master_recv(data->client, buf, 2);
	if (error < 0)
		return error;

	code = buf[1] + buf[0]*256;

	temp = ((long long)(code)*125000)/65536-6000;
	*val = (long)temp;

	return 0;
}

/**
 * @brief HWMON function to get temperature
 * @param [in] dev struct device pointer
 * @return 0 if success
 * @details Returns the temperature from the given register in milli celsius
 * handling mutex and avoid to address sensor when measure are made close
 * in time.
 */
static long si7006_get_temperature(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);
	long temperature=0;
	int ret;

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->temperature_updated + HZ)
																					|| !data->temperature_valid) {

		ret = si7006_get_master_temperature(dev, data, &temperature);

		if (ret < 0) {
			goto error;
		}

		data->temperature=temperature;
		data->temperature_updated = jiffies;
		if (data->temperature_valid) {
			if (temperature>data->max_temperature)
				data->max_temperature = temperature;
			if (temperature<data->min_temperature)
				data->min_temperature = temperature;
		} else {
			data->min_temperature = temperature;
			data->max_temperature = temperature;
			data->temperature_valid = true;
		}
	} else {
		temperature = data->temperature;
	}

error:
	mutex_unlock(&data->update_lock);
	return temperature;
}

/**
 * @brief HWMON function to get maximum temperature
 * @param [in] dev struct device pointer
 * @return maximum measured temperature
 * @details The function doesn't make any real measure, but simply returns the
 * maximum temperature read by the sensor.
 */
static long si7006_get_temperature_max(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);

	return data->max_temperature;
}

/**
 * @brief HWMON function to get minimum temperature
 * @param [in] dev struct device pointer
 * @return maximum measured temperature
 * @details The function doesn't make any real measure, but simply returns the
 * minimum temperature read by the sensor.
 */
static long si7006_get_temperature_min(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);

	return data->min_temperature;
}

/**
 * @brief HWMON function to get humidity
 * @param [in] dev struct device pointer
 * @return 0 if success
 * @details Returns the humidity from the given register in milli %HR
 * handling mutex and avoid to address sensor when measure are made close
 * in time.
 */
static long si7006_get_humidity(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);
	long humidity=0;
	int ret;

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->humidity_updated + HZ)
																					|| !data->humidity_valid) {

		ret = si7006_get_master_humidity(dev, data, &humidity);

		if (ret < 0) {
			goto error;
		}

		data->humidity=humidity;
		data->humidity_updated = jiffies;
		if (data->humidity_valid) {
			if (humidity>data->max_humidity)
				data->max_humidity = humidity;
			if (humidity<data->min_humidity)
				data->min_humidity = humidity;
		} else {
			data->min_humidity = humidity;
			data->max_humidity = humidity;
			data->humidity_valid = true;
		}
	} else {
		humidity = data->humidity;
	}

error:
	mutex_unlock(&data->update_lock);
	return humidity;
}

/**
 * @brief HWMON function to get maximum humidity
 * @param [in] dev struct device pointer
 * @return maximum measured temperature
 * @details The function doesn't make aby real measure, but simply returns the
 * maximum humidity read by the sensor.
 */
static long si7006_get_humidity_max(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);

	return data->max_humidity;
}

/**
 * @brief HWMON function to get minimum humidity
 * @param [in] dev struct device pointer
 * @return maximum measured temperature
 * @details The function doesn't make any real measure, but simply returns the
 * minimum humidity read by the sensor.
 */
static long si7006_get_humidity_min(struct device *dev)
{
	struct si7006_private *data = dev_get_drvdata(dev);

	return data->min_humidity;
}

/**
 * @brief HWMON temperature read method
 * @param [in] dev struct device pointer
 * @param [in] attr attribute
 * @param [in] channel
 * @param [out] val pointer
 * @return 0 if success
 * @details Select what temperature is requested (current/max/min)
 * and return it.
 */
static int si7006_read_temperature(struct device *dev, u32 attr,
			int channel, long *val)
{
	switch (attr) {
		case hwmon_temp_input:
			if (channel < SI7006_NUM_CH_TEMP)
				*val = si7006_get_temperature(dev);
			else
				return -EOPNOTSUPP;
			return 0;
		case hwmon_temp_max:
				if (channel < SI7006_NUM_CH_TEMP)
					*val = si7006_get_temperature_max(dev);
				else
					return -EOPNOTSUPP;
				return 0;
		case hwmon_temp_min:
				if (channel < SI7006_NUM_CH_TEMP)
					*val = si7006_get_temperature_min(dev);
				else
					return -EOPNOTSUPP;
				return 0;
		default:
			return -EOPNOTSUPP;
	}
}

/**
 * @brief HWMON humidity read method
 * @param [in] dev struct device pointer
 * @param [in] attr attribute
 * @param [in] channel
 * @param [out] val pointer
 * @return 0 if success
 * @details Select what humidity is requested (current/max/min)
 * and return it.
 */
static int si7006_read_humidity(struct device *dev, u32 attr,
			int channel, long *val)
{
	switch (attr) {
		case hwmon_humidity_input:
			if (channel < SI7006_NUM_CH_TEMP)
				*val = si7006_get_humidity(dev);
			else
				return -EOPNOTSUPP;
			return 0;
		case hwmon_humidity_max:
				if (channel < SI7006_NUM_CH_TEMP)
					*val = si7006_get_humidity_max(dev);
				else
					return -EOPNOTSUPP;
				return 0;
		case hwmon_humidity_min:
				if (channel < SI7006_NUM_CH_TEMP)
					*val = si7006_get_humidity_min(dev);
				else
					return -EOPNOTSUPP;
				return 0;
		default:
			return -EOPNOTSUPP;
	}
}

/* HWMON input read ops */
/**
 * @brief HWMON Si7006 read method
 * @param [in] dev struct device pointer
 * @param [in] type struct hwmon_sensor_types pointer
 * @param [in] attr attribute
 * @param [in] channel
 * @param [out] val pointer
 * @return 0 if success
 * @details Select between temperature and humidity and run the measure.
 */
static int si7006_read(struct device *dev, enum hwmon_sensor_types type,
			u32 attr, int channel, long *val)
{
	switch (type) {
		case hwmon_temp:
			return si7006_read_temperature(dev, attr, channel, val);
		case hwmon_humidity:
			return si7006_read_humidity(dev, attr, channel, val);
		default:
			return -EOPNOTSUPP;
	}
}

/**
 * @brief HWMON function return channel name
 * @param [in] dev struct device pointer
 * @param [in] type enum hwmon_sensor_types
 * @param [in] attr attribute
 * @param [in] channel
 * @param [out] str string pointer
 * @return 0 if success.
 * @details Return the label of channel.
 */
static int si7006_read_string(struct device *dev, enum hwmon_sensor_types type,
		       u32 attr, int channel, const char **str)
{
	switch (type) {
		case hwmon_temp:
			*str = "BOARD TEMP";
			return 0;
		case hwmon_humidity:
			*str = "BOARD HR";
			return 0;
		default:
			return -EOPNOTSUPP;
	}
	return -EOPNOTSUPP;
}

/**
 * @brief HWMON function return access attribute
 * @param [in] data unused
 * @param [in] type enum hwmon_sensor_types
 * @param [in] attr attribute
 * @param [in] channel
 * @return permission.
 * @details Returns file access permission
 */
static umode_t si7006_is_visible(const void *data, enum hwmon_sensor_types type,
			u32 attr, int channel)
{
	switch (type) {
		case hwmon_temp:
			switch (attr) {
				case hwmon_temp_input:
				case hwmon_temp_max:
				case hwmon_temp_min:
					return S_IRUGO;
				default:
					break;
			}
			break;
		case hwmon_humidity:
			switch (attr) {
				case hwmon_humidity_input:
				case hwmon_humidity_max:
				case hwmon_humidity_min:
					return S_IRUGO;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return 0;
}

/****************************************************************************
 * HWMON STRUCTURES
 ****************************************************************************/

static const u32 si7006_temperature_config[] = {
	(HWMON_T_INPUT|HWMON_I_LABEL|HWMON_T_MAX|HWMON_T_MIN),
	0
};

static const struct hwmon_channel_info si7006_temperature = {
	.type = hwmon_temp,
	.config = si7006_temperature_config,
};

static const u32 si7006_humidity_config[] = {
	(HWMON_H_INPUT|HWMON_I_LABEL|HWMON_H_MAX|HWMON_H_MIN),
	0
};

static const struct hwmon_channel_info si7006_humidity = {
	.type = hwmon_humidity,
	.config = si7006_humidity_config,
};

static const struct hwmon_channel_info *si7006_info[] = {
	&si7006_temperature,
	&si7006_humidity,
	NULL
};

static const struct hwmon_ops si7006_hwmon_ops = {
	.is_visible = si7006_is_visible,
	.read = si7006_read,
	.read_string = si7006_read_string,
};

static const struct hwmon_chip_info si7006_chip_info = {
	.ops = &si7006_hwmon_ops,
	.info = si7006_info,
};

/****************************************************************************
 * PROBE SUPPORT FUNCTIONS
 ****************************************************************************/

static int si7006_get_device_id(struct i2c_client *client, int *id)
{
	char buf[6];
	int  error;

	/* Put the 2-byte command into the buffer */
	buf[0] = 0xFC;
	buf[1] = 0xC9;

	/* Send the command */
	error = i2c_master_send(client, buf, 2);
	if (error < 0)
	       return error;

	/* Receive the 6-byte result */
	error = i2c_master_recv(client, buf, 6);
	if (error < 0)
		return error;

	/* Return the device ID */
	*id = buf[0];

	return 0;  /* Success */
}

/****************************************************************************
 * Si7006 PROBE
 ****************************************************************************/

static int si7006_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct si7006_private *data;
	struct device *hwmon_dev;
	int chip_id=0;

	data = devm_kzalloc(dev, sizeof(struct si7006_private),GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	dev_set_drvdata(dev, data);

	mutex_init(&data->update_lock);

	/* Verify that we have a si7006 */
	si7006_get_device_id(client,&chip_id);
	if (chip_id!=ID_SI7006) {
		dev_err(dev, "Si7006 not found");
		return -ENXIO;
	}

	data->client = client;

	hwmon_dev = devm_hwmon_device_register_with_info(dev, client->name,
							 data, &si7006_chip_info, NULL);

	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	dev_info(dev, "%s: sensor '%s'\n", dev_name(hwmon_dev), client->name);

	return 0;
}

static int si7006_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id si7006_id[] = {
	{ "si7006", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, si7006_id);


static struct i2c_driver si7006_i2c_driver = {
		.class		= I2C_CLASS_HWMON,
		.driver = {
			.name = "si7006",
		},
		.probe    = si7006_i2c_probe,
		.remove	  = si7006_remove,
		.id_table = si7006_id,
};

module_i2c_driver(si7006_i2c_driver);

MODULE_DESCRIPTION("HWMON Si7006 driver");
MODULE_AUTHOR("Massimiliano Negretti <massimiliano.negretti@open-eyes.it>");
MODULE_LICENSE("GPL");

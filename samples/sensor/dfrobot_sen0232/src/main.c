/*
 * Copyright (c) 2021 Henrik Brix Andersen <henrik@brixandersen.dk>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/sensor/dfrobot_sen0232.h>
#include <logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

/* Sound pressure level sample interval */
#define SAMPLE_INTERVAL K_MSEC(125)

void main(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_INST(0, dfrobot_sen0232));
	struct sensor_value val;
	int err;

	if (!device_is_ready(dev)) {
		LOG_ERR("dfrobot sen0232 device not ready");
		return;
	}

	while (true) {
		k_sleep(SAMPLE_INTERVAL);

		err = sensor_sample_fetch(dev);
		if (err) {
			LOG_ERR("failed to fetch sensor sample (err %d)", err);
			continue;
		}

		err = sensor_channel_get(dev, SENSOR_CHAN_SOUND_PRESSURE_LEVEL_DBA,
					 &val);
		if (err) {
			LOG_ERR("failed to get sensor sample (err %d)", err);
			continue;
		}

		printf("Sound Level (LAF): %.03f dB(A)\n",
			sensor_value_to_double(&val));
	}
}

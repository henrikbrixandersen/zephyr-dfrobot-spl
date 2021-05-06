/*
 * Copyright (c) 2021 Henrik Brix Andersen <henrik@brixandersen.dk>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Extended public API for the DFRobot SEN0232 Sound Level Meter
 */

#ifndef ZEPHYR_DFROBOT_SEN0232_INCLUDE_DRIVERS_SENSOR_DFROBOT_SEN0232_H_
#define ZEPHYR_DFROBOT_SEN0232_INCLUDE_DRIVERS_SENSOR_DFROBOT_SEN0232_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <drivers/sensor.h>

enum sensor_channel_dfrobot_sen0232 {
	/** Sound pressure level in A-weighted decibels (dB(A)). */
	SENSOR_CHAN_SOUND_PRESSURE_LEVEL_DBA = SENSOR_CHAN_PRIV_START,
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DFROBOT_SEN0232_INCLUDE_DRIVERS_SENSOR_DFROBOT_SEN0232_H_ */

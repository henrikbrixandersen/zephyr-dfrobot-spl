/*
 * Copyright (c) 2021 Henrik Brix Andersen <henrik@brixandersen.dk>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT dfrobot_sen0232

#include <device.h>
#include <drivers/adc.h>
#include <drivers/sensor.h>
#include <drivers/sensor/dfrobot_sen0232.h>
#include <logging/log.h>

/* TODO */
#include <hal/nrf_saadc.h>

LOG_MODULE_REGISTER(dfrobot_sen0232, CONFIG_SENSOR_LOG_LEVEL);

/* Scaling factor for converting from voltage to A-weighted decibels */
#define DFROBOT_SEN0232_DBA_SCALING_FACTOR 50

/* TODO: ADC gain factor */
#define DFROBOT_SEN0232_ADC_GAIN ADC_GAIN_1_6

struct dfrobot_sen0232_config {
	const struct device *adc;
	struct adc_sequence adc_sequence;
	int32_t adc_reference;
	uint8_t adc_channel;
};

struct dfrobot_sen0232_data {
	uint16_t buffer;
};

static int dfrobot_sen0232_sample_fetch(const struct device *dev,
					enum sensor_channel chan)
{
	const struct dfrobot_sen0232_config *config = dev->config;
	struct dfrobot_sen0232_data *data = dev->data;
	int err;

	if (chan != SENSOR_CHAN_ALL &&
	    (uint16_t)chan != SENSOR_CHAN_SOUND_PRESSURE_LEVEL_DBA) {
		return -ENOTSUP;
	}

	err = adc_read(config->adc, &config->adc_sequence);
	if (err) {
		LOG_ERR("failed to read ADC channel (err %d)", err);
		return err;
	}

	LOG_DBG("ADC %u", data->buffer);

	return 0;
}

static int dfrobot_sen0232_channel_get(const struct device *dev,
				       enum sensor_channel chan,
				       struct sensor_value *val)
{
	const struct dfrobot_sen0232_config *config = dev->config;
	struct dfrobot_sen0232_data *data = dev->data;
	int32_t millivolts = data->buffer;
	int err;

	if ((uint16_t)chan != SENSOR_CHAN_SOUND_PRESSURE_LEVEL_DBA) {
		return -ENOTSUP;
	}

	err = adc_raw_to_millivolts(config->adc_reference, DFROBOT_SEN0232_ADC_GAIN,
				    config->adc_sequence.resolution, &millivolts);
	if (err) {
		LOG_ERR("failed to convert ADC value to millivolts (err %d)", err);
		return err;
	}

	LOG_DBG("%d mV", millivolts);

	val->val1 = (millivolts * DFROBOT_SEN0232_DBA_SCALING_FACTOR) / 1000;
	val->val2 = ((millivolts * DFROBOT_SEN0232_DBA_SCALING_FACTOR) % 1000) * 1000;

	return 0;
}

static int dfrobot_sen0232_init(const struct device *dev)
{
	const struct dfrobot_sen0232_config *config = dev->config;
	const struct adc_channel_cfg adc_channel_cfg = {
		.gain = DFROBOT_SEN0232_ADC_GAIN,
		.reference = ADC_REF_INTERNAL,
		.acquisition_time = ADC_ACQ_TIME_DEFAULT,
		.channel_id = config->adc_channel,
		.differential = 0,
		/* TODO: support nrfx etc. */
		.input_positive = NRF_SAADC_INPUT_AIN4,
	};
	int err;

	if (!device_is_ready(config->adc)) {
		LOG_ERR("ADC device is not ready");
		return -EINVAL;
	}

	err = adc_channel_setup(config->adc, &adc_channel_cfg);
	if (err) {
		LOG_ERR("failed to configure ADC channel (err %d)", err);
		return err;
	}

	return 0;
}

static const struct sensor_driver_api dfrobot_sen0232_driver_api = {
	.sample_fetch = dfrobot_sen0232_sample_fetch,
	.channel_get = dfrobot_sen0232_channel_get,
};

#define DFROBOT_SEN0232_INIT(inst)					\
	static struct dfrobot_sen0232_data dfrobot_sen0232_data_##inst; \
									\
	static const struct dfrobot_sen0232_config dfrobot_sen0232_config_##inst = { \
		.adc = DEVICE_DT_GET(DT_INST_IO_CHANNELS_CTLR(inst)),	\
		.adc_sequence = {					\
			.options = NULL,				\
			.channels = BIT(DT_INST_IO_CHANNELS_INPUT(inst)), \
			.buffer = &dfrobot_sen0232_data_##inst.buffer,	\
			.buffer_size = sizeof(dfrobot_sen0232_data_##inst.buffer), \
			.resolution = CONFIG_DFROBOT_SEN0232_RESOLUTION, \
			.oversampling = 0,				\
			.calibrate = false,				\
		},							\
		.adc_reference = DT_INST_PROP(inst, reference_voltage),	\
		.adc_channel = DT_INST_IO_CHANNELS_INPUT(inst),		\
	};								\
									\
	DEVICE_DT_INST_DEFINE(inst, dfrobot_sen0232_init,		\
			      NULL, &dfrobot_sen0232_data_##inst,	\
			      &dfrobot_sen0232_config_##inst, POST_KERNEL, \
			      CONFIG_SENSOR_INIT_PRIORITY,		\
			      &dfrobot_sen0232_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DFROBOT_SEN0232_INIT)

#include "bme280.h"
#include "bme280_add.h"
#include "bme280_defs.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"  // for huart2

static struct bme280_dev bme;
static struct bme280_data comp_data;
static int8_t init_done;

int8_t BME280_init(void) {
	int8_t rslt = BME280_OK;
	uint8_t settings_sel;
	init_done = BME280_E_DEV_NOT_FOUND;

	bme.dev_id = (BME280_I2C_ADDR_PRIM << 1);
	bme.intf = BME280_I2C_INTF;
	bme.read = i2c_read;
	bme.write = i2c_write;
	bme.delay_ms = delay_ms;

	rslt = bme280_init(&bme);

	bme.settings.osr_h = BME280_OVERSAMPLING_1X;
	bme.settings.osr_p = BME280_OVERSAMPLING_16X;
	bme.settings.osr_t = BME280_OVERSAMPLING_2X;
	bme.settings.filter = BME280_FILTER_COEFF_16;
	bme.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	if (rslt == BME280_OK) {
		settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL |
		               BME280_OSR_HUM_SEL | BME280_STANDBY_SEL | BME280_FILTER_SEL;

		rslt = bme280_set_sensor_settings(settings_sel, &bme);

		if (rslt == BME280_OK) {
			rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme);
			init_done = rslt;
		}
	}

	return rslt;
}

int8_t BME280_read_data(void) {
	int8_t rslt = BME280_E_COMM_FAIL;

	if (init_done == BME280_OK) {
		rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &bme);
		bme.delay_ms(100);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		print_sensor_data(&comp_data);
	}

	return rslt;
}

void print_sensor_data(struct bme280_data *comp_data) {
#ifdef BME280_FLOAT_ENABLE
	char message[128];
	snprintf(message, sizeof(message),
		"\r\n--- BME280 Sensor Data ---\r\n"
		"Temperature: %.2f °C\r\n"
		"Pressure: %.2f hPa\r\n"
		"Humidity: %.2f %%\r\n",
		comp_data->temperature,
		comp_data->pressure / 100.0f,
		comp_data->humidity);

	HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
#else
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nTemperature:\r\n", 16, HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nPressure:\r\n", 14, HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nHumidity:\r\n", 14, HAL_MAX_DELAY);
#endif
}

void delay_ms(uint32_t period) {
	HAL_Delay(period);
}

int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	return HAL_I2C_Mem_Read(&hi2c3, dev_id, reg_addr, 1, reg_data, len, 100) == HAL_OK ? 0 : -1;
}

int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	return HAL_I2C_Mem_Write(&hi2c3, dev_id, reg_addr, 1, reg_data, len, 100) == HAL_OK ? 0 : -1;
}

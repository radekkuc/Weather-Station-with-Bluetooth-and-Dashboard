#include "adc.h"
#include "mq9.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"  // for huart2

#define RL 10.0f        // obciążenie do kalibracji
#define VREF 3.3f       // napięcie do kalibracji
#define ADC_RES 4095.0f // 12-bit ADC

extern ADC_HandleTypeDef hadc1;

static float R0 = 1.0f;

int8_t MQ9_calbration(void) {
    float sensorValue = 0.0f;
    int currentValue;
    char buffer[64];

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 30);

    for (int x = 0; x < 10; x++) {
        HAL_ADC_Start(&hadc1);
        currentValue = HAL_ADC_GetValue(&hadc1);
        sensorValue += currentValue;

        snprintf(buffer, sizeof(buffer), "\r\nMQ-9 calibration: %d", currentValue);
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
        HAL_Delay(1000);
    }

    sensorValue /= 10.0;
    currentValue = sensorValue;

    snprintf(buffer, sizeof(buffer), "\r\nMQ-9 after calibration: %d", currentValue);
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

    HAL_ADC_Stop(&hadc1);

    float voltage = (sensorValue * VREF) / ADC_RES;
    float rs = RL * (VREF - voltage) / voltage;
    R0 = rs / 10.0f;

    return 0;
}

int8_t MQ9_read_data(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 30);
    float sensorValue = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    float voltage = (sensorValue * VREF) / ADC_RES;
    float rs = RL * (VREF - voltage) / voltage;
    float ratio = rs / R0;

    float ppm = 1000 * powf(ratio, -1.8f);
    MQ9_print_sensor_data(ppm, ratio);
    return 0;
}

void MQ9_print_sensor_data(float ppm_val, float ratio_val) {
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "\r\nThe estimated value based on Rs/R0=%.2f is: %.2f ppm", ratio_val, ppm_val);
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

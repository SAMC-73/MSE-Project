/**
 * @file lm35_driver.h
 * @brief LM35 temperature sensor driver for STM32F411RE.
 *
 *
 * @author  Melanie Picen
 * @date    2026-05-17
 */

#ifndef LM35_DRIVER_H
#define LM35_DRIVER_H

#include <stdint.h>
#include "adc_driver.h"
#include "GPIO_stm32.h"

#define LM35_GPIO_PORT         GPIO_PORT_A
#define LM35_GPIO_PIN          (1U)
#define LM35_ADC_CHANNEL       (1U)
#define LM35_NUM_SAMPLES       (64U)
#define LM35_ADC_FULL_SCALE    (4095U)
#define LM35_VREF_MV           (3300U)

typedef enum
{
    LM35_OK             = 0,
    LM35_ERROR_NULL_PTR = 1,
    LM35_ERROR_ADC      = 2,
    LM35_ERROR_GPIO     = 3
} LM35_Status_t;

typedef struct
{
    uint32_t ambientOffsetX10;
} LM35_CalData_t;

LM35_Status_t lm35_init(Adc_Instance_t instance);
LM35_Status_t lm35_calibrate(Adc_Instance_t instance, uint32_t ambientTempX10, LM35_CalData_t *calData);
LM35_Status_t lm35_read(Adc_Instance_t instance, const LM35_CalData_t *calData, uint32_t *tempX10);

#endif
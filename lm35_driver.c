/**
 * @file lm35_driver.c
 * @brief LM35 temperature sensor driver implementation for STM32F411RE.
 *
 * @author  Melanie Picen 
 * @date    2026-05-17
 */

#include "lm35_driver.h"
#include "serial.h"

static uint32_t lm35_averageSamples(Adc_Instance_t instance, uint32_t nSamples)
{
    uint32_t accumulator = 0U;
    uint32_t i;

    for (i = 0U; i < nSamples; i++)
    {
        ADC1->ADC_SR = 0U;
        ADC1->ADC_CR2 |= (1U << 30);

        volatile uint32_t timeout = 2000000U;
        while (!(ADC1->ADC_SR & (1U << 1)) && timeout > 0U)
        {
            timeout--;
        }

        if (timeout > 0U)
        {
            accumulator += (uint32_t)(ADC1->ADC_DR & 0x0FFFU);
        }
    }

    return (accumulator / nSamples);
}

static uint32_t lm35_rawToCelsiusX10(uint32_t rawAvg)
{
    /* temp * 10 = raw * 33 / 4095 */
    return ((rawAvg * 3300U) / 4095U);
}

LM35_Status_t lm35_init(Adc_Instance_t instance)
{
    GPIO_Status_t gpioStatus;
    ADC_Status_t  adcStatus;

    gpioStatus = gpio_initPort(LM35_GPIO_PORT);
    if (gpioStatus != GPIO_OK)
    {
        return LM35_ERROR_GPIO;
    }

    gpioStatus = gpio_setPinMode(LM35_GPIO_PORT, LM35_GPIO_PIN, GPIO_MODE_ANALOG);
    if (gpioStatus != GPIO_OK)
    {
        return LM35_ERROR_GPIO;
    }

    adcStatus = adc_init();
    if (adcStatus != ADC_OK)
    {
        return LM35_ERROR_ADC;
    }

    adcStatus = adc_setChannel(instance, LM35_ADC_CHANNEL);
    if (adcStatus != ADC_OK)
    {
        return LM35_ERROR_ADC;
    }

    adcStatus = adc_enableAdc(instance);
    if (adcStatus != ADC_OK)
    {
        return LM35_ERROR_ADC;
    }

    return LM35_OK;
}

LM35_Status_t lm35_calibrate(Adc_Instance_t  instance,
                              uint32_t        ambientTempX10,
                              LM35_CalData_t *calData)
{
    uint32_t rawAvg;
    uint32_t measuredX10;

    if (calData == ((void *)0))
    {
        return LM35_ERROR_NULL_PTR;
    }

    rawAvg      = lm35_averageSamples(instance, LM35_NUM_SAMPLES);
    measuredX10 = lm35_rawToCelsiusX10(rawAvg);

    /* DEBUG */
    //serial_printf("rawAvg: %d\n", (int)rawAvg);
    //serial_printf("measuredX10: %d\n", (int)measuredX10);

    if (measuredX10 >= ambientTempX10)
    {
        calData->ambientOffsetX10 = measuredX10 - ambientTempX10;
    }
    else
    {
        calData->ambientOffsetX10 = 0U;
    }

    return LM35_OK;
}

LM35_Status_t lm35_read(Adc_Instance_t        instance,
                         const LM35_CalData_t *calData,
                         uint32_t             *tempX10)
{
    uint32_t rawAvg;
    uint32_t rawTempX10;

    if ((calData == ((void *)0)) || (tempX10 == ((void *)0)))
    {
        return LM35_ERROR_NULL_PTR;
    }

    rawAvg     = lm35_averageSamples(instance, LM35_NUM_SAMPLES);
    rawTempX10 = lm35_rawToCelsiusX10(rawAvg);

    if (rawTempX10 >= calData->ambientOffsetX10)
    {
        *tempX10 = rawTempX10 - calData->ambientOffsetX10;
    }
    else
    {
        *tempX10 = 0U;
    }

    return LM35_OK;
}
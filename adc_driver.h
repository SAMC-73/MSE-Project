/**
 * @file    adc_driver.h
 * @brief   ADC Driver - Software Requirements Specification v1.0
 * @author  Adrián Rafael Amaro Avala
 *          Leonardo Ivan García Espinosa 
 *          Mario Morales Vega
 *
 * @details Provides a standardized API for initializing ADC peripherals,
 *          configuring sampling channels, and retrieving digital conversion
 *          results from analog inputs on STM32 microcontrollers.
 *
 * @note    Target: STM32F411RE (RM0383)
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* =========================================================================
 * Type Definitions
 * ========================================================================= */

/**
 * @brief Supported ADC instances.
 */

typedef struct 
{
    uint32_t ADC_SR;      /**< ADC status register */
    uint32_t ADC_CR1;     /**< ADC control register 1 */
    uint32_t ADC_CR2;     /**< ADC control register 2 */
    uint32_t ADC_SMPR1;   /**< ADC sample time register 1 */
    uint32_t ADC_SMPR2;   /**< ADC sample time register 2 */
    uint32_t ADC_JOFR1;   /**< ADC injected channel data offset register 1 */
    uint32_t ADC_JOFR2;   /**< ADC injected channel data offset register 2 */
    uint32_t ADC_JOFR3;   /**< ADC injected channel data offset register 3 */
    uint32_t ADC_JOFR4;   /**< ADC injected channel data offset register 4 */
    uint32_t ADC_HTR;     /**< ADC watchdog higher threshold register */
    uint32_t ADC_LTR;     /**< ADC watchdog lower threshold register */
    uint32_t ADC_SQR1;    /**< ADC regular sequence register 1 */
    uint32_t ADC_SQR2;    /**< ADC regular sequence register 2 */
    uint32_t ADC_SQR3;    /**< ADC regular sequence register 3 */
    uint32_t ADC_JSQR;    /**< ADC injected sequence register */
    uint32_t ADC_JDR1;    /**< ADC injected data register 1 */
    uint32_t ADC_JDR2;    /**< ADC injected data register 2 */
    uint32_t ADC_JDR3;    /**< ADC injected data register 3 */
    uint32_t ADC_JDR4;    /**< ADC injected data register 4 */
    uint32_t ADC_DR;      /**< ADC regular data register */

} Adc_Registers;

typedef Adc_Registers ADC_RegDef_t; /**< Alias para compatibilidad con otros módulos */

typedef struct 
{
    uint32_t CSR;      /**< ADC common status register */
    uint32_t CCR;      /**< ADC common control register */

} Adc_Common_Registers;

#define ADC1 ((Adc_Registers *)0x40012000u) /**< Base address for ADC1 registers */
#define ADC_COMMON ((Adc_Common_Registers *)0x40012300u +  0x00u) /**< Base address for ADC common registers */

typedef enum
{
    ADC_INSTANCE_1 = 0u, /**< ADC1 peripheral */
    ADC_INSTANCE_2 = 1u,      /**< ADC2 peripheral */
    ADC_INSTANCE_3 = 2u,      /**< ADC3 peripheral */
    ADC_INSTANCE_MAX = 3u     /**< Sentinel - do not use */
} Adc_Instance_t;

/**
 * @brief Regular and injected ADC channel identifiers.
 */
typedef enum
{
    ADC_CHANNEL_0  = 0u,
    ADC_CHANNEL_1 = 1u,
    ADC_CHANNEL_2 = 2u,
    ADC_CHANNEL_3 = 3u,
    ADC_CHANNEL_4 = 4u,
    ADC_CHANNEL_5 = 5u,
    ADC_CHANNEL_6 = 6u,
    ADC_CHANNEL_7 = 7u,
    ADC_CHANNEL_8 = 8u,
    ADC_CHANNEL_9 = 9u,
    ADC_CHANNEL_10 = 10u,
    ADC_CHANNEL_11 = 11u,
    ADC_CHANNEL_12 = 12u,
    ADC_CHANNEL_13 = 13u,
    ADC_CHANNEL_14 = 14u,
    ADC_CHANNEL_15 = 15u,
    ADC_CHANNEL_16 = 16u,
    ADC_CHANNEL_17 = 17u,
    ADC_CHANNEL_18 = 18u,
    ADC_CHANNEL_MAX = 19u /**< Sentinel - do not use */
} Adc_Channel_t;

/**
 * @brief Injected channel slot index (1-4 as per STM32 hardware).
 */
typedef enum
{
    ADC_INJECTED_RANK_1 = 1u,
    ADC_INJECTED_RANK_2 = 2u,
    ADC_INJECTED_RANK_3 = 3u,
    ADC_INJECTED_RANK_4 = 4u
} Adc_InjectedRank_t;

/**
 * @brief Return status codes (NFR-2: error handling).
 */
typedef enum
{
    ADC_OK = 0u,         /**< Operation successful          */
    ADC_ERROR_INVALID = 1u,   /**< Invalid instance or channel   */
    ADC_ERROR_TIMEOUT = 2u,   /**< Conversion timed out          */
    ADC_ERROR_BUSY = 3u       /**< ADC already in use            */
} Adc_Status_t;

typedef Adc_Status_t ADC_Status_t; /**< Alias para compatibilidad con otros módulos */

/* =========================================================================
 * API - Initialization (FR-1)
 * ========================================================================= */

/**
 * @brief   Initializes the ADC subsystem.
 * @details Enables clocking y configura todos los periféricos ADC a estado
 *          default (FR-1). Llamar una vez antes de cualquier otra función.
 * @return  ADC_OK on success, ADC_ERROR_INVALID otherwise.
 */
Adc_Status_t adc_init(void);

/* =========================================================================
 * API - Enable (FR-2)
 * ========================================================================= */

/**
 * @brief   Habilita la instancia ADC especificada para conversiones.
 * @param   instance  Periférico ADC a habilitar.
 * @return  ADC_OK on success, ADC_ERROR_INVALID for an unknown instance.
 */
Adc_Status_t adc_enableAdc(Adc_Instance_t instance);

/* =========================================================================
 * API - Channel Configuration (FR-3, FR-4)
 * ========================================================================= */
/**
 * @brief   Configura un canal regular ADC (FR-3).
 * @param   instance  Periférico ADC destino.
 * @param   channel   Canal a configurar (0-15).
 * @return  ADC_OK on success, ADC_ERROR_INVALID for out-of-range arguments.
 */
Adc_Status_t adc_setChannel(Adc_Instance_t instance, Adc_Channel_t channel);

/**
 * @brief   Configura un canal inyectado ADC (FR-4).
 * @param   instance  Periférico ADC destino.
 * @param   channel   Canal a configurar (0-15).
 * @param   rank      Rango en la secuencia inyectada (1-4).
 * @return  ADC_OK on success, ADC_ERROR_INVALID for out-of-range arguments.
 */
Adc_Status_t adc_setInjectedChannel(Adc_Instance_t     instance,
                                     Adc_Channel_t      channel,
                                     Adc_InjectedRank_t rank);

/* =========================================================================
 * API - Conversion Control (FR-5, FR-6, FR-7)
 * ========================================================================= */

/**
 * @brief   Inicia una conversión simple en el canal regular configurado (FR-5).
 * @param   instance  Periférico ADC destino.
 * @return  ADC_OK if started, ADC_ERROR_BUSY if already running.
 */
Adc_Status_t adc_startSingleConversion(Adc_Instance_t instance);

/**
 * @brief   Inicia conversiones continuas en el canal regular configurado (FR-6).
 * @details Las conversiones se repiten hasta ser detenidas.
 * @param   instance  Periférico ADC destino.
 * @return  ADC_OK if started, ADC_ERROR_BUSY if already running.
 */
Adc_Status_t adc_startContinuousConversion(Adc_Instance_t instance);

/**
 * @brief   Dispara una conversión en el canal inyectado configurado (FR-7).
 * @param   instance  Periférico ADC destino.
 * @return  ADC_OK if triggered, ADC_ERROR_INVALID for an unknown instance.
 */
Adc_Status_t adc_startInjectedConversion(Adc_Instance_t instance);

/* =========================================================================
 * API - Read Results (FR-8, FR-9)
 * ========================================================================= */

/**
 * @brief   Lee el resultado de la última conversión regular (FR-8).
 * @param   instance  Periférico ADC fuente.
 * @param   data      Puntero donde se almacena el resultado de 12 bits.
 * @return  ADC_OK on success, ADC_ERROR_INVALID for NULL pointer or bad instance.
 */
Adc_Status_t adc_readData(Adc_Instance_t instance, uint16_t *data);

/**
 * @brief   Lee el resultado de la última conversión del canal inyectado (FR-9).
 * @param   instance  Periférico ADC fuente.
 * @param   rank      Rango inyectado cuyo resultado se solicita (1-4).
 * @param   data      Puntero donde se almacena el resultado de 12 bits.
 * @return  ADC_OK on success, ADC_ERROR_INVALID for invalid args or NULL pointer.
 */
Adc_Status_t adc_readInjectedChannelData(Adc_Instance_t     instance,
                                          Adc_InjectedRank_t rank,
                                          uint16_t          *data);

#ifdef __cplusplus
}
#endif

#endif /* ADC_DRIVER_H */
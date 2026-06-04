#include "adc_driver.h"
#include <stddef.h>

/* =========================================================================
 * API - Initialization (FR-1)
 * ========================================================================= */

 
#define RCC_BASE             0x40023800u
#define ADC1_OFFSET       8u
#define RCC_APB2RSTR_OFFSET 0x24u 
#define RCC_APB2ENR_OFFSET  0x44u  
#define RCC_APB2ENRO_OFFSET  (*(volatile uint32_t *)(RCC_BASE + 0x44u))
#define RCC_APB2RSTR_REG    (*(volatile uint32_t *)(RCC_BASE + RCC_APB2RSTR_OFFSET))
#define RCC_APB2ENR_REG     (*(volatile uint32_t *)(RCC_BASE + RCC_APB2ENR_OFFSET))

/////////////////////////////////////
/* ADC CR register bit definitions */
/////////////////////////////////////

#define ADC_CR_OVRIE_BIT        26u  /**< Overrun interrupt enable */
#define ADC_CR_RES_BIT          24u   /**< Resolution configuration bit */
#define ADC_CR_AWDEN_BIT        23u    /**< Data alignment bit */
#define ADC_CR_JAWDEN_BIT       22u    /**< ADC in scan mode for injected channels */
#define ADC_CR_DISCNUM_BIT      13u    /**< Discontinuous mode channel count */
#define ADC_CR_JDISCEN_BIT      12u    /**< Discontinuous mode on injected channels */
#define ADC_CR_DISCEN_BIT       11u    /**< Discontinuous mode on regular channels */
#define ADC_CR_JAUTO_BIT        10u    /**< Automatic injected group conversion */
#define ADC_CR_AWDSGL_BIT       9u     /**< Enable watchdog on single channel in scan mode */
#define ADC_CR_SCAN_BIT         8u     /**< Scan mode enable */
#define ADC_CR_JEOCIE_BIT       7u     /**< Interrupt enable for injected channels end of conversion */
#define ADC_CR_AWDIE_BIT        6u     /**< Analog watchdog interrupt enable */
#define ADC_CR_EOCIE_BIT        5u     /**< Interrupt enable for end of conversion */
#define ADC_CR_AWDCH_BIT        0u     /**< Analog watchdog channel select bits start */


Adc_Status_t adc_init(void){

    /* 1. Encender el clock para poder acceder al periférico */
    RCC_APB2ENR_REG |= (1u << ADC1_OFFSET);

    /* 2. Reset del periférico */
    RCC_APB2RSTR_REG |= (1u << ADC1_OFFSET);
    RCC_APB2RSTR_REG &= ~(1u << ADC1_OFFSET);

    /* Clock permanece encendido para uso posterior */
    return ADC_OK;
}


Adc_Status_t adc_enableAdc(Adc_Instance_t instance){

    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    /* Encender el ADC (ADON = bit 0 de CR2) */
    ADC1->ADC_CR2 |= (1u << 0u);

    /* Asegurar que EXTEN = 00 (trigger por software, bits 29:28) */
    ADC1->ADC_CR2 &= ~(3u << 28u);

    /* Asegurar que EOCS = 0 (EOC al final de secuencia, bit 10) */
    ADC1->ADC_CR2 &= ~(1u << 10u);

    /* Asegurar ALIGN = 0 (alineación a la derecha, bit 11) */
    ADC1->ADC_CR2 &= ~(1u << 11u);

    /* Espera de estabilización */
    volatile uint32_t delay = 1000u;
    while (delay--);

    return ADC_OK;
}

    
Adc_Status_t adc_setChannel(Adc_Instance_t instance, Adc_Channel_t channel){

    /* Rutina de validación de parámetros */

    // Verificación de validez de instancia
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    // Verificación de canal dentro del rango permitido (0-18)
    if (channel > 18u) {
        return ADC_ERROR_INVALID; 
    }

    /* Configuración de la longitud de la secuencia regular (L = 1 conversión) */
    // Limpieza de los bits 23:20 en SQR1 para establecer la secuencia en 0000
    ADC1->ADC_SQR1 &= ~(0xFu << 20u);

    /* Asignación del canal solicitado en la primera posición de la secuencia (SQ1) */
    // Limpieza de los 5 bits inferiores correspondientes a SQ1 en SQR3
    ADC1->ADC_SQR3 &= ~0x1Fu;
    
    // Escritura del número de canal mediante enmascaramiento seguro
    ADC1->ADC_SQR3 |= (channel & 0x1Fu);

    return ADC_OK;

}


Adc_Status_t adc_setInjectedChannel(Adc_Instance_t instance, Adc_Channel_t channel, Adc_InjectedRank_t rank){

    /* Rutina de validación de parámetros */

    // Verificación de validez de instancia
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    // Verificación de canal dentro del rango permitido (0-18)
    if (channel > 18u) {
        return ADC_ERROR_INVALID;
    }

    // Verificación de rango inyectado válido de hardware (1-4)
    if (rank < 1u || rank > 4u) {
        return ADC_ERROR_INVALID;
    }

    /* Configuración del canal inyectado en la secuencia JSQR */
    // Obtención de los bits específicos y cálculo de la máscara según el rango
    uint32_t jsqr_mask = 0x1Fu << ((rank - 1u) * 5u);
    
    // Aislación y limpieza de los bits del rango seleccionado
    ADC1->ADC_JSQR &= ~jsqr_mask;

    // Asignación final del canal aplicando el desplazamiento calculado
    ADC1->ADC_JSQR |= ((channel & 0x1Fu) << ((rank - 1u) * 5u));

    return ADC_OK;
}


Adc_Status_t adc_startSingleConversion(Adc_Instance_t instance){

    /* Rutina de validación de parámetros */

    // Validación de instancia para el STM32F411
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    /* Limpiar EOC antes de iniciar nueva conversión */
    ADC1->ADC_SR &= ~(1u << 1u);
    
    /* Esperar a que la conversión anterior termine */
    uint32_t timeout = 100000;
    while (ADC1->ADC_CR2 & (1u << 30u)) {
        if (--timeout == 0) return ADC_ERROR_TIMEOUT;
    }

    // Iniciar la conversión regular (SWSTART: bit 30 en CR2)
    ADC1->ADC_CR2 |= (1u << 30u); 

    return ADC_OK;
}


Adc_Status_t adc_startContinuousConversion(Adc_Instance_t instance){

    /* Rutina de validación de parámetros */
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    /* Verificación de operación en curso en canales regulares */

    if (ADC1->ADC_CR2 & (1u << 30u)) { 
        return ADC_ERROR_BUSY;
    }

    /* Activación del modo de conversión continua */

    // Configuración del bit CONT (Bit 1) en el registro CR2
    ADC1->ADC_CR2 |= (1u << 1u);

    /* Disparo de la secuencia de conversión continua */
    // Activación del bit SWSTART (Bit 30)
    ADC1->ADC_CR2 |= (1u << 30u);
    return ADC_OK;
}


Adc_Status_t adc_startInjectedConversion(Adc_Instance_t instance){

    /* Rutina de validación de parámetros */
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    /* Verificación de operación en curso en el grupo inyectado */

    // Comprobación del estado del bit JSWSTART (Bit 22) en el registro CR2
    if (ADC1->ADC_CR2 & (1u << 22u)) { 
        return ADC_ERROR_BUSY;
    }

    /* Disparo de la conversión inyectada por software */

    // Activación del bit JSWSTART (Bit 22)
    ADC1->ADC_CR2 |= (1u << 22u);
    return ADC_OK;
}


Adc_Status_t adc_readData(Adc_Instance_t instance, uint16_t *data){

    /* Rutina de validación de parámetros */

    // Verificación de instancia para el STM32F411
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    /* Verificación de finalización de conversión regular */

    // Espera para finalizaci[on de la conversión regular a
    // través de polling.
    uint32_t timeout = 100000;
    while (!(ADC1->ADC_SR & (1u << 1u))) { 
        if (--timeout == 0) {
            return ADC_ERROR_TIMEOUT;
        }
    }
    /* Lectura y aislamiento del resultado de la conversión */

    // Obtención de la media palabra de datos (16 bits) desde el registro DR
    *data = (uint16_t)(ADC1->ADC_DR & 0xFFFFu);
    /* Limpiar el flag EOC manualmente */
    ADC1->ADC_SR &= ~(1u << 1u);


    return ADC_OK;
}


Adc_Status_t adc_readInjectedChannelData(Adc_Instance_t instance, Adc_InjectedRank_t rank, uint16_t *data){
    
    /* Rutina de validación de parámetros */

    // Verificación de instancia para el STM32F411
    if (instance >= ADC_INSTANCE_MAX) {
        return ADC_ERROR_INVALID;
    }

    // Verificación de rango inyectado válido de hardware (1-4)
    if (rank < 1u || rank > 4u) {
        return ADC_ERROR_INVALID;
    }

    uint32_t timeout = 100000;

    // Esperar mientras el bit EOC (bit 1) sea 0, hasta que el timeout llegue a 0
    while (!(ADC1->ADC_SR & (1u << 2u))) {
        if (--timeout == 0) {
            return ADC_ERROR_TIMEOUT;
        }
    }

    /* Selección y lectura del registro de datos inyectado objetivo (JDRx) */
    switch(rank) {
        case 1: *data = (uint16_t)(ADC1->ADC_JDR1 & 0xFFFFu); break;
        case 2: *data = (uint16_t)(ADC1->ADC_JDR2 & 0xFFFFu); break;
        case 3: *data = (uint16_t)(ADC1->ADC_JDR3 & 0xFFFFu); break;
        case 4: *data = (uint16_t)(ADC1->ADC_JDR4 & 0xFFFFu); break;
        default: return ADC_ERROR_INVALID;
    }
    
    return ADC_OK;
}
#include <stdint.h>
#include <stddef.h>
#include "GPIO_stm32.h"


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_init(void){

    /* No se realiza operación particular al inicializarse por defecto
       todos los registro GPIOx en apagado. S*/
    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_initPort(gpio_port_t port){

    /* Verificación de la validez del puerto */
    if( port >= GPIO_PORT_COUNT || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    }
     
    /* Activación del clock dedicado del puerto 
       en el registro RCC                        */
    RCC_AHB1ENR |= (1 << port);

    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_setPinMode(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    } else

    // Verificación de modo válido
    if( mode >= GPIO_TOTAL_MODES ){
        return GPIO_ERR_INVALID_MODE;
    }

    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }

    /* Asignación de modo de operación del pin */

    // Se limpian los bits del pin objetivo (00)
    gpio_port->MODER &= ~(3U << (2U*pin));  

    // Se escribe el modo de operación deseado 
    gpio_port->MODER |= (mode << (2U*pin));

    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_setPin(gpio_port_t port, gpio_pin_t pin){

   /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    } 


    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }


    /* Verificación de operación correcta del pin */

    // Obtención de los bits específicos de la operación del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin*2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Verificación del modo OUTPUT
    if (pin_mode != GPIO_MODE_OUTPUT){
        return GPIO_ERR_INVALID_MODE;
    }

    /* Escritura de estado HIGH de salida */
    gpio_port->BSRR = (1U << pin);

    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_clearPin(gpio_port_t port, gpio_pin_t pin){

      /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U ){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    } 


    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }


    /* Verificación de operación correcta del pin */

    // Obtención de los bits específicos de la operación del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin*2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Verificación del modo OUTPUT
    if (pin_mode != GPIO_MODE_OUTPUT){
        return GPIO_ERR_INVALID_MODE;
    }


    /* Escritura de estado LOW de salida */
    gpio_port->BSRR = (1U << (16U + pin));

    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_togglePin(gpio_port_t port, gpio_pin_t pin){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    } 


    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }

   /* Verificación de operación correcta del pin */

    // Obtención de los bits específicos de la operación del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin*2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Verificación del modo OUTPUT
    if (pin_mode != GPIO_MODE_OUTPUT){
        return GPIO_ERR_INVALID_MODE;
    }


    /* Operación de inversión del pin especificado */
    gpio_port->ODR ^= (1U << pin);

    return GPIO_OK;

}


/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_readPin(gpio_port_t port, gpio_pin_t pin, uint8_t *state){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    }  else

    // Verificación de puntero no nulo
    if( state == NULL){
        return GPIO_ERR_NULL_POINTER;
    }

   /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */

    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }

   /* Verificación de operación correcta del pin */

    // Obtención de los bits específicos de la operación del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin*2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Verificación del modo INPUT
    if (pin_mode != GPIO_MODE_INPUT){
        return GPIO_ERR_INVALID_MODE;
    }

    /* Operación de lectura del bit */

    // Obtención del estado actual del registro de datos INPUT
    uint32_t input = ((gpio_port ->IDR)>>pin);

    // Aislación del bit específico del pin
    input &= 1U;

    // Escritura del estado en la dirección del registro destino
    (*state) = (uint8_t)input;

    return GPIO_OK;
}

/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_setAlternateFunction(gpio_port_t port, gpio_pin_t pin, gpio_altfunction_t af_val){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT  || port==5U || port == 6U){
        return GPIO_ERR_INVALID_PORT;
    } else 

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    } else

    // Verificación de función alterna válida
    if( af_val >= GPIO_AF_COUNT ){
        return GPIO_ERR_INVALID_MODE;
    }

   /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR>>port);
    
    // Aislación del bit
    gpio_check &= 1U; 

    // Verificación de estado válido
    if( gpio_check != 1U){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; 
                          break;
        case GPIO_PORT_B: gpio_port = GPIOB; 
                          break;
        case GPIO_PORT_C: gpio_port = GPIOC; 
                          break;
        case GPIO_PORT_D: gpio_port = GPIOD; 
                          break;
        case GPIO_PORT_E: gpio_port = GPIOE; 
                          break;
        case GPIO_PORT_H: gpio_port = GPIOH; 
                          break;
        default: return GPIO_ERR_INVALID_PORT;
    }

   /* Verificación de operación correcta del pin */

    // Obtención de los bits específicos de la operación del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin*2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Verificación del modo ALT_FUNC
    if (pin_mode != GPIO_MODE_ALT_FUNC){
        return GPIO_ERR_INVALID_MODE;
    }

   /* Asignación de función alterna al pin especificado */

   // Selección del registro AFR[0] o AFR[1] dependiendo del número del pin
   uint8_t afr_index = (pin < 8U) ? 0U : 1U;

   // El desplazamiento es (pin modulo 8) multiplicado por 4 bits por pin
    uint8_t bit_shift = (pin % 8U) * 4U;

    // Limpiar los 4 bits del pin (Máscara 0xF = 1111)
    gpio_port->AFR[afr_index] &= ~(0xFU << bit_shift);

    // Escribir el valor de la función alternativa
    gpio_port->AFR[afr_index] |= ((uint32_t)af_val << bit_shift);

    return GPIO_OK;
}

/// @brief 
/// @param  
/// @return //
gpio_status_t gpio_setOpenDrain(gpio_port_t port, gpio_pin_t pin){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT || port == 5U || port == 6U ){
        return GPIO_ERR_INVALID_PORT;
    } else

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    }

    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR >> port);

    // Aislación del bit
    gpio_check &= 1U;

    // Verificación de estado válido
    if( gpio_check != 1U ){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; break;
        case GPIO_PORT_B: gpio_port = GPIOB; break;
        case GPIO_PORT_C: gpio_port = GPIOC; break;
        case GPIO_PORT_D: gpio_port = GPIOD; break;
        case GPIO_PORT_E: gpio_port = GPIOE; break;
        case GPIO_PORT_H: gpio_port = GPIOH; break;
        default: return GPIO_ERR_INVALID_PORT;
    }

    /* Verificación de modo compatible (OUTPUT o ALT_FUNC) */

    // Obtención de los bits de modo del pin objetivo
    uint32_t pin_mode = (gpio_port->MODER >> (pin * 2U));

    // Aislación de los dos bits de operación
    pin_mode &= 3U;

    // Solo OUTPUT (01) y ALT_FUNC (10) son compatibles con open-drain
    if( pin_mode != GPIO_MODE_OUTPUT && pin_mode != GPIO_MODE_ALT_FUNC ){
        return GPIO_ERR_INVALID_MODE;
    }

    /* Configuración de open-drain en el registro OTYPER */

    // Un 1 en el bit del pin selecciona open-drain
    gpio_port->OTYPER |= (1U << pin);

    return GPIO_OK;

}

/// @brief
/// @param
/// @return //
gpio_status_t gpio_setPullUp(gpio_port_t port, gpio_pin_t pin){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT || port == 5U || port == 6U ){
        return GPIO_ERR_INVALID_PORT;
    } else

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    }

    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR >> port);

    // Aislación del bit
    gpio_check &= 1U;

    // Verificación de estado válido
    if( gpio_check != 1U ){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; break;
        case GPIO_PORT_B: gpio_port = GPIOB; break;
        case GPIO_PORT_C: gpio_port = GPIOC; break;
        case GPIO_PORT_D: gpio_port = GPIOD; break;
        case GPIO_PORT_E: gpio_port = GPIOE; break;
        case GPIO_PORT_H: gpio_port = GPIOH; break;
        default: return GPIO_ERR_INVALID_PORT;
    }

    /* Configuración de pull-up en el registro PUPDR */

    // Se limpian los 2 bits del pin (00 = sin resistencia)
    gpio_port->PUPDR &= ~(3U << (pin * 2U));

    // Se escribe 01 = pull-up habilitado (RM0383 §8.4.4)
    gpio_port->PUPDR |=  (1U << (pin * 2U));

    return GPIO_OK;

}

/// @brief
/// @param
/// @return //
gpio_status_t gpio_setPullDown(gpio_port_t port, gpio_pin_t pin){

    /* Rutina de validación de parámetros */

    // Verificación de puerto válido
    if( port >= GPIO_PORT_COUNT || port == 5U || port == 6U ){
        return GPIO_ERR_INVALID_PORT;
    } else

    // Verificación de pin válido
    if( pin >= GPIO_PIN_COUNT ){
        return GPIO_ERR_INVALID_PIN;
    }

    /* Verificación de puerto activado */

    // Obtención del bit específico del puerto
    uint32_t gpio_check = (RCC_AHB1ENR >> port);

    // Aislación del bit
    gpio_check &= 1U;

    // Verificación de estado válido
    if( gpio_check != 1U ){
        return GPIO_ERR_NOT_INITIALISED;
    }

    /* Obtención del puerto deseado */
    GPIO_RegDef_t *gpio_port;

    switch (port){
        case GPIO_PORT_A: gpio_port = GPIOA; break;
        case GPIO_PORT_B: gpio_port = GPIOB; break;
        case GPIO_PORT_C: gpio_port = GPIOC; break;
        case GPIO_PORT_D: gpio_port = GPIOD; break;
        case GPIO_PORT_E: gpio_port = GPIOE; break;
        case GPIO_PORT_H: gpio_port = GPIOH; break;
        default: return GPIO_ERR_INVALID_PORT;
    }

    /* Configuración de pull-down en el registro PUPDR */

    // Se limpian los 2 bits del pin (00 = sin resistencia)
    gpio_port->PUPDR &= ~(3U << (pin * 2U));

    // Se escribe 10 = pull-down habilitado (RM0383 §8.4.4)
    gpio_port->PUPDR |=  (2U << (pin * 2U));

    return GPIO_OK;

}
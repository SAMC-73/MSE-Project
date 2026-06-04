/*

#include <stdint.h>

#define RCC_BASE (0x40023800UL)
#define RCC_AHB1ENR_OFFSET (RCC_BASE + 0x30UL)
#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_AHB1ENR_OFFSET))

#define GPIOA_BASE (0x40020000UL)
#define GPIOA_MODER_OFFSET (GPIOA_BASE + 0x00UL)
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_MODER_OFFSET))



#define GPIOA_ODR_OFFSET (GPIOA_BASE + 0x14UL)
#define GPIOA_ODR (*(volatile unsigned int *)(GPIOA_ODR_OFFSET))

#define GPIOA_EN (0)
#define CLK_EN (1U)


#define __I 		volatile const
#define __O 		volatile
#define __IO		volatile

 gpio_init(void);

 gpio_initPort(uint8_t port);

 gpio_setPinMode()

*/

/**
 * @file    gpio_driver.h
 * @brief   GPIO Driver - Software Requirements Specification v1.0
 * @author  Leonardo Ivan García Espinosa
 *
 * @details Provides a portable, register-level GPIO abstraction for the
 *          STM32F411RE microcontroller (RM0383). All public functions return
 *          a gpio_status_t code so that callers can perform error handling
 *          (NFR-2). Pin state is returned through an output pointer in
 *          gpio_readPin() to keep the return-type convention uniform.
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>
#include <stddef.h>

/* =========================================================================
 * Compiler / access-qualifier macros
 * ========================================================================= */

#define __I     volatile const   /**< Read-only  */
#define __O     volatile         /**< Write-only */
#define __IO    volatile         /**< Read/Write */

/* =========================================================================
 * RCC – Reset and Clock Control (RM0383 §6.3)
 * ========================================================================= */
#ifndef RCC_BASE
#define RCC_BASE        0x40023800UL
#endif
#define RCC_AHB1ENR     (*((__IO uint32_t *)(RCC_BASE + 0x30UL)))

/** Bit positions in RCC_AHB1ENR for each GPIO port clock enable */
#define RCC_AHB1ENR_GPIOAEN_POS  (0U)
#define RCC_AHB1ENR_GPIOBEN_POS  (1U)
#define RCC_AHB1ENR_GPIOCEN_POS  (2U)
#define RCC_AHB1ENR_GPIODEN_POS  (3U)
#define RCC_AHB1ENR_GPIOEEN_POS  (4U)
#define RCC_AHB1ENR_GPIOHEN_POS  (7U)

/* =========================================================================
 * GPIO register map (RM0383 §8.4)
 * ========================================================================= */

/**
 * @brief Memory-mapped register layout for one GPIO port.
 *        Cast a base address to (GPIO_RegDef_t *) to access its registers.
 */
typedef struct
{
    __IO uint32_t MODER;    /**< 0x00 – Mode register              */
    __IO uint32_t OTYPER;   /**< 0x04 – Output type register       */
    __IO uint32_t OSPEEDR;  /**< 0x08 – Output speed register      */
    __IO uint32_t PUPDR;    /**< 0x0C – Pull-up/pull-down register */
    __I  uint32_t IDR;      /**< 0x10 – Input data register        */
    __IO uint32_t ODR;      /**< 0x14 – Output data register       */
    __O  uint32_t BSRR;     /**< 0x18 – Bit set/reset register     */
    __IO uint32_t LCKR;     /**< 0x1C – Configuration lock register*/
    __IO uint32_t AFR[2];   /**< 0x20 – Alternate function regs    */
} GPIO_RegDef_t;

/** Convenience pointers – dereference to access registers directly */
#define GPIOA   ((GPIO_RegDef_t *) 0x40020000UL)
#define GPIOB   ((GPIO_RegDef_t *) 0x40020400UL)
#define GPIOC   ((GPIO_RegDef_t *) 0x40020800UL)
#define GPIOD   ((GPIO_RegDef_t *) 0x40020C00UL)
#define GPIOE   ((GPIO_RegDef_t *) 0x40021000UL)
#define GPIOH   ((GPIO_RegDef_t *) 0x40021C00UL)

/* =========================================================================
 * Enumerations
 * ========================================================================= */

/**
 * @brief Port identifiers.
 *        Passed to functions that need to resolve a base address or a
 *        clock-enable bit.
 */
typedef enum
{
    GPIO_PORT_A = 0U,
    GPIO_PORT_B = 1U,
    GPIO_PORT_C = 2U,
    GPIO_PORT_D = 3U,
    GPIO_PORT_E = 4U,
    GPIO_PORT_H = 7U,
    GPIO_PORT_COUNT    /**< Sentinel – do not use as a port value */
} gpio_port_t;

/**
 * @brief Pin identifiers (0–15 map to the 16 lines of each port).
 */
typedef enum
{
    GPIO_PIN_0  = 0U,
    GPIO_PIN_1  = 1U,
    GPIO_PIN_2  = 2U,
    GPIO_PIN_3  = 3U,
    GPIO_PIN_4  = 4U,
    GPIO_PIN_5  = 5U,
    GPIO_PIN_6  = 6U,
    GPIO_PIN_7  = 7U,
    GPIO_PIN_8  = 8U,
    GPIO_PIN_9  = 9U,
    GPIO_PIN_10 = 10U,
    GPIO_PIN_11 = 11U,
    GPIO_PIN_12 = 12U,
    GPIO_PIN_13 = 13U,
    GPIO_PIN_14 = 14U,
    GPIO_PIN_15 = 15U,
    GPIO_PIN_COUNT   /**< Sentinel – do not use as a pin value */
} gpio_pin_t;

/**
 * @brief Pin mode values written to MODER (2 bits per pin).
 *        (RM0383 §8.4.1)
 */
typedef enum
{
    GPIO_MODE_INPUT     = 0x0U,  /**< Digital input          */
    GPIO_MODE_OUTPUT    = 0x1U,  /**< General-purpose output */
    GPIO_MODE_ALT_FUNC  = 0x2U,  /**< Alternate function     */
    GPIO_MODE_ANALOG    = 0x3U,   /**< Analog                 */
    GPIO_TOTAL_MODES    = 0x4U   /**< Sentinel – do not use as a mode value */
} gpio_mode_t;


/**
 * @brief Selector of Alternate Functions (AF0 to AF15)
 */
typedef enum {
    GPIO_AF0 = 0U,
    GPIO_AF1 = 1U,
    GPIO_AF2 = 2U,
    GPIO_AF3 = 3U,
    GPIO_AF4  = 4U,
    GPIO_AF5 = 5U,
    GPIO_AF6 = 6U,
    GPIO_AF7 = 7U,
    GPIO_AF8 = 8U,
    GPIO_AF9 = 9U,
    GPIO_AF10 = 10U,
    GPIO_AF11 = 11U,
    GPIO_AF12 = 12U,
    GPIO_AF13 = 13U,
    GPIO_AF14 = 14U,
    GPIO_AF15 = 15U,
    GPIO_AF_COUNT = 16U
} gpio_altfunction_t;

/**
 * @brief Return codes for all driver functions (NFR-2).
 */
typedef enum
{
    GPIO_OK             = 0U,   /**< Operation succeeded            */
    GPIO_ERR_INVALID_PORT = 1U, /**< Port argument out of range     */
    GPIO_ERR_INVALID_PIN = 2U,  /**< Pin argument out of range      */
    GPIO_ERR_INVALID_MODE = 3U, /**< Mode argument unrecognised     */
    GPIO_ERR_NULL_POINTER = 4U, /**< A required pointer was NULL    */
    GPIO_ERR_NOT_INITIALISED = 5U /**< Driver used before gpio_init() */
} gpio_status_t;

typedef gpio_status_t GPIO_Status_t; /**< Alias for compatibility with other modules */

/* =========================================================================
 * Function definitions
 * ========================================================================= */

/**
 * @brief   Initialises the GPIO subsystem (FR-1).
 * @details Configures all GPIO ports to their default (reset) state.
 *          Must be called once before any other gpio_* function.
 * @return  GPIO_OK on success, or a gpio_status_t error code.
 */
gpio_status_t gpio_init(void);

/**
 * @brief   Initialises a single GPIO port (FR-2).
 * @details Enables the peripheral clock for @p port via RCC_AHB1ENR.
 * @param   port  Target port (gpio_port_t).
 * @return  GPIO_OK on success, GPIO_ERR_INVALID_PORT if @p port is out
 *          of range.
 */
gpio_status_t gpio_initPort(gpio_port_t port);

/**
 * @brief   Configures the mode of a pin (FR-3).
 * @details Writes the appropriate 2-bit field in the port MODER register.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @param   mode  Desired pin mode (gpio_mode_t).
 * @return  GPIO_OK on success, or GPIO_ERR_INVALID_PORT /
 *          GPIO_ERR_INVALID_PIN / GPIO_ERR_INVALID_MODE on bad arguments.
 */
gpio_status_t gpio_setPinMode(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode);

/**
 * @brief   Drives a pin to logic HIGH (FR-4).
 * @details Uses the BSRR set bits so the operation is atomic.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, or GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_setPin(gpio_port_t port, gpio_pin_t pin);

/**
 * @brief   Drives a pin to logic LOW (FR-5).
 * @details Uses the BSRR reset bits so the operation is atomic.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, or GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_clearPin(gpio_port_t port, gpio_pin_t pin);

/**
 * @brief   Inverts the current output state of a pin (FR-6).
 * @details Reads the ODR, XORs the target bit, and writes back.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, or GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_togglePin(gpio_port_t port, gpio_pin_t pin);

/**
 * @brief   Reads the digital state of a pin (FR-7).
 * @details Samples the IDR register for input pins; callers should
 *          configure the pin as input first with gpio_setPinMode().
 * @param   port   Target port (gpio_port_t).
 * @param   pin    Target pin  (gpio_pin_t).
 * @param   state  Output parameter: written with 0 (LOW) or 1 (HIGH).
 *                 Must not be NULL.
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, GPIO_ERR_INVALID_PIN, or
 *          GPIO_ERR_NULL_POINTER if @p state is NULL.
 */
gpio_status_t gpio_readPin(gpio_port_t port, gpio_pin_t pin, uint8_t *state);

/**
 * @brief   Configures the alternate fuction of a pin
 * @details Modifies the AFRL or register AFRH depending of the pin
 * @param   port   Target port (gpio_port_t).
 * @param   pin    Target pin  (gpio_pin_t).
 * @param   af_val Alternate Function's value (0 to 15).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_setAlternateFunction(gpio_port_t port, gpio_pin_t pin, gpio_altfunction_t af_val);

/**
 * @brief   Sets the output type of a pin to open-drain.
 * @details Writes a 1 to the pin bit in the port OTYPER register.
 *          The pin must be configured as OUTPUT or ALT_FUNC beforehand
 *          with gpio_setPinMode().
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, GPIO_ERR_INVALID_PIN, or
 *          GPIO_ERR_INVALID_MODE if the pin is not in OUTPUT / ALT_FUNC mode.
 */
gpio_status_t gpio_setOpenDrain(gpio_port_t port, gpio_pin_t pin);

/**
 * @brief   Enables the internal pull-up resistor of a pin.
 * @details Writes 01 to the two-bit field of @p pin in the port PUPDR register.
 *          Compatible with INPUT, OUTPUT, and ALT_FUNC pin modes.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, or GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_setPullUp(gpio_port_t port, gpio_pin_t pin);

/**
 * @brief   Enables the internal pull-down resistor of a pin.
 * @details Writes 10 to the two-bit field of @p pin in the port PUPDR register.
 *          Compatible with INPUT, OUTPUT, and ALT_FUNC pin modes.
 * @param   port  Target port (gpio_port_t).
 * @param   pin   Target pin  (gpio_pin_t).
 * @return  GPIO_OK, GPIO_ERR_INVALID_PORT, or GPIO_ERR_INVALID_PIN.
 */
gpio_status_t gpio_setPullDown(gpio_port_t port, gpio_pin_t pin);

#endif /* GPIO_DRIVER_H */
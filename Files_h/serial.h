/**
 * @file    serial.h
 * @brief   Serial Module – High-level API for terminal communication v1.0
 * @author  Adrián Rafael Amaro Avala
 *          Leonardo Ivan García Espinosa 
 *          Mario Morales Vega
 * 
 * @details Provides a high-level serial interface for the STM32F411RE,
 *          built on top of the UART driver and the Utils module.
 *          Exposes two functions: serial_init to configure the communication
 *          channel, and serial_printf to send formatted strings to the
 *          terminal (Task 2, Lab Assignment 04).
 *
 *          Dependency stack:
 *            serial_printf
 *              ├── utils_snprintf   (string formatting – Utils module)
 *              └── uart_write       (byte transmission – UART driver)
 *            serial_init
 *              └── uart_init        (peripheral setup  – UART driver)
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdarg.h>      /* va_list – required for serial_printf */
#include "uart_driver.h"

/* =========================================================================
 * Internal buffer size
 * =========================================================================
 * Maximum number of characters that serial_printf can format in one call,
 * including the null terminator. Adjust if longer messages are needed.
 * ========================================================================= */

#define SERIAL_TX_BUFFER_SIZE   (256U)

/* =========================================================================
 * Function declarations
 * ========================================================================= */

/**
 * @brief   Initialises the Serial module (Task 2).
 *
 * @details Calls uart_init() internally to enable the USART2 peripheral,
 *          configure the baud rate, and activate the transmitter.
 *          Must be called once in main before any serial_printf call.
 *
 *          The GPIO pins (PA2 – TX) must be configured for Alternate
 *          Function 7 (AF7) via gpio_driver before calling this function.
 *
 * @return  void
 *
 * @note    Example usage:
 * @code
 *          gpio_initPort(GPIO_PORT_A);
 *          gpio_setPinMode(GPIO_PORT_A, GPIO_PIN_2, GPIO_MODE_ALT_FUNC);
 *          gpio_setAlternateFunction(GPIO_PORT_A, GPIO_PIN_2, GPIO_AF7);
 *
 *          serial_init();
 * @endcode
 */
void serial_init(void);

/**
 * @brief   Sends a formatted string over USART2 (Task 2, Task 3).
 *
 * @details Internally uses utils_snprintf to format the message into a
 *          local buffer, then iterates over each character calling
 *          uart_write to transmit it byte by byte.
 *
 * @param   fmt   Format string, following printf-style specifiers
 *                (%d, %s, %c, etc.) as supported by the Utils module.
 * @param   ...   Variable arguments matching the format specifiers.
 * @return  void
 *
 * @note    Output is truncated to SERIAL_TX_BUFFER_SIZE - 1 characters.
 *
 * @note    Example usage (Task 3):
 * @code
 *          uint32_t adc_value  = 2048;
 *          uint32_t voltage_mv = 1650;
 *
 *          serial_printf("ADC Value: %d | Voltage: %d mV\n",
 *                         adc_value, voltage_mv);
 * @endcode
 */
void serial_printf(const char *fmt, ...);

#endif /* SERIAL_H */

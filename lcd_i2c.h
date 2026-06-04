/**
 * @file    lcd_i2c.h
 * @brief   driver para lcd hd44780 20x4 via expansor pcf8574 (i2c)
 *
 * el pcf8574 mapea sus 8 pines de i/o al bus de 4 bits del hd44780:
 *
 *   pcf8574 bit   lcd pin   funcion
 *   -----------   -------   -------
 *       P0          RS      register select  (0 = comando, 1 = dato)
 *       P1          RW      read/write       (siempre 0 = escritura)
 *       P2          EN      enable pulse
 *       P3          BL      backlight (activo en alto)
 *       P4          D4      dato bit 4
 *       P5          D5      dato bit 5
 *       P6          D6      dato bit 6
 *       P7          D7      dato bit 7
 *
 * dependencias: i2c_driver.h
 *
 * @note  la direccion i2c del pcf8574 depende de los pines A0-A2:
 *        - pcf8574  (texas/nxp generico): 0x20 - 0x27
 *        - pcf8574a (variante):           0x38 - 0x3F
 *        el valor mas comun en modulos lcd de arduino es 0x27.
 */

#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "i2c_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * configuracion del modulo — ajusta segun tu hardware
 * ============================================================ */

/** direccion i2c del pcf8574 (sin desplazar, 7 bits) */
#define LCD_PCF8574_ADDR    0x27U

/** numero de columnas del lcd */
#define LCD_COLS            20U

/** numero de filas del lcd */
#define LCD_ROWS            4U

/* ============================================================
 * mascaras de bits del pcf8574
 * ============================================================ */
#define LCD_BIT_RS          (1U << 0)   /**< register select */
#define LCD_BIT_RW          (1U << 1)   /**< read/write (siempre 0) */
#define LCD_BIT_EN          (1U << 2)   /**< enable */
#define LCD_BIT_BL          (1U << 3)   /**< backlight */

/* ============================================================
 * comandos hd44780
 * ============================================================ */
#define LCD_CMD_CLEAR           0x01U   /**< borrar pantalla */
#define LCD_CMD_HOME            0x02U   /**< cursor al inicio */
#define LCD_CMD_ENTRY_MODE      0x06U   /**< incremento automatico, sin shift */
#define LCD_CMD_DISPLAY_ON      0x0CU   /**< display on, cursor off */
#define LCD_CMD_DISPLAY_OFF     0x08U   /**< display off */
#define LCD_CMD_CURSOR_ON       0x0EU   /**< display on, cursor on */
#define LCD_CMD_BLINK_ON        0x0FU   /**< display on, cursor on, blink on */
#define LCD_CMD_4BIT_2LINE      0x28U   /**< modo 4 bits, 2 lineas, 5x8 */
#define LCD_CMD_SET_DDRAM       0x80U   /**< direccion base ddram */

/* direcciones ddram por fila (20x4) */
#define LCD_ROW0_ADDR           0x00U
#define LCD_ROW1_ADDR           0x40U
#define LCD_ROW2_ADDR           0x14U
#define LCD_ROW3_ADDR           0x54U

/* ============================================================
 * codigos de retorno
 * ============================================================ */
typedef enum {
    LCD_OK              =  0,
    LCD_ERR_INVALID_ARG = -1,
    LCD_ERR_I2C         = -2,   /**< error del bus i2c subyacente */
} lcd_status_t;

/* ============================================================
 * estructura de handle del lcd
 * ============================================================ */
typedef struct {
    I2C_RegDef_t *i2c;         /**< instancia i2c inicializada */
    uint8_t       addr;         /**< direccion pcf8574 (7 bits) */
    uint8_t       backlight;    /**< lcd_bit_bl si encendido, 0 si apagado */
} lcd_handle_t;

/* ============================================================
 * api publica
 * ============================================================ */

/**
 * @brief  inicializa el lcd en modo 4 bits.
 *
 * realiza la secuencia de inicializacion obligatoria del hd44780:
 * tres escrituras a 0x03 en modo 8 bits y luego la transicion a 4 bits.
 * despues configura funcion, display y modo de entrada.
 *
 * @param[out] lcd   handle a rellenar.  no debe ser null.
 * @param[in]  i2c   periferico i2c ya inicializado con i2c_init().
 * @param[in]  addr  direccion del pcf8574 (ej. lcd_pcf8574_addr).
 *
 * @retval lcd_ok              inicializacion correcta.
 * @retval lcd_err_invalid_arg lcd o i2c es null.
 * @retval lcd_err_i2c         fallo la comunicacion i2c.
 */
lcd_status_t lcd_init(lcd_handle_t *lcd, I2C_RegDef_t *i2c, uint8_t addr);

/**
 * @brief  borra la pantalla y coloca el cursor en (0,0).
 */
lcd_status_t lcd_clear(lcd_handle_t *lcd);

/**
 * @brief  mueve el cursor a la posicion indicada.
 *
 * @param[in] col  columna (0 - lcd_cols-1).
 * @param[in] row  fila    (0 - lcd_rows-1).
 */
lcd_status_t lcd_set_cursor(lcd_handle_t *lcd, uint8_t col, uint8_t row);

/**
 * @brief  escribe un caracter en la posicion actual del cursor.
 */
lcd_status_t lcd_put_char(lcd_handle_t *lcd, char c);

/**
 * @brief  escribe una cadena en la posicion actual del cursor.
 *
 * @param[in] str  cadena terminada en '\0'.
 */
lcd_status_t lcd_print(lcd_handle_t *lcd, const char *str);

/**
 * @brief  enciende o apaga la retroiluminacion.
 *
 * @param[in] on  1 = encender, 0 = apagar.
 */
lcd_status_t lcd_backlight(lcd_handle_t *lcd, uint8_t on);

#ifdef __cplusplus
}
#endif

#endif /* LCD_I2C_H */

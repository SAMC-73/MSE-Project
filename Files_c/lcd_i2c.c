/**
 * @file    lcd_i2c.c
 * @brief   implementacion del driver lcd hd44780 20x4 via pcf8574 (i2c)
 *
 * protocolo de escritura al hd44780 en modo 4 bits:
 *   1. colocar los 4 bits altos del byte en p4-p7 del pcf8574.
 *   2. pulsar en (p2) alto -> bajo para que el hd44780 lea el nibble.
 *   3. repetir para los 4 bits bajos.
 *
 * cada escritura i2c envia un byte al pcf8574 con el estado
 * completo de sus 8 pines (datos + rs + rw + en + bl).
 */

#include "lcd_i2c.h"
#include <stddef.h>

/* ============================================================
 * retardo de software (bloqueante)
 * ajusta el multiplicador segun la frecuencia real de tu mcu.
 * a 16 mhz un ciclo dura ~62 ns; 1000 iteraciones aprox 62 us.
 * ============================================================ */
static void delay_us(uint32_t us)
{
    /* calibrado para stm32f4 a 16 mhz (hsi sin pll).
     * si usas pll a 84/168 mhz, multiplica el factor. */
    volatile uint32_t count = us * 16U;
    while (count--) {
        __asm__ volatile ("nop");
    }
}

/* ============================================================
 * funcion interna: enviar un byte al pcf8574
 * ============================================================ */
static lcd_status_t pcf8574_write(lcd_handle_t *lcd, uint8_t data)
{
    i2c_status_t st = i2c_writeDevice(lcd->i2c, lcd->addr, &data, 1U);
    return (st == I2C_OK) ? LCD_OK : LCD_ERR_I2C;
}

/* ============================================================
 * funcion interna: pulsar en para que el lcd lea el nibble
 * ============================================================ */
static lcd_status_t lcd_pulse_enable(lcd_handle_t *lcd, uint8_t data)
{
    lcd_status_t st;

    /* en = 1 */
    st = pcf8574_write(lcd, data | LCD_BIT_EN);
    if (st != LCD_OK) return st;
    delay_us(1U);          /* t_eh >= 450 ns */

    /* en = 0 — el hd44780 lee en el flanco de bajada */
    st = pcf8574_write(lcd, data & ~LCD_BIT_EN);
    if (st != LCD_OK) return st;
    delay_us(50U);         /* tiempo de ejecucion del comando */

    return LCD_OK;
}

/* ============================================================
 * funcion interna: enviar un nibble (4 bits) al hd44780
 *   rs = 0 -> comando, rs = lcd_bit_rs -> dato
 * ============================================================ */
static lcd_status_t lcd_send_nibble(lcd_handle_t *lcd, uint8_t nibble, uint8_t rs)
{
    /* los 4 bits de datos van en p4-p7 */
    uint8_t data = (nibble << 4) | rs | lcd->backlight;
    return lcd_pulse_enable(lcd, data);
}

/* ============================================================
 * funcion interna: enviar un byte completo en dos nibbles
 * ============================================================ */
static lcd_status_t lcd_send_byte(lcd_handle_t *lcd, uint8_t byte, uint8_t rs)
{
    lcd_status_t st;

    st = lcd_send_nibble(lcd, (byte >> 4) & 0x0FU, rs);  /* nibble alto */
    if (st != LCD_OK) return st;

    st = lcd_send_nibble(lcd, byte & 0x0FU, rs);          /* nibble bajo */
    return st;
}

/* ============================================================
 * funcion interna: enviar un comando al hd44780
 * ============================================================ */
static lcd_status_t lcd_send_cmd(lcd_handle_t *lcd, uint8_t cmd)
{
    return lcd_send_byte(lcd, cmd, 0U);  /* rs = 0 */
}

/* ============================================================
 * funcion interna: enviar un dato (caracter) al hd44780
 * ============================================================ */
static lcd_status_t lcd_send_data(lcd_handle_t *lcd, uint8_t data)
{
    return lcd_send_byte(lcd, data, LCD_BIT_RS);  /* rs = 1 */
}

/* ============================================================
 * lcd_init — secuencia de inicializacion hd44780
 *
 * segun la hoja de datos, para arrancar en modo 4 bits desde
 * un estado indeterminado hay que enviar tres veces 0x03 en
 * modo 8 bits (nibble alto = 0x3) y luego el comando 0x02
 * para cambiar a modo 4 bits.
 * ============================================================ */
lcd_status_t lcd_init(lcd_handle_t *lcd, I2C_RegDef_t *i2c, uint8_t addr)
{
    if (lcd == NULL || i2c == NULL) {
        return LCD_ERR_INVALID_ARG;
    }

    lcd->i2c       = i2c;
    lcd->addr      = addr;
    lcd->backlight = LCD_BIT_BL;   /* retroiluminacion encendida por defecto */

    /* espera inicial: el hd44780 necesita >40 ms tras vcc >= 2.7 v */
    delay_us(50000U);

    /* --- secuencia de inicializacion en modo 8 bits (x3) --- */
    /* primera escritura: nibble 0x3 */
    lcd_send_nibble(lcd, 0x03U, 0U);
    delay_us(4500U);    /* >= 4.1 ms */

    /* segunda escritura */
    lcd_send_nibble(lcd, 0x03U, 0U);
    delay_us(4500U);

    /* tercera escritura */
    lcd_send_nibble(lcd, 0x03U, 0U);
    delay_us(150U);

    /* --- cambio a modo 4 bits --- */
    lcd_send_nibble(lcd, 0x02U, 0U);
    delay_us(150U);

    /* --- configuracion en modo 4 bits --- */
    lcd_send_cmd(lcd, LCD_CMD_4BIT_2LINE);   /* 4 bits, 2 lineas logicas, 5x8 */
    delay_us(50U);

    lcd_send_cmd(lcd, LCD_CMD_DISPLAY_ON);   /* display on, cursor off */
    delay_us(50U);

    lcd_send_cmd(lcd, LCD_CMD_CLEAR);        /* borrar pantalla */
    delay_us(2000U);   /* clear requiere >= 1.52 ms */

    lcd_send_cmd(lcd, LCD_CMD_ENTRY_MODE);   /* incremento automatico */
    delay_us(50U);

    return LCD_OK;
}

/* ============================================================
 * lcd_clear
 * ============================================================ */
lcd_status_t lcd_clear(lcd_handle_t *lcd)
{
    if (lcd == NULL) return LCD_ERR_INVALID_ARG;

    lcd_status_t st = lcd_send_cmd(lcd, LCD_CMD_CLEAR);
    delay_us(2000U);
    return st;
}

/* ============================================================
 * lcd_set_cursor
 * ============================================================ */
lcd_status_t lcd_set_cursor(lcd_handle_t *lcd, uint8_t col, uint8_t row)
{
    if (lcd == NULL)           return LCD_ERR_INVALID_ARG;
    if (col >= LCD_COLS)       return LCD_ERR_INVALID_ARG;
    if (row >= LCD_ROWS)       return LCD_ERR_INVALID_ARG;

    /* tabla de offsets ddram para lcd 20x4 */
    static const uint8_t row_addr[4] = {
        LCD_ROW0_ADDR,
        LCD_ROW1_ADDR,
        LCD_ROW2_ADDR,
        LCD_ROW3_ADDR,
    };

    uint8_t addr = LCD_CMD_SET_DDRAM | (row_addr[row] + col);
    return lcd_send_cmd(lcd, addr);
}

/* ============================================================
 * lcd_put_char
 * ============================================================ */
lcd_status_t lcd_put_char(lcd_handle_t *lcd, char c)
{
    if (lcd == NULL) return LCD_ERR_INVALID_ARG;
    return lcd_send_data(lcd, (uint8_t)c);
}

/* ============================================================
 * lcd_print
 * ============================================================ */
lcd_status_t lcd_print(lcd_handle_t *lcd, const char *str)
{
    if (lcd == NULL || str == NULL) return LCD_ERR_INVALID_ARG;

    lcd_status_t st = LCD_OK;
    while (*str != '\0' && st == LCD_OK) {
        st = lcd_send_data(lcd, (uint8_t)*str);
        str++;
    }
    return st;
}

/* ============================================================
 * lcd_backlight
 * ============================================================ */
lcd_status_t lcd_backlight(lcd_handle_t *lcd, uint8_t on)
{
    if (lcd == NULL) return LCD_ERR_INVALID_ARG;

    lcd->backlight = on ? LCD_BIT_BL : 0U;

    /* enviar un byte vacio para que el pcf8574 actualice el pin bl */
    return pcf8574_write(lcd, lcd->backlight);
}

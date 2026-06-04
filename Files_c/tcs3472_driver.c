#include "tcs3472_driver.h"

/* ============================================================
 * Internal helpers
 * ============================================================ */


static void delay_ms(uint32_t ms)
{
    /* ~4 cycles per iteration at 16 MHz → 4000 iter ≈ 1 ms */
    for (uint32_t i = 0; i < ms * 4000U; i++)
    {
        __asm volatile ("nop");
    }
}
 
/**
 * @brief  Write one byte to a TCS3472 register.
 */
static tcs3472_status_t tcs3472_write_reg(I2C_RegDef_t *i2c, uint8_t reg, uint8_t val)
{
    uint8_t data = val;
    if (i2c_writeRegDevice(i2c, TCS3472_ADDR, TCS3472_CMD_BYTE(reg), &data, 1) != I2C_OK)
        return TCS3472_ERR_I2C;
 
    return TCS3472_OK;
}
 
/**
 * @brief  Read one byte from a TCS3472 register.
 */
static tcs3472_status_t tcs3472_read_reg(I2C_RegDef_t *i2c, uint8_t reg, uint8_t *out)
{
    if (i2c_readRegDevice(i2c, TCS3472_ADDR, TCS3472_CMD_BYTE(reg), out, 1) != I2C_OK)
        return TCS3472_ERR_I2C;
 
    return TCS3472_OK;
}
 
/**
 * @brief  Read multiple consecutive bytes using auto-increment command.
 */
static tcs3472_status_t tcs3472_read_burst(I2C_RegDef_t *i2c, uint8_t reg, uint8_t *buf, size_t len)
{
    if (i2c_readRegDevice(i2c, TCS3472_ADDR, TCS3472_CMD(reg), buf, len) != I2C_OK)
        return TCS3472_ERR_I2C;
 
    return TCS3472_OK;
}
 
/**
 * @brief  Verify sensor presence by reading the ID register.
 */
static tcs3472_status_t tcs3472_check_id(I2C_RegDef_t *i2c)
{
    uint8_t id = 0;
    tcs3472_status_t st = tcs3472_read_reg(i2c, TCS3472_REG_ID, &id);
    if (st != TCS3472_OK)
        return st;
 
    if (id != TCS3472_ID_TCS34721_25 &&
        id != TCS3472_ID_TCS34723_27)
        return TCS3472_ERR_WRONG_ID;
 
    return TCS3472_OK;
}
 
/**
 * @brief  Poll STATUS register until AVALID is set or timeout expires.
 */
static tcs3472_status_t tcs3472_wait_valid(I2C_RegDef_t *i2c)
{
    uint8_t  status  = 0;
    uint32_t timeout = 100000U;
 
    do {
        tcs3472_status_t st = tcs3472_read_reg(i2c, TCS3472_REG_STATUS, &status);
        if (st != TCS3472_OK)
            return st;
 
        if (timeout-- == 0)
            return TCS3472_ERR_TIMEOUT;
 
    } while (!(status & TCS3472_STATUS_AVALID));
 
    return TCS3472_OK;
}
 
/* ============================================================
 * Public API implementation
 * ============================================================ */
 
tcs3472_status_t tcs3472_init(const tcs3472_config_t *config)
{
    if (config == NULL || config->i2c == NULL)
        return TCS3472_ERR_INVALID_ARG;
 
    I2C_RegDef_t *i2c = config->i2c;
    tcs3472_status_t st;
 
    /* 1 — Verify device ID */
    st = tcs3472_check_id(i2c);
    if (st != TCS3472_OK)
        return st;
 
    /* 2 — Power ON: enable internal oscillator (PON) */
    st = tcs3472_write_reg(i2c, TCS3472_REG_ENABLE, TCS3472_ENABLE_PON);
    if (st != TCS3472_OK)
        return st;
 
    /* 3 — Warm-up delay: datasheet requires ≥2.4 ms after PON before
     *     the first RGBC integration cycle can start.                 */
    delay_ms(3U);
 
    /* 4 — Apply gain and integration time via the unified config call */
    st = tcs3472_config(i2c, config->gain, config->atime);
    if (st != TCS3472_OK)
        return st;
 
    /* 5 — Enable RGBC (PON + AEN) */
    st = tcs3472_write_reg(i2c,
                           TCS3472_REG_ENABLE,
                           TCS3472_ENABLE_PON | TCS3472_ENABLE_AEN);
    if (st != TCS3472_OK)
        return st;
 
    return TCS3472_OK;
}
 
tcs3472_status_t tcs3472_config(I2C_RegDef_t   *i2c,
                                 tcs3472_gain_t  gain,
                                 tcs3472_atime_t atime)
{
    if (i2c == NULL)
        return TCS3472_ERR_INVALID_ARG;
 
    if (gain != TCS3472_GAIN_1X  &&
        gain != TCS3472_GAIN_4X  &&
        gain != TCS3472_GAIN_16X &&
        gain != TCS3472_GAIN_60X)
        return TCS3472_ERR_INVALID_ARG;
 
    tcs3472_status_t st;
 
    st = tcs3472_write_reg(i2c, TCS3472_REG_CONTROL, (uint8_t)gain);
    if (st != TCS3472_OK)
        return st;
 
    st = tcs3472_write_reg(i2c, TCS3472_REG_ATIME, (uint8_t)atime);
    if (st != TCS3472_OK)
        return st;
 
    return TCS3472_OK;
}
 
tcs3472_status_t tcs3472_read_rgbc(I2C_RegDef_t *i2c, tcs3472_rgbc_t *rgbc)
{
    if (i2c == NULL || rgbc == NULL)
        return TCS3472_ERR_INVALID_ARG;
 
    /* Wait for integration cycle to complete */
    tcs3472_status_t st = tcs3472_wait_valid(i2c);
    if (st != TCS3472_OK)
        return st;
 
    /* Burst-read all 8 RGBC bytes (registers 0x14 → 0x1B) */
    uint8_t raw[8] = {0};
    st = tcs3472_read_burst(i2c, TCS3472_REG_CDATAL, raw, 8U);
    if (st != TCS3472_OK)
        return st;
 
    /* Reconstruct 16-bit values: low byte | (high byte << 8) */
    rgbc->clear = (uint16_t)(raw[0] | ((uint16_t)raw[1] << 8));
    rgbc->red   = (uint16_t)(raw[2] | ((uint16_t)raw[3] << 8));
    rgbc->green = (uint16_t)(raw[4] | ((uint16_t)raw[5] << 8));
    rgbc->blue  = (uint16_t)(raw[6] | ((uint16_t)raw[7] << 8));
 
    /* Check Clear saturation before returning */
    if (rgbc->clear >= TCS3472_ADC_MAX)
        return TCS3472_ERR_SATURATED;
 
    return TCS3472_OK;
}
 
tcs3472_status_t tcs3472_read_clear(I2C_RegDef_t *i2c, uint16_t *clear)
{
    if (i2c == NULL || clear == NULL)
        return TCS3472_ERR_INVALID_ARG;
 
    /* Wait for integration cycle to complete */
    tcs3472_status_t st = tcs3472_wait_valid(i2c);
    if (st != TCS3472_OK)
        return st;
 
    /* Read only the 2 Clear channel bytes (0x14, 0x15) */
    uint8_t raw[2] = {0};
    st = tcs3472_read_burst(i2c, TCS3472_REG_CDATAL, raw, 2U);
    if (st != TCS3472_OK)
        return st;
 
    *clear = (uint16_t)(raw[0] | ((uint16_t)raw[1] << 8));
 
    return TCS3472_OK;
}
 
tcs3472_status_t tcs3472_power_off(I2C_RegDef_t *i2c)
{
    if (i2c == NULL)
        return TCS3472_ERR_INVALID_ARG;
 
    return tcs3472_write_reg(i2c, TCS3472_REG_ENABLE, 0x00U);
}
 
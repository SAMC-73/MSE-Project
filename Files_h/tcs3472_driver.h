/**
 * @file    tcs3472_driver.h
 * @brief   Driver for the TCS3472 Color Light-to-Digital Converter
 *
 * Provides initialization, configuration, and data acquisition functions
 * for the TCS3472 RGB + Clear light sensor over I2C.
 *
 * Dependencies:
 *   - i2c_driver.h  (i2c_writeRegDevice, i2c_readRegDevice)
 *
 * @author  Carlos Villarreal
 * @version 2.0
 *
 * @ref     TCS3472 Datasheet — TAOS135, August 2012
 */
 
#ifndef TCS3472_DRIVER_H
#define TCS3472_DRIVER_H
 
#include <stdint.h>
#include <stddef.h>
#include "i2c_driver.h"
 
#ifdef __cplusplus
extern "C" {
#endif
 
/* ============================================================
 * Device address
 * ============================================================ */
 
#define TCS3472_ADDR            0x29U   /**< 7-bit I2C address (TCS34725/27) */
 
/* ============================================================
 * Command register
 * ============================================================ */
 
#define TCS3472_CMD(reg)        (0xA0U | (reg))   /**< Auto-increment protocol */
#define TCS3472_CMD_BYTE(reg)   (0x80U | (reg))   /**< Single byte protocol    */
 
/* ============================================================
 * Register addresses  (Table 3 — Register Address)
 * ============================================================ */
 
#define TCS3472_REG_ENABLE      0x00U
#define TCS3472_REG_ATIME       0x01U
#define TCS3472_REG_CONTROL     0x0FU
#define TCS3472_REG_ID          0x12U
#define TCS3472_REG_STATUS      0x13U
#define TCS3472_REG_CDATAL      0x14U
#define TCS3472_REG_RDATAL      0x16U
#define TCS3472_REG_GDATAL      0x18U
#define TCS3472_REG_BDATAL      0x1AU
 
/* ============================================================
 * ENABLE register bit masks  (Table 5)
 * ============================================================ */
 
#define TCS3472_ENABLE_PON      (1U << 0)   /**< Power ON    */
#define TCS3472_ENABLE_AEN      (1U << 1)   /**< RGBC enable */
 
/* ============================================================
 * STATUS register bit masks  (Table 13)
 * ============================================================ */
 
#define TCS3472_STATUS_AVALID   (1U << 0)   /**< RGBC data valid */
 
/* ============================================================
 * Device ID  (Table 12)
 * ============================================================ */
 
#define TCS3472_ID_TCS34721_25  0x44U
#define TCS3472_ID_TCS34723_27  0x4DU
 
/* ============================================================
 * ADC saturation threshold
 * ============================================================ */
 
#define TCS3472_ADC_MAX         65535U
 
/* ============================================================
 * AGAIN — Gain values  (Table 11 — Control Register 0x0F)
 * ============================================================ */
 
typedef enum
{
    TCS3472_GAIN_1X  = 0x00U,
    TCS3472_GAIN_4X  = 0x01U,
    TCS3472_GAIN_16X = 0x02U,
    TCS3472_GAIN_60X = 0x03U,
} tcs3472_gain_t;
 
/* ============================================================
 * ATIME — Integration time  (Table 6 — RGBC Timing Register 0x01)
 * ============================================================ */
 
typedef uint8_t tcs3472_atime_t;
 
#define TCS3472_ATIME_2_4MS   0xFFU
#define TCS3472_ATIME_24MS    0xF6U
#define TCS3472_ATIME_101MS   0xD5U
#define TCS3472_ATIME_154MS   0xC0U
#define TCS3472_ATIME_700MS   0x00U
 
/* ============================================================
 * Return codes
 * ============================================================ */
 
typedef enum
{
    TCS3472_OK              =  0,
    TCS3472_ERR_INVALID_ARG = -1,
    TCS3472_ERR_I2C         = -2,
    TCS3472_ERR_WRONG_ID    = -3,
    TCS3472_ERR_NOT_READY   = -4,
    TCS3472_ERR_SATURATED   = -5,
    TCS3472_ERR_TIMEOUT     = -6,
} tcs3472_status_t;
 
/* ============================================================
 * Data types
 * ============================================================ */
 
/**
 * @brief  Holds one complete RGBC measurement.
 */
typedef struct
{
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} tcs3472_rgbc_t;
 
/**
 * @brief  Run-time configuration for tcs3472_init().
 */
typedef struct
{
    I2C_RegDef_t    *i2c;
    tcs3472_gain_t   gain;
    tcs3472_atime_t  atime;
} tcs3472_config_t;
 
/* ============================================================
 * Public API  (4 essential functions + power off)
 * ============================================================ */
 
/**
 * @brief  Initialize the TCS3472 sensor.
 *
 * Verifies device ID, powers on the oscillator, waits the 2.4 ms
 * warm-up delay, applies gain and integration time, then enables RGBC.
 *
 * @param[in]  config  Pointer to a populated tcs3472_config_t.
 *
 * @retval  TCS3472_OK              Sensor ready.
 * @retval  TCS3472_ERR_INVALID_ARG config or i2c is NULL.
 * @retval  TCS3472_ERR_WRONG_ID    Device ID mismatch.
 * @retval  TCS3472_ERR_I2C         I2C transaction failed.
 */
tcs3472_status_t tcs3472_init(const tcs3472_config_t *config);
 
/**
 * @brief  Reconfigure gain and integration time at runtime.
 *
 * Consolidates tcs3472_set_gain() and tcs3472_set_atime() into a
 * single call, which is the typical use case.
 *
 * @param[in]  i2c    Initialized I2C peripheral instance.
 * @param[in]  gain   New AGAIN setting (tcs3472_gain_t).
 * @param[in]  atime  New integration time (tcs3472_atime_t).
 *
 * @retval  TCS3472_OK              Configuration applied.
 * @retval  TCS3472_ERR_INVALID_ARG i2c is NULL or gain is invalid.
 * @retval  TCS3472_ERR_I2C         I2C transaction failed.
 */
tcs3472_status_t tcs3472_config(I2C_RegDef_t   *i2c,
                                 tcs3472_gain_t  gain,
                                 tcs3472_atime_t atime);
 
/**
 * @brief  Read all four RGBC channels in a single burst.
 *
 * Waits for AVALID internally, then reads registers 0x14–0x1B.
 * Returns TCS3472_ERR_SATURATED if the Clear channel is at full scale.
 *
 * @param[in]  i2c   Initialized I2C peripheral instance.
 * @param[out] rgbc  Pointer to store the result.
 *
 * @retval  TCS3472_OK              Data written into rgbc.
 * @retval  TCS3472_ERR_INVALID_ARG i2c or rgbc is NULL.
 * @retval  TCS3472_ERR_SATURATED   Clear channel saturated.
 * @retval  TCS3472_ERR_TIMEOUT     AVALID wait timed out.
 * @retval  TCS3472_ERR_I2C         I2C transaction failed.
 */
tcs3472_status_t tcs3472_read_rgbc(I2C_RegDef_t *i2c, tcs3472_rgbc_t *rgbc);
 
/**
 * @brief  Read only the Clear channel.
 *
 * Lighter alternative to tcs3472_read_rgbc() when only Clear is
 * needed, e.g. during an AGC/AEC control loop.
 *
 * @param[in]  i2c    Initialized I2C peripheral instance.
 * @param[out] clear  Pointer to store the Clear channel value.
 *
 * @retval  TCS3472_OK              Value written into clear.
 * @retval  TCS3472_ERR_INVALID_ARG i2c or clear is NULL.
 * @retval  TCS3472_ERR_TIMEOUT     AVALID wait timed out.
 * @retval  TCS3472_ERR_I2C         I2C transaction failed.
 */
tcs3472_status_t tcs3472_read_clear(I2C_RegDef_t *i2c, uint16_t *clear);
 
/**
 * @brief  Power the sensor off (clears PON and AEN).
 *
 * @param[in]  i2c  Initialized I2C peripheral instance.
 *
 * @retval  TCS3472_OK       Sensor powered down.
 * @retval  TCS3472_ERR_I2C  I2C transaction failed.
 */
tcs3472_status_t tcs3472_power_off(I2C_RegDef_t *i2c);
 
/* ============================================================
 * Usage example
 * ============================================================
 *
 * --- Initialization ---
 *
 *   tcs3472_config_t cfg = {
 *       .i2c   = I2C1,
 *       .gain  = TCS3472_GAIN_4X,
 *       .atime = TCS3472_ATIME_154MS,
 *   };
 *   tcs3472_init(&cfg);
 *
 *
 * --- Read all channels ---
 *
 *   tcs3472_rgbc_t data;
 *   tcs3472_read_rgbc(I2C1, &data);
 *   // data.clear, data.red, data.green, data.blue
 *
 *
 * --- Read only Clear (for AGC/AEC loop) ---
 *
 *   uint16_t clear;
 *   tcs3472_read_clear(I2C1, &clear);
 *
 *
 * --- Reconfigure at runtime ---
 *
 *   tcs3472_config(I2C1, TCS3472_GAIN_16X, TCS3472_ATIME_24MS);
 *
 * ============================================================ */
 
#ifdef __cplusplus
}
#endif
 
#endif /* TCS3472_DRIVER_H */
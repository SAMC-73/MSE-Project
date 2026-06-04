/**
 * @file    tcs3472_control.h
 * @brief   Módulo de Control de Lazo Cerrado para el sensor TCS3472.
 *
 * Implementa control automático de ganancia (AGC) y exposición (AEC)
 * mediante dos fases secuenciales:
 *
 *   Fase 1 — Barrido de AGAIN: con ATIME en máximo (700 ms) recorre
 *            1× → 4× → 16× → 60× hasta que clear >= SETPOINT.
 *
 *   Fase 2 — Búsqueda binaria de ATIME: con AGAIN fijo, bisecta el registro
 *            ATIME [0x00, 0xFF] hasta que |clear − SETPOINT| ≤ HYSTERESIS.
 *
 * rgbc_control_update() contiene el bucle interno y retorna solo cuando
 * el sistema converge o falla.
 */

#ifndef CONTROL_RGBC_H
#define CONTROL_RGBC_H

#include <stdint.h>
#include "tcs3472_driver.h"
#include "serial.h"

/* =========================================================================
 * Parámetros del setpoint
 * ========================================================================= */

#define RGBC_CONTROL_SETPOINT       6000U   /**< Valor ideal del canal Clear */
#define RGBC_CONTROL_HYSTERESIS      300U   /**< Banda muerta: ±5 % del setpoint */
#define RGBC_CONTROL_MAX_COUNTS    60000U   /**< Umbral de saturación crítica */
#define RGBC_CONTROL_MIN_COUNTS       10U   /**< Umbral mínimo: oscuridad total */

/* =========================================================================
 * Códigos de retorno
 * ========================================================================= */

typedef enum {
    CONTROL_OK = 0,           /**< Control exitoso y sistema estable */
    CONTROL_ERR_INVALID_ARG,  /**< Argumento nulo o fuera de rango */
    CONTROL_ERR_I2C,          /**< Error en comunicación I2C */
    CONTROL_SATURATED,        /**< Sensor saturado (clear >= MAX_COUNTS) */
    CONTROL_UNDEREXPOSED,     /**< Subexpuesto (60× + 700 ms insuficiente) */
} control_status_t;

/* =========================================================================
 * Estructura de estado del sistema de control
 * ========================================================================= */

typedef struct {
    tcs3472_config_t sensor_cfg;   /**< Configuración activa del sensor */
    tcs3472_rgbc_t   last_raw;     /**< Última lectura cruda (RGBC) */
    float            norm_clear;   /**< Clear normalizado por ganancia × ciclos */
    uint8_t          is_stable;    /**< 1 si el sistema alcanzó la banda de histéresis */
} rgbc_control_t;

/* =========================================================================
 * API pública
 * ========================================================================= */

/**
 * @brief  Inicializa el sistema de control y el hardware del sensor.
 * @param  ctrl  Puntero a la estructura de control (será llenada).
 * @param  i2c   Instancia del periférico I2C (I2C1, I2C2, I2C3).
 * @return CONTROL_OK si éxito, CONTROL_ERR_INVALID_ARG o CONTROL_ERR_I2C.
 */
control_status_t rgbc_control_init(rgbc_control_t *ctrl, I2C_RegDef_t *i2c);

/**
 * @brief  Ejecuta el ciclo de control completo (Fase 1 + Fase 2).
 * @param  ctrl  Puntero a la estructura de control.
 * @return CONTROL_OK si estable, CONTROL_UNDEREXPOSED si muy oscuro,
 *         CONTROL_ERR_I2C si error de comunicación.
 */
control_status_t rgbc_control_update(rgbc_control_t *ctrl);

/**
 * @brief  Devuelve los canales R, G, B crudos.
 * @param  ctrl   Puntero a la estructura de control.
 * @param  red    Canal rojo crudo.
 * @param  green  Canal verde crudo.
 * @param  blue   Canal azul crudo.
 */
void rgbc_get_raw_data(const rgbc_control_t *ctrl,
                               float *red, float *green, float *blue);

#endif /* CONTROL_RGBC_H */
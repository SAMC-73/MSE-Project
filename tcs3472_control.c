/**
 * @file    tcs3472_control.c
 * @brief   Módulo de Control de Lazo Cerrado – Algoritmo de dos fases.
 *
 * ESTRATEGIA DE CONTROL — DOS FASES SECUENCIALES
 * ────────────────────────────────────────────────
 *
 * Fase 1 — Barrido de AGAIN (AGC):
 *   Con ATIME fijo en su valor máximo (0x00 = 700 ms, máxima sensibilidad),
 *   se recorren los niveles de ganancia en orden ascendente:
 *     1× → 4× → 16× → 60×
 *   Se detiene en el primer nivel donde clear >= SETPOINT.
 *   Ese nivel de AGAIN queda fijo para la Fase 2.
 *   Si ni siquiera con 60× y 700 ms se supera el setpoint → CONTROL_UNDEREXPOSED.
 *
 * Fase 2 — Búsqueda binaria de ATIME (AEC):
 *   AGAIN fijo. Se bisecta el espacio del registro ATIME [0x00, 0xFF]:
 *     Registro bajo  (0x00) → 700 ms → más ciclos → más cuentas
 *     Registro alto  (0xFF) → 2.4 ms → menos ciclos → menos cuentas
 *   Reglas de bisección:
 *     clear > SP + HYS  → acortar tiempo → atime_lo_reg = mid + 1
 *     clear < SP - HYS  → alargar tiempo → atime_hi_reg = mid - 1
 *     |clear − SP| ≤ HYS → ESTABLE, terminar
 *
 *   Si el espacio de búsqueda se agota sin encontrar la banda exacta, se
 *   acepta el último valor como la mejor aproximación posible.
 *
 * El bucle interno imprime UNA línea por lectura con el formato:
 *   RAW: %u | NORM_C: %d | GAIN: %d | ATIME: %u | [%s]  Status: %d
 */

#include "tcs3472_control.h"

/* ============================================================
 * Tabla de niveles de ganancia discretos (índice 0 = 1×, 3 = 60×)
 * ============================================================ */
static const tcs3472_gain_t gain_levels[4] = {
    TCS3472_GAIN_1X,
    TCS3472_GAIN_4X,
    TCS3472_GAIN_16X,
    TCS3472_GAIN_60X,
};

/* ============================================================
 * Funciones auxiliares privadas
 * ============================================================ */

/** Ciclos de integración correspondientes a un valor de registro ATIME. */
static float get_integration_cycles(tcs3472_atime_t atime)
{
    return (float)(256U - (uint8_t)atime);
}

/** Multiplicador numérico del nivel de ganancia AGAIN. */
static float get_gain_multiplier(tcs3472_gain_t gain)
{
    switch (gain) {
        case TCS3472_GAIN_1X:  return 1.0f;
        case TCS3472_GAIN_4X:  return 4.0f;
        case TCS3472_GAIN_16X: return 16.0f;
        case TCS3472_GAIN_60X: return 60.0f;
        default:               return 1.0f;
    }
}

/** Devuelve el multiplicador entero de AGAIN para impresión legible. */
static int gain_as_int(tcs3472_gain_t gain)
{
    switch (gain) {
        case TCS3472_GAIN_1X:  return 1;
        case TCS3472_GAIN_4X:  return 4;
        case TCS3472_GAIN_16X: return 16;
        case TCS3472_GAIN_60X: return 60;
        default:               return 1;
    }
}

/** Recalcula norm_clear con la configuración actualmente cargada en ctrl. */
static void update_norm_clear(rgbc_control_t *ctrl)
{
    float gain_val = get_gain_multiplier(ctrl->sensor_cfg.gain);
    float time_val = get_integration_cycles(ctrl->sensor_cfg.atime);
    ctrl->norm_clear = (float)ctrl->last_raw.clear / (gain_val * time_val);
}

/**
 * @brief  Espera un ciclo completo de integración con el ATIME actual.
 * @param  atime  Valor del registro ATIME (0x00..0xFF)
 */
static void wait_integration_cycle(tcs3472_atime_t atime)
{
    float    integration_ms = (float)(256U - (uint8_t)atime) * 2.4f;
    uint32_t delay_ms       = (uint32_t)integration_ms + 1U;
    for (uint32_t i = 0; i < delay_ms * 4000U; i++) {
        __asm volatile ("nop");
    }
}

/**
 * @brief  Imprime la línea de estado única por ciclo de muestreo.
 * @param  ctrl    Puntero a la estructura de control.
 * @param  status  Código de estado actual.
 */
static void print_status_line(const rgbc_control_t *ctrl, control_status_t status)
{
    serial_printf("RAW: %u | NORM_C: %d | GAIN: %d | ATIME: %u | [%s]  Status: %d\r\n",
                  ctrl->last_raw.clear,
                  (int)(ctrl->norm_clear * 1000.0f),
                  gain_as_int(ctrl->sensor_cfg.gain),
                  (unsigned int)(uint8_t)ctrl->sensor_cfg.atime,
                  ctrl->is_stable ? "ESTABLE" : "AJUSTANDO",
                  (int)status);
}

/* ============================================================
 * Implementación de la API pública
 * ============================================================ */

control_status_t rgbc_control_init(rgbc_control_t *ctrl, I2C_RegDef_t *i2c)
{
    if (ctrl == NULL || i2c == NULL)
        return CONTROL_ERR_INVALID_ARG;

    ctrl->sensor_cfg.i2c   = i2c;
    ctrl->sensor_cfg.gain  = TCS3472_GAIN_1X;
    ctrl->sensor_cfg.atime = TCS3472_ATIME_700MS;
    ctrl->is_stable        = 0U;
    ctrl->norm_clear       = 0.0f;

    if (tcs3472_init(&(ctrl->sensor_cfg)) != TCS3472_OK)
        return CONTROL_ERR_I2C;

    return CONTROL_OK;
}

control_status_t rgbc_control_update(rgbc_control_t *ctrl)
{
    if (ctrl == NULL) return CONTROL_ERR_INVALID_ARG;

    ctrl->is_stable = 0U;

    /* ================================================================
     * FASE 1: Barrido de AGAIN con ATIME en máximo (700 ms / 0x00)
     * ================================================================ */

    /* Fijar ATIME al máximo para maximizar la sensibilidad */
    ctrl->sensor_cfg.atime = TCS3472_ATIME_700MS;
    if (tcs3472_config(ctrl->sensor_cfg.i2c, ctrl->sensor_cfg.gain, ctrl->sensor_cfg.atime) != TCS3472_OK)
        return CONTROL_ERR_I2C;

    uint8_t gain_found = 0U;

    for (uint8_t i = 0U; i < 4U; i++) {

        /* Aplicar el siguiente nivel de ganancia */
        ctrl->sensor_cfg.gain = gain_levels[i];
        if (tcs3472_config(ctrl->sensor_cfg.i2c, ctrl->sensor_cfg.gain, ctrl->sensor_cfg.atime) != TCS3472_OK)
            return CONTROL_ERR_I2C;

        /* Esperar un ciclo completo con la nueva ganancia (ATIME=0 -> 700 ms) */
        wait_integration_cycle(ctrl->sensor_cfg.atime);

        /* Leer el sensor */
        if (tcs3472_read_rgbc(ctrl->sensor_cfg.i2c, &ctrl->last_raw) != TCS3472_OK)
            return CONTROL_ERR_I2C;

        update_norm_clear(ctrl);
        print_status_line(ctrl, CONTROL_OK);

        /* ¿Canal Clear superó el setpoint con esta ganancia? */
        if (ctrl->last_raw.clear >= RGBC_CONTROL_SETPOINT) {
            gain_found = 1U;
            break;
        }
    }

    if (!gain_found)
        return CONTROL_UNDEREXPOSED;

    /* Comprobación rápida post-Fase 1: ¿ya estamos dentro de la banda? */
    {
        uint16_t c = ctrl->last_raw.clear;
        if (c >= (RGBC_CONTROL_SETPOINT - RGBC_CONTROL_HYSTERESIS) &&
            c <= (RGBC_CONTROL_SETPOINT + RGBC_CONTROL_HYSTERESIS))
        {
            ctrl->is_stable = 1U;
            print_status_line(ctrl, CONTROL_OK);
            return CONTROL_OK;
        }
    }

    /* ================================================================
     * FASE 2: Búsqueda binaria de ATIME con AGAIN fijo
     * ================================================================ */

    uint8_t atime_lo_reg = 0x00U;   /* Extremo sensible:   700 ms */
    uint8_t atime_hi_reg = 0xFFU;   /* Extremo insensible: 2.4 ms */

    while (atime_lo_reg <= atime_hi_reg) {

        /* Punto medio del espacio de búsqueda actual */
        uint8_t atime_mid_reg =
            (uint8_t)(((uint16_t)atime_lo_reg + (uint16_t)atime_hi_reg) / 2U);

        /* Aplicar nuevo ATIME al sensor */
        ctrl->sensor_cfg.atime = (tcs3472_atime_t)atime_mid_reg;
        if (tcs3472_config(ctrl->sensor_cfg.i2c, ctrl->sensor_cfg.gain, ctrl->sensor_cfg.atime) != TCS3472_OK)
            return CONTROL_ERR_I2C;

        /* Esperar un ciclo completo con el nuevo ATIME */
        wait_integration_cycle(ctrl->sensor_cfg.atime);

        /* Leer con el nuevo tiempo de integración */
        if (tcs3472_read_rgbc(ctrl->sensor_cfg.i2c, &ctrl->last_raw) != TCS3472_OK)
            return CONTROL_ERR_I2C;

        update_norm_clear(ctrl);
        print_status_line(ctrl, CONTROL_OK);

        uint16_t c = ctrl->last_raw.clear;

        /* --- Decisión de bisección --- */
        if (c >= (RGBC_CONTROL_SETPOINT - RGBC_CONTROL_HYSTERESIS) &&
            c <= (RGBC_CONTROL_SETPOINT + RGBC_CONTROL_HYSTERESIS))
        {
            ctrl->is_stable = 1U;
            return CONTROL_OK;
        }
        else if (c >= RGBC_CONTROL_MAX_COUNTS ||
                 c  > (RGBC_CONTROL_SETPOINT + RGBC_CONTROL_HYSTERESIS))
        {
            /* Demasiadas cuentas → reducir tiempo (aumentar registro ATIME) */
            if (atime_mid_reg == 0xFFU) break;
            atime_lo_reg = atime_mid_reg + 1U;
        }
        else
        {
            /* Muy pocas cuentas → aumentar tiempo (disminuir registro ATIME) */
            if (atime_mid_reg == 0x00U) break;
            atime_hi_reg = atime_mid_reg - 1U;
        }
    }

    /* El espacio de búsqueda se agotó sin encontrar la banda exacta.
     * Se acepta el último valor como la mejor aproximación. */
    ctrl->is_stable = 1U;
    return CONTROL_OK;
}

void rgbc_get_raw_data(const rgbc_control_t *ctrl,
                               float *red, float *green, float *blue)
{
    if (ctrl == NULL || red == NULL || green == NULL || blue == NULL)
        return;

    *red   = (float)ctrl->last_raw.red;
    *green = (float)ctrl->last_raw.green;
    *blue  = (float)ctrl->last_raw.blue;
}
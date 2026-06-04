/**
 * @file    fruit_db.h
 * @brief   Base de datos dinámica y motor de clasificación geométrica (3D) de frutas.
 */

#ifndef FRUIT_DB_H
#define FRUIT_DB_H

#include <stdint.h>

#define DB_MAX_NAME_LEN    20U    // Máximo de caracteres para el nombre de la fruta (incluyendo '\0')
#define DB_MAX_FRUITS      10U    // Capacidad máxima de la base de datos
#define DB_MATCH_TOLERANCE 0.1f  /**< Umbral máximo de distancia cromática para aceptar coincidencia */

/* Estructura para almacenar datos de color de fruta */

/**
 * @brief Estructura de almacenamiento para la firma cromática
 */
typedef struct {
    char nombre[DB_MAX_NAME_LEN];   // String del nombre
    float r_ref;    // Proporción de rojo
    float g_ref;    // Proporción de verde
    float b_ref;    // Proporción de azul
    uint8_t es_valida;  // 1 = Ocupado, 0 = Vacío
} fruta_registro_t;

typedef enum {
    FRUIT_DB_OK = 0,
    FRUIT_DB_ERR_FULL,
    FRUIT_DB_ERR_INVALID_INDEX,
    FRUIT_DB_ERR_NO_MATCH
} fruit_db_status_t;

/* --- Prototipos de la API --- */

/**
 * @brief  Inicializa el arreglo y carga las frutas de fábrica.
 */
fruit_db_status_t fruit_db_init(void);

/**
 * @brief  Agrega una nueva fruta calibrada a la base de datos.
 * @param  nombre  Nombre de la fruta a guardar.
 * @param  r, g, b Valores crudos o normalizados de la lectura promediada.
 * @return Estado de la operación.
 */
fruit_db_status_t fruit_db_add(const char *nombre, float r_raw, float g_raw, float b_raw);

/**
 * @brief  Compara una lectura contra una referencia en la base de datos para identificar la fruta.
 * @param  r_raw, g_raw, b_raw Lecturas actuales del sensor.
 * @param  target_idx Índice de la fruta de referencia en la base de datos.
 * @return Estado de la operación.
 */
fruit_db_status_t fruit_db_compare(float r_raw, float g_raw, float b_raw, uint8_t target_idx);

/**
 * @brief  Obtiene la cantidad actual de frutas válidas en la base de datos.
 * @return Número del 0 al DB_MAX_FRUITS.
 */
uint8_t fruit_db_get_count(void);

/**
 * @brief  Elimina una fruta por su índice y reordena el arreglo para evitar huecos.
 * @param  index Índice de la fruta a eliminar.
 * @return Estado de la operación.
 */
fruit_db_status_t fruit_db_delete(uint8_t index);

/**
 * @brief  Obtiene un puntero de solo lectura a la base de datos para los menús.
 */
const fruta_registro_t* fruit_db_get_table(void);

#endif // FRUIT_DB_H
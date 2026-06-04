/**
 * @file    fruit_db.c
 * @brief   Módulo para Datos y Clasificación de Frutas.
 */

#include "fruit_db.h"

/* --- Base de valores cromáticos estática --- */
static fruta_registro_t base_datos_frutas[DB_MAX_FRUITS]; // Arreglo de registros de frutas
static uint8_t total_frutas = 0; // Variable de conteo actual de frutas

/* --- Funciones auxiliares locales --- */

static uint8_t db_strcpy(char *dest, const char *src)
{
    uint8_t i = 0;
    while (src[i] != '\0' && i < (DB_MAX_NAME_LEN - 1)) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return i;
}

static float fast_sqrtf(float x)
{
    if (x <= 0.0f) return 0.0f;
    float y = x;
    for (int i = 0; i < 5; i++) {
        y = (y + x / y) * 0.5f;
    }
    return y;
}

fruit_db_status_t fruit_db_init(void)
{
    // Limpieza de la base de datos marcando
    // todos los espacios como vacíos
    for (uint8_t i = 0; i < DB_MAX_FRUITS; i++) {
        base_datos_frutas[i].es_valida = 0;
    }

    // Inclusión de frutas base.
    // Calibración previa realizada con el propio sensor
    db_strcpy(base_datos_frutas[0].nombre, "Manzana");
    base_datos_frutas[0].r_ref = 0.615f;
    base_datos_frutas[0].g_ref = 0.205f;
    base_datos_frutas[0].b_ref = 0.178f;
    base_datos_frutas[0].es_valida = 1;
    total_frutas++;
    
    db_strcpy(base_datos_frutas[1].nombre, "Naranja");
    base_datos_frutas[1].r_ref = 0.595f;
    base_datos_frutas[1].g_ref = 0.281f;
    base_datos_frutas[1].b_ref = 0.124f;
    base_datos_frutas[1].es_valida = 1;
    total_frutas++;

    db_strcpy(base_datos_frutas[2].nombre, "Plátano");
    base_datos_frutas[2].r_ref = 0.508f;
    base_datos_frutas[2].g_ref = 0.345f;
    base_datos_frutas[2].b_ref = 0.147f;
    base_datos_frutas[2].es_valida = 1;
    total_frutas++;

    db_strcpy(base_datos_frutas[3].nombre, "Tomate");
    base_datos_frutas[3].r_ref = 0.6f;
    base_datos_frutas[3].g_ref = 0.24f;
    base_datos_frutas[3].b_ref = 0.159f;
    base_datos_frutas[3].es_valida = 1;
    total_frutas++;

    db_strcpy(base_datos_frutas[4].nombre, "Uva");
    base_datos_frutas[4].r_ref = 0.422f;
    base_datos_frutas[4].g_ref = 0.324f;
    base_datos_frutas[4].b_ref = 0.254f;
    base_datos_frutas[4].es_valida = 1;
    total_frutas++;

    return FRUIT_DB_OK;
}

fruit_db_status_t fruit_db_add(const char *nombre, float r_raw, float g_raw, float b_raw)
{
    // Buscar la primera ranura vacía
    for (uint8_t i = 0; i < DB_MAX_FRUITS; i++) {
        if (base_datos_frutas[i].es_valida == 0) {
            
            // Calculamos proporciones relativas para aislar la crominancia
            float suma = r_raw + g_raw + b_raw;
            if (suma == 0.0f) suma = 1.0f; // Evitar división por cero
            
            db_strcpy(base_datos_frutas[i].nombre, nombre);
            base_datos_frutas[i].r_ref = r_raw / suma;
            base_datos_frutas[i].g_ref = g_raw / suma;
            base_datos_frutas[i].b_ref = b_raw / suma;
            base_datos_frutas[i].es_valida = 1;
            total_frutas++;
            return FRUIT_DB_OK; // Éxito
        }
    }
    return FRUIT_DB_ERR_FULL; // Error: Base de datos llena
}

fruit_db_status_t fruit_db_delete(uint8_t index)
{
    if (index >= total_frutas) {
        return FRUIT_DB_ERR_INVALID_INDEX; // Protección contra índices fuera de rango
    }

    // Desplazar todos los elementos un espacio hacia atrás
    
    for (uint8_t i = index; i < total_frutas - 1; i++) {
        // C permite copiar structs directamente
        base_datos_frutas[i] = base_datos_frutas[i + 1]; 
    }

    // Limpiar la última ranura que quedó duplicada tras el desplazamiento
    base_datos_frutas[total_frutas - 1].es_valida = 0;
    
    // Decrementar el conteo total de frutas válidas
    total_frutas--;

    return FRUIT_DB_OK; // Éxito
}

fruit_db_status_t fruit_db_compare(float r_raw, float g_raw, float b_raw, uint8_t target_idx)
{

    // Validación de índice y disponibilidad de la fruta
    if (target_idx >= DB_MAX_FRUITS || base_datos_frutas[target_idx].es_valida == 0) {
        return FRUIT_DB_ERR_INVALID_INDEX; 
    }

    // 1. Obtención de datos de la fruta de referencia
    fruta_registro_t fruta = base_datos_frutas[target_idx];

    // 2. Normalizar la lectura actual a proporciones cromáticas

    float suma = r_raw + g_raw + b_raw;  //Suma total para normalizar y aislar la crominancia
    if(suma == 0.0f) {suma = 1.0f;} // Evitar división por cero en caso de oscuridad total
    
    float r_med = r_raw / suma;     // Proporción de rojo
    float g_med = g_raw / suma;     // Proporción de verde
    float b_med = b_raw / suma;     // Proporción de azul

    // 3. Calcular la distancia euclidiana entre la lectura y la referencia
    float dr = r_med - fruta.r_ref; // Diferencia en rojo
    float dg = g_med - fruta.g_ref; // Diferencia en verde
    float db = b_med - fruta.b_ref; // Diferencia en azul
    
    float distancia = fast_sqrtf(dr * dr + dg * dg + db * db);
    
    // 3. Comparar con el umbral de tolerancia
    if (distancia <= DB_MATCH_TOLERANCE) {
        return FRUIT_DB_OK; // Coincidencia aceptable
    }

    return FRUIT_DB_ERR_NO_MATCH;
}


const fruta_registro_t* fruit_db_get_table(void)
{
    return base_datos_frutas;
}

uint8_t fruit_db_get_count(void)
{
    return total_frutas;
}
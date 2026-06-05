/**
 * @file    fruit_db.h
 * @brief   Dynamic fruit database and geometric 3D classification engine.
 */

#ifndef FRUIT_DB_H
#define FRUIT_DB_H

#include <stdint.h>

#define DB_MAX_NAME_LEN    20U   /**< Maximum fruit name length, including '\0'. */
#define DB_MAX_FRUITS      10U   /**< Maximum database capacity. */
#define DB_MATCH_TOLERANCE 0.1f  /**< Maximum chromatic distance used to accept a match. */

/**
 * @brief Stores the chromatic signature of a fruit.
 */
typedef struct {
    char name[DB_MAX_NAME_LEN];  /**< Fruit name string. */
    float r_ref;                 /**< Red color proportion. */
    float g_ref;                 /**< Green color proportion. */
    float b_ref;                 /**< Blue color proportion. */
    uint8_t is_valid;            /**< 1 = occupied, 0 = empty. */
} fruit_record_t;

/**
 * @brief Result codes returned by the fruit database module.
 */
typedef enum {
    FRUIT_DB_OK = 0,
    FRUIT_DB_ERR_FULL,
    FRUIT_DB_ERR_INVALID_INDEX,
    FRUIT_DB_ERR_NO_MATCH
} fruit_db_status_t;

/* --- Public API prototypes --- */

/**
 * @brief  Initializes the database and loads the default factory fruit records.
 * @return Operation status.
 */
fruit_db_status_t fruit_db_init(void);

/**
 * @brief  Adds a newly calibrated fruit to the database.
 * @param  name  Fruit name to store.
 * @param  r_raw Raw or normalized red value from the averaged sensor reading.
 * @param  g_raw Raw or normalized green value from the averaged sensor reading.
 * @param  b_raw Raw or normalized blue value from the averaged sensor reading.
 * @return Operation status.
 */
fruit_db_status_t fruit_db_add(const char *name, float r_raw, float g_raw, float b_raw);

/**
 * @brief  Compares a sensor reading against a database reference to identify a fruit.
 * @param  r_raw      Current red sensor reading.
 * @param  g_raw      Current green sensor reading.
 * @param  b_raw      Current blue sensor reading.
 * @param  target_idx Reference fruit index in the database.
 * @return Operation status.
 */
fruit_db_status_t fruit_db_compare(float r_raw, float g_raw, float b_raw, uint8_t target_idx);

/**
 * @brief  Returns the current number of valid fruit records in the database.
 * @return Number of valid records from 0 to DB_MAX_FRUITS.
 */
uint8_t fruit_db_get_count(void);

/**
 * @brief  Deletes a fruit by index and shifts the array to avoid empty gaps.
 * @param  index Fruit index to delete.
 * @return Operation status.
 */
fruit_db_status_t fruit_db_delete(uint8_t index);

/**
 * @brief  Returns a read-only pointer to the database table for menu display.
 * @return Pointer to the internal fruit record table.
 */
const fruit_record_t* fruit_db_get_table(void);

#endif /* FRUIT_DB_H */

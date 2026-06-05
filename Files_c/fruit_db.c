/**
 * @file    fruit_db.c
 * @brief   Module for fruit data storage and classification.
 */

#include "fruit_db.h"

/* --- Static chromatic reference database --- */
static fruit_record_t fruit_database[DB_MAX_FRUITS]; /**< Array of fruit records. */
static uint8_t fruit_count = 0;                      /**< Current number of valid fruits. */

/* --- Local helper functions --- */

/**
 * @brief  Copies a string into a bounded destination buffer.
 * @param  dest Destination buffer.
 * @param  src  Source string.
 * @return Number of copied characters, excluding the null terminator.
 */
static uint8_t db_strcpy(char *dest, const char *src)
{
    uint8_t i = 0;

    while ((src[i] != '\0') && (i < (DB_MAX_NAME_LEN - 1U))) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return i;
}

/**
 * @brief  Approximates the square root of a floating-point value.
 * @param  x Input value.
 * @return Approximate square root of x.
 */
static float fast_sqrtf(float x)
{
    if (x <= 0.0f) {
        return 0.0f;
    }

    float y = x;

    for (int i = 0; i < 5; i++) {
        y = (y + (x / y)) * 0.5f;
    }

    return y;
}

fruit_db_status_t fruit_db_init(void)
{
    // Clear the database by marking all slots as empty.
    for (uint8_t i = 0; i < DB_MAX_FRUITS; i++) {
        fruit_database[i].is_valid = 0U;
    }

    fruit_count = 0U;

    // Load default fruit reference values.
    // Previous calibration was performed using the same color sensor.
    db_strcpy(fruit_database[0].name, "Apple");
    fruit_database[0].r_ref = 0.615f;
    fruit_database[0].g_ref = 0.205f;
    fruit_database[0].b_ref = 0.178f;
    fruit_database[0].is_valid = 1U;
    fruit_count++;

    db_strcpy(fruit_database[1].name, "Orange");
    fruit_database[1].r_ref = 0.595f;
    fruit_database[1].g_ref = 0.281f;
    fruit_database[1].b_ref = 0.124f;
    fruit_database[1].is_valid = 1U;
    fruit_count++;

    db_strcpy(fruit_database[2].name, "Banana");
    fruit_database[2].r_ref = 0.508f;
    fruit_database[2].g_ref = 0.345f;
    fruit_database[2].b_ref = 0.147f;
    fruit_database[2].is_valid = 1U;
    fruit_count++;

    db_strcpy(fruit_database[3].name, "Tomato");
    fruit_database[3].r_ref = 0.600f;
    fruit_database[3].g_ref = 0.240f;
    fruit_database[3].b_ref = 0.159f;
    fruit_database[3].is_valid = 1U;
    fruit_count++;

    db_strcpy(fruit_database[4].name, "Grape");
    fruit_database[4].r_ref = 0.422f;
    fruit_database[4].g_ref = 0.324f;
    fruit_database[4].b_ref = 0.254f;
    fruit_database[4].is_valid = 1U;
    fruit_count++;

    return FRUIT_DB_OK;
}

fruit_db_status_t fruit_db_add(const char *name, float r_raw, float g_raw, float b_raw)
{
    // Search for the first empty slot.
    for (uint8_t i = 0; i < DB_MAX_FRUITS; i++) {
        if (fruit_database[i].is_valid == 0U) {

            // Calculate relative proportions to isolate chrominance.
            float sum = r_raw + g_raw + b_raw;

            if (sum == 0.0f) {
                sum = 1.0f; // Avoid division by zero.
            }

            db_strcpy(fruit_database[i].name, name);
            fruit_database[i].r_ref = r_raw / sum;
            fruit_database[i].g_ref = g_raw / sum;
            fruit_database[i].b_ref = b_raw / sum;
            fruit_database[i].is_valid = 1U;
            fruit_count++;

            return FRUIT_DB_OK;
        }
    }

    return FRUIT_DB_ERR_FULL;
}

fruit_db_status_t fruit_db_delete(uint8_t index)
{
    if (index >= fruit_count) {
        return FRUIT_DB_ERR_INVALID_INDEX;
    }

    // Shift all elements one position backward.
    for (uint8_t i = index; i < (fruit_count - 1U); i++) {
        // C allows direct struct assignment.
        fruit_database[i] = fruit_database[i + 1U];
    }

    // Clear the last slot, which was duplicated after shifting.
    fruit_database[fruit_count - 1U].is_valid = 0U;

    // Decrease the total number of valid fruit records.
    fruit_count--;

    return FRUIT_DB_OK;
}

fruit_db_status_t fruit_db_compare(float r_raw, float g_raw, float b_raw, uint8_t target_idx)
{
    // Validate the index and fruit availability.
    if ((target_idx >= DB_MAX_FRUITS) || (fruit_database[target_idx].is_valid == 0U)) {
        return FRUIT_DB_ERR_INVALID_INDEX;
    }

    // 1. Get the reference fruit data.
    fruit_record_t fruit = fruit_database[target_idx];

    // 2. Normalize the current reading into chromatic proportions.
    float sum = r_raw + g_raw + b_raw;

    if (sum == 0.0f) {
        sum = 1.0f; // Avoid division by zero in complete darkness.
    }

    float r_measured = r_raw / sum;
    float g_measured = g_raw / sum;
    float b_measured = b_raw / sum;

    // 3. Calculate Euclidean distance between the reading and the reference.
    float dr = r_measured - fruit.r_ref;
    float dg = g_measured - fruit.g_ref;
    float db = b_measured - fruit.b_ref;

    float distance = fast_sqrtf((dr * dr) + (dg * dg) + (db * db));

    // 4. Compare against the tolerance threshold.
    if (distance <= DB_MATCH_TOLERANCE) {
        return FRUIT_DB_OK;
    }

    return FRUIT_DB_ERR_NO_MATCH;
}

const fruit_record_t* fruit_db_get_table(void)
{
    return fruit_database;
}

uint8_t fruit_db_get_count(void)
{
    return fruit_count;
}

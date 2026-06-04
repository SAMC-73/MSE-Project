/**
 * @file    main.c
 * @brief   Sistema de Control RGBC (AEC/AGC) — máquina de estados principal.
 *
 * Flujo por presión de botón:
 *   IDLE → READ_SENSOR → WAIT_RELEASE → IDLE
 *
 * read_sensor_data() llama a rgbc_control_update() una sola vez.
 * El bucle de convergencia (Fase 1 + Fase 2) vive dentro del módulo de
 * control; esta función retorna solo cuando el sistema es estable o falla.
 */

#include "GPIO_stm32.h"
#include "uart_driver.h"
#include "serial.h"
#include "i2c_driver.h"
#include "tcs3472_control.h"
#include "fruit_db.h"
#include "lcd_i2c.h"
#include "lm35_driver.h"
#include "utils.h"


/* --- Prototipos de funciones locales --- */
static void hardware_init(void);
static void delay_ms(uint32_t delay_ms);
static uint8_t leer_boton(void);

/* Menús */
static void main_menu(void);
static void submenu_agregar_fruta(void);
static void submenu_medir_fruta(void);
static uint8_t rutina_nueva_fruta(void);
static uint8_t rutina_eliminar_fruta(void);

/* --- Definiciones de Botones --- */
#define BTN_NAV_PORT    GPIO_PORT_A
#define BTN_NAV_PIN     GPIO_PIN_10   // B1: Navegar / Siguiente
#define BTN_OK_PORT     GPIO_PORT_B
#define BTN_OK_PIN      GPIO_PIN_5    // B2: Seleccionar / Confirmar
#define BTN_BACK_PORT   GPIO_PORT_B
#define BTN_BACK_PIN    GPIO_PIN_3    // B3: Cancelar / Regresar

/* --- Máquina de estados para botones --- */
typedef enum {
    WAIT_PRESS,
    WAIT_RELEASE
} btn_state_t;

static btn_state_t btn_estado = WAIT_PRESS;


/*  -- Structs de control de periféricos --  */
static lcd_handle_t     lcd;
static LM35_CalData_t   t_ambiente; 
static rgbc_control_t   ctrl_rgbc;

int main(void)
{
    hardware_init();    

    while (1) {
        main_menu();
    }
    return 0;
}

/**
 * @brief Lee los botones usando una máquina de estados global.
 * Si un botón quedó presionado en la llamada anterior, espera
 * en WAIT_RELEASE para permitir la detección de la siguiente 
 * presión.
 * @return 1 (B1 - Nav), 2 (B2 - Ok), 3 (B3 - Back)
 */
static uint8_t leer_boton(void)
{
    uint8_t st_b1 = 0, st_b2 = 0, st_b3 = 0;
    uint8_t boton_detectado = 0;

    while (1) {
        gpio_readPin(BTN_NAV_PORT, BTN_NAV_PIN, &st_b1);
        gpio_readPin(BTN_OK_PORT, BTN_OK_PIN, &st_b2);
        gpio_readPin(BTN_BACK_PORT, BTN_BACK_PIN, &st_b3);

        switch (btn_estado) {
            case WAIT_RELEASE:

                // Si ya no hay ningún botón presionado 
                if (!(st_b1 | st_b2 | st_b3)) {

                    delay_ms(50U);              // Pequeño debounce al soltar
                    btn_estado = WAIT_PRESS;    // Liberado, listo para nueva lectura
                
                }
                break;

            case WAIT_PRESS:

                // Si cualquiera es presionado
                if (st_b1 | st_b2 | st_b3) {

                    if (st_b1) boton_detectado = 1;
                    else if (st_b2) boton_detectado = 2;
                    else if (st_b3) boton_detectado = 3;
                    
                    // Retraso solicitado para que los mensajes del sistema sean visibles
                    delay_ms(15U); 
                    
                    // Cambiamos el estado global para la SIGUIENTE vez que se llame a la función
                    btn_estado = WAIT_RELEASE; 
                    return boton_detectado;

                }
                break;
        }
    }
}

/**
 * @brief Menú principal del sistema.
 */
static void main_menu(void)
{
    uint8_t cursor = 0; // 0: Agregar Fruta, 1: Medir Fruta
    
    while (1) {
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "-- MENU PRINCIPAL --");
        
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, cursor == 0 ? "> 1. Agregar Fruta" : "  1. Agregar Fruta");
        
        lcd_set_cursor(&lcd, 0, 2);
        lcd_print(&lcd, cursor == 1 ? "> 2. Medir Fruta" : "  2. Medir Fruta");

        uint8_t btn = leer_boton();
        
        if (btn == 1) {        
            cursor = (cursor + 1) % 2;
        } else if (btn == 2) { 
            if (cursor == 0) {
                submenu_agregar_fruta();
            } else {
                submenu_medir_fruta();
            }
        }
    }
}

/**
 * @brief Submenú para calibración y gestión de frutas.
 */
static void submenu_agregar_fruta(void)
{
    uint8_t cursor = 0; // 0: Nueva, 1: Eliminar, 2: Regresar
    
    while (1) {
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Menu Calibracion:");
        
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, cursor == 0 ? "> Nueva fruta" : "  Nueva fruta");
        
        lcd_set_cursor(&lcd, 0, 2);
        lcd_print(&lcd, cursor == 1 ? "> Eliminar fruta" : "  Eliminar fruta");
        
        lcd_set_cursor(&lcd, 0, 3);
        lcd_print(&lcd, cursor == 2 ? "> Regresar" : "  Regresar");

        uint8_t btn = leer_boton();
        
        if (btn == 1) { 
            cursor = (cursor + 1) % 3;
        } else if (btn == 2) { 
            uint8_t salir_a_main = 0;
            
            if (cursor == 0) {
                salir_a_main = rutina_nueva_fruta();
            } else if (cursor == 1) {
                salir_a_main = rutina_eliminar_fruta();
            } else if (cursor == 2) {
                return; 
            }
            
            if (salir_a_main) return;
            
        } else if (btn == 3) { 
            return;
        }
    }
}


/**
 * @brief Submenú para seleccionar una fruta de referencia y medirla.
 */
static void submenu_medir_fruta(void)
{
    uint8_t count = fruit_db_get_count();

    // Validación inicial: verificar que existan referencias en la base
    if (count == 0) {
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Error: No hay frutas");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, "Agregue una primero");
        delay_ms(2000U);
        return;
    }

    const fruta_registro_t *db = fruit_db_get_table();
    uint8_t idx = 0;

    while (1) {
        // --- Pantalla de selección de fruta ---
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Selec. Referencia:");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, db[idx].nombre);
        lcd_set_cursor(&lcd, 0, 3);
        lcd_print(&lcd, "B1:>> B2:Medir B3:Ret");

        uint8_t btn = leer_boton();

        if (btn == 1) {                     // Navegar
            idx = (idx + 1) % count;
        }
        else if (btn == 2) {                // Iniciar medición
            // --- Pantalla de preparación ---
            lcd_clear(&lcd);
            lcd_set_cursor(&lcd, 0, 0);
            lcd_print(&lcd, "Midiendo...");
            lcd_set_cursor(&lcd, 0, 1);
            lcd_print(&lcd, db[idx].nombre);
            lcd_set_cursor(&lcd, 0, 2);
            lcd_print(&lcd, "Ajustando sensor");
            lcd_set_cursor(&lcd, 0, 3);
            lcd_print(&lcd, "B2:Iniciar B3:Can");

            uint8_t confirm;
            do {
                confirm = leer_boton();
            } while (confirm != 2 && confirm != 3);

            if (confirm == 3) {
                continue;   // Cancelar, vuelve a la selección
            }

            // --- Ejecutar el ciclo de control AEC/AGC (ajuste automático) ---
            lcd_set_cursor(&lcd, 0, 2);
            lcd_print(&lcd, "Estabilizando... ");
            control_status_t ctrl_st = rgbc_control_update(&ctrl_rgbc);

            if (ctrl_st != CONTROL_OK) {
                lcd_clear(&lcd);
                lcd_set_cursor(&lcd, 0, 0);
                lcd_print(&lcd, "Error sensor");
                lcd_set_cursor(&lcd, 0, 1);
                lcd_print(&lcd, (ctrl_st == CONTROL_UNDEREXPOSED) ?
                          "Luz insuficiente" : "Fallo I2C");
                delay_ms(2000U);
                continue;   // Vuelve a la selección
            }

            // --- Obtener los valores crudos R, G, B ---
            float r_raw, g_raw, b_raw;
            rgbc_get_raw_data(&ctrl_rgbc, &r_raw, &g_raw, &b_raw);

            // --- Comparar con la fruta seleccionada ---
            fruit_db_status_t cmp = fruit_db_compare(r_raw, g_raw, b_raw, idx);

            // --- Mostrar veredicto ---
            lcd_clear(&lcd);
            lcd_set_cursor(&lcd, 0, 0);
            lcd_print(&lcd, "Veredicto:");
            lcd_set_cursor(&lcd, 0, 1);
            lcd_print(&lcd, db[idx].nombre);

            lcd_set_cursor(&lcd, 0, 2);
            if (cmp == FRUIT_DB_OK) {
                lcd_print(&lcd, "-> Madura");
            } else {
                lcd_print(&lcd, "-> No segura");
            }

            lcd_set_cursor(&lcd, 0, 3);
            lcd_print(&lcd, "B1: Continuar");

            // Esperar a que el usuario presione B1 específicamente
            while (leer_boton() != 1) {
                // Espera activa ignorando pulsaciones de B2 o B3
            }
            // Al salir, vuelve a la pantalla de selección de fruta
        }
        else if (btn == 3) {                // Regresar al menú principal
            return;
        }
    }
}


/**
 * @brief Lógica para agregar 5 mediciones de una nueva fruta.
 */
static uint8_t rutina_nueva_fruta(void)
{
    float acc_r = 0.0f;
    float acc_g = 0.0f;
    float acc_b = 0.0f;

    lcd_clear(&lcd);
    lcd_set_cursor(&lcd, 0, 0);
    lcd_print(&lcd, "Iniciando calibrado");
    delay_ms(1000U);

    for (uint8_t i = 1U; i <= 5U; i++) {

        /* --- Pantalla de espera por muestra i --- */
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Medida ");
        lcd_put_char(&lcd, '0' + i);
        lcd_print(&lcd, " de 5");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, "Coloca la fruta...");
        lcd_set_cursor(&lcd, 0, 3);
        lcd_print(&lcd, "B2: Medir B3: Cancel");

        uint8_t btn = leer_boton();

        if (btn == 3) {
            lcd_clear(&lcd);
            lcd_set_cursor(&lcd, 0, 0);
            lcd_print(&lcd, "Operacion cancelada");
            delay_ms(1500U);
            return 1;
        }

        /* --- Ejecutar ciclo de control AEC/AGC --- */
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Midiendo...");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_put_char(&lcd, '0' + i);
        lcd_print(&lcd, "/5");

        control_status_t st = rgbc_control_update(&ctrl_rgbc);

        if (st != CONTROL_OK) {
            lcd_clear(&lcd);
            lcd_set_cursor(&lcd, 0, 0);
            lcd_print(&lcd, "Error de lectura");
            lcd_set_cursor(&lcd, 0, 1);
            lcd_print(&lcd, st == CONTROL_UNDEREXPOSED
                            ? "Luz insuficiente"
                            : "Fallo I2C");
            delay_ms(2000U);
            return 1;
        }

        /* --- Acumular valores normalizados (ganancia × ciclos) --- */
        float r, g, b;
        rgbc_get_raw_data(&ctrl_rgbc, &r, &g, &b);
        acc_r += r;
        acc_g += g;
        acc_b += b;

        lcd_set_cursor(&lcd, 0, 2);
        lcd_print(&lcd, "Lectura OK");
        delay_ms(100U);
    }

    /* --- Promediar y guardar --- */
    lcd_clear(&lcd);
    lcd_set_cursor(&lcd, 0, 0);
    lcd_print(&lcd, "Promediando...");

    float avg_r = acc_r / 5.0f;
    float avg_g = acc_g / 5.0f;
    float avg_b = acc_b / 5.0f;

    /* Nombre automático: "Fruta N" con N = posición que ocupará */
    char nombre[DB_MAX_NAME_LEN];
    utils_snprintf(nombre, "Fruta %u", (unsigned int)(fruit_db_get_count() + 1U));

    fruit_db_status_t db_st = fruit_db_add(nombre, avg_r, avg_g, avg_b);

    lcd_clear(&lcd);
    lcd_set_cursor(&lcd, 0, 0);
    if (db_st == FRUIT_DB_OK) {
        lcd_print(&lcd, "Fruta guardada!");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, nombre);
    } else {
        lcd_print(&lcd, "Error: DB llena");
    }
    delay_ms(1500U);

    return 1;
}

/**
 * @brief Navega el arreglo para eliminar una fruta.
 */
static uint8_t rutina_eliminar_fruta(void)
{

    // Si ya no hay frutas, no se muestra el menú de eliminación
    if (fruit_db_get_count() == 0) {
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Error: No hay frutas");
        delay_ms(800U);
        return 1;
    }

    uint8_t idx = 0;

    while (1) {
        uint8_t count              = fruit_db_get_count();
        const fruta_registro_t *db = fruit_db_get_table();

        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_print(&lcd, "Eliminar:");
        lcd_set_cursor(&lcd, 0, 1);
        lcd_print(&lcd, db[idx].nombre);
        lcd_set_cursor(&lcd, 0, 3);
        lcd_print(&lcd, "B1:>> B2:Ok B3:Atras");

        uint8_t btn = leer_boton();

        // Si se presiona B1, se avanza al siguiente índice 
        // y regresa al inicio de la lista si llega al final
        if (btn == 1) {
            idx = (idx + 1) % count;
        } 
        
        // Si se presiona B2, se muestra una pantalla de 
        // confirmación de eliminación antes de continuar
        else if (btn == 2) {

            /* Pantalla de confirmación — refresca db por si cambió */
            db = fruit_db_get_table();
            lcd_clear(&lcd);
            lcd_print(&lcd, "Seguro borrar?");
            lcd_set_cursor(&lcd, 0, 1);
            lcd_print(&lcd, db[idx].nombre);
            lcd_set_cursor(&lcd, 0, 3);
            lcd_print(&lcd, "B2: Si   B3: No");

            /* Bucle de protección: Solo sale con B2 o B3 */
            while (1) {

                uint8_t conf = leer_boton();
                
                // Se confirma eliminación
                if (conf == 2) { 
                    fruit_db_delete(idx);
                    lcd_clear(&lcd);
                    lcd_print(&lcd, "Eliminado exitoso");
                    delay_ms(1500U);
                    return 1; // Sale al menú superior
                } 

                // Se cancela eliminación
                else if (conf == 3) { // No, cancelar
                    break; // Rompe el bucle interno y vuelve a dibujar el menú "Eliminar:"
                }

                // Si presiona B1 (1) u otra cosa, simplemente no hace nada y sigue esperando.
            }
        } 
        
        // Si se presiona B3, se regresa al menú anterior sin 
        // eliminar nada
        else if (btn == 3) {
            return 0;
        }
    }
}


/**
 * @brief Configuración de pines y periféricos necesarios.
 */
static void hardware_init(void)
{
    uint32_t *SCB_CPACR = (uint32_t *)0xE000ED88;
    *SCB_CPACR |= (0xF << 20);   /* Habilitar FPU (CP10 y CP11) */

    /* --- UART2 (Serial) — PA2 = TX, AF7 --- */
    gpio_initPort(GPIO_PORT_A);
    gpio_initPort(GPIO_PORT_B);
    gpio_setPinMode(GPIO_PORT_A, GPIO_PIN_2, GPIO_MODE_ALT_FUNC);
    gpio_setAlternateFunction(GPIO_PORT_A, GPIO_PIN_2, 7);
    serial_init();

    /* --- USER BUTTONS --- */
    gpio_setPinMode(GPIO_PORT_A, GPIO_PIN_10, GPIO_MODE_INPUT);
    gpio_setPinMode(GPIO_PORT_B, GPIO_PIN_5, GPIO_MODE_INPUT);
    gpio_setPinMode(GPIO_PORT_B, GPIO_PIN_3, GPIO_MODE_INPUT);
    gpio_setPullDown(GPIO_PORT_A, GPIO_PIN_10);
    gpio_setPullDown(GPIO_PORT_B, GPIO_PIN_5);
    gpio_setPullDown(GPIO_PORT_B, GPIO_PIN_3);

    /* --- I2C1 (PB8 = SCL, PB9 = SDA), AF4, Open-Drain + Pull-Up --- */
    gpio_setPinMode(GPIO_PORT_B, GPIO_PIN_8, GPIO_MODE_ALT_FUNC);
    gpio_setPinMode(GPIO_PORT_B, GPIO_PIN_9, GPIO_MODE_ALT_FUNC);
    gpio_setAlternateFunction(GPIO_PORT_B, GPIO_PIN_8, 4);
    gpio_setAlternateFunction(GPIO_PORT_B, GPIO_PIN_9, 4);
    gpio_setOpenDrain(GPIO_PORT_B, GPIO_PIN_8);
    gpio_setOpenDrain(GPIO_PORT_B, GPIO_PIN_9);
    gpio_setPullUp(GPIO_PORT_B, GPIO_PIN_8);
    gpio_setPullUp(GPIO_PORT_B, GPIO_PIN_9);

    /* --- Configuración de I2C1 --- */
    i2c_config_t i2c_cfg = {
        .instance  = I2C1,
        .clk_speed = I2C_SPEED_SM_HZ,
        .addr_mode = I2C_ADDR_7BIT,
    };

    // Periférico I2C inicializado
    i2c_init(&i2c_cfg);

    // Inicialización de la base de datos de frutas
    fruit_db_init();

    // Inicialización del LCD con la instancia de 
    // I2C configurada
    lcd_init(&lcd, I2C1, LCD_PCF8574_ADDR);

    /* Mensaje de inicio de sistema */
    lcd_clear(&lcd);

    lcd_set_cursor(&lcd, 0, 0);
    lcd_print(&lcd, "Iniciando sistema...");

    delay_ms(150U);
    
    /* Inicialización del controlador RGBC */
    control_status_t ctrl_st = rgbc_control_init(&ctrl_rgbc, I2C1);

    // Mensaje de estado del sensor RGBC
    lcd_set_cursor(&lcd, 0, 1);

    if (ctrl_st == CONTROL_OK) 
         {lcd_print(&lcd, "Sensor RGBC listo!");} 
    
    else {lcd_print(&lcd, "Error sensor RGBC");}

    delay_ms(250U);

    /*  */
    lcd_set_cursor(&lcd, 0, 1);
    lcd_print(&lcd, "Sensor RBGC listo!");

    /* ADC Configuration */

    lm35_init(ADC_INSTANCE_1);
    lm35_calibrate(ADC_INSTANCE_1, 250, &t_ambiente);

    delay_ms(250U);
    lcd_set_cursor(&lcd, 0, 2);
    lcd_print(&lcd, "Sensor LM35 listo!");

    delay_ms(250U);

    lcd_set_cursor(&lcd, 0, 2);
    lcd_print(&lcd, "Database listo!");

    delay_ms(200U);

}

static void delay_ms(uint32_t delay_ms)
{
    /* Aproximación burda: cada iteración dura ~4 ciclos (leer, comparar, saltar) */
    for (uint32_t i = 0; i < delay_ms * 4000U; i++) {
        __asm volatile ("nop");
    }
}

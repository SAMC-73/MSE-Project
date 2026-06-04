#include "serial.h"
#include "utils.h"
#include "uart_driver.h"


void serial_init(void){

    uart_init();

}

void serial_printf(const char *format, ...){

    char buffer[SERIAL_TX_BUFFER_SIZE];
    va_list args;
    va_start(args, format);

    utils_vsnprintf(buffer, format, args);

    for (size_t i = 0; buffer[i] != '\0'; i++) {
        uart_write((uint8_t)buffer[i]);
    }

    va_end(args);
}
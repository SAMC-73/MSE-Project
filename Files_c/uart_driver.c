#include "uart_driver.h"


uart_status_t uart_init(void){

    /* Habilitar el reloj para USART2 */
    RCC_APB1ENR |= (1U << RCC_APB1ENR_USART2EN_POS);

    /* Configurar el Baud Rate*/
    USART2->BRR = UART_BRR_VALUE;

    /* Habilitar transmisor y periferico */
    USART2->CR1 |= (1U << USART_CR1_TE_POS);
    USART2->CR1 |= (1U << USART_CR1_UE_POS);

    /* Función de inicialización exitosa */
    return UART_OK;
}


uart_status_t uart_write(uint8_t data){

    /* Tiempo arbitrario de espera */
    uint32_t timeout = 1000000; 

    /* Se monitorea el bit TXE del registro SR como indicador
       para esperar a que el transmisor se haya vaciado y esté listo
       para poder ser modificado                                         */
    while (!(USART2->SR & (1U << USART_SR_TXE_POS))) { 
        if (--timeout == 0) {
            return UART_ERR_TIMEOUT; // Timeout alcanzado
        }
    }

    /* Escribir el dato en el registro de datos */
    USART2->DR = data;

    return UART_OK;

}
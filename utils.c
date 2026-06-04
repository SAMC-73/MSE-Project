/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Carlos Villarreal and CETYS Universidad are not liable for any
 * misuse of this material.
 *
 *****************************************************************************/
/**
 * @file utils.c
 * @brief Utility library with helper functions.
 *
 * Utils module has helper functions to treat strings, ASCII conversions, and
 * printing utilities.
 *
 * @author Leonardo Ivan García Espinosa
 * @date 04/30/2026
 *
 */

/*** Includes ***/
#include "utils.h"

/*** Preprocessor Definitions ***/
#define ASCII_SIGN          45u
#define ASCII_NUM_OFFSET    '0'
#define ASCII_CHAR_OFFSET   'A'

/*** Type Prototypes ***/

/*** Local Variables ***/

/*** External Variables ***/

/*** Function Prototypes ***/

static uint32_t utils_printString(char *dst, char *src);
static uint32_t utils_printInt(char *dst, int32_t num, uint8_t sign, uint8_t base);



/*** Function Definitions ***/

/**
 * @brief Format and store a string into a buffer
 *
 * This function builds a formatted string based on a format specifier string
 * and a variable list of arguments, similar to the standard snprintf.
 * The resulting string is written into the provided destination buffer.
 *
 * @param dst Pointer to the destination buffer where the formatted string will be stored.
 * @param format Format string containing text and conversion specifiers (e.g., %d, %s, %x).
 * @param ... Variable arguments corresponding to the format specifiers.
 *
 * @return None.
 */
void utils_snprintf(char *dst, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format == '%')
        {
            format++;

            switch (*format)
            {
                case 's': {
                    char *str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    dst += utils_printString(dst, str);
                    break;
                }

                case 'd': {
                    int32_t num = va_arg(args, int);
                    dst += utils_printInt(dst, num, SIGNED, DECIMAL);
                    break;
                }

                case 'u': {
                    int32_t num = (int32_t)va_arg(args, unsigned int);
                    dst += utils_printInt(dst, num, UNSIGNED, DECIMAL);
                    break;
                }

                case 'x': {
                    int32_t num = (int32_t)va_arg(args, unsigned int);
                    dst += utils_printInt(dst, num, UNSIGNED, HEX);
                    break;
                }

                case 'c':
                    *dst++ = (char)va_arg(args, int);
                    break;

                case '%':
                    *dst++ = '%';
                    break;

                default:
                    *dst++ = *format;
                    break;
            }
        }
        else
        {
            *dst++ = *format;
        }

        format++;
    }

    *dst = '\0';
    va_end(args);
}



void utils_vsnprintf(char *dst, const char *format, va_list args){

    while (*format)
    {
        if (*format == '%')
        {
            format++;

            switch (*format)
            {
                case 's': {
                    char *str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    dst += utils_printString(dst, str);
                    break;
                }

                case 'd': {
                    int32_t num = va_arg(args, int);
                    dst += utils_printInt(dst, num, SIGNED, DECIMAL);
                    break;
                }

                case 'u': {
                    int32_t num = (int32_t)va_arg(args, unsigned int);
                    dst += utils_printInt(dst, num, UNSIGNED, DECIMAL);
                    break;
                }

                case 'x': {
                    int32_t num = (int32_t)va_arg(args, unsigned int);
                    dst += utils_printInt(dst, num, UNSIGNED, HEX);
                    break;
                }

                case 'c':
                    *dst++ = (char)va_arg(args, int);
                    break;

                case '%':
                    *dst++ = '%';
                    break;

                default:
                    *dst++ = *format;
                    break;
            }
        }
        else
        {
            *dst++ = *format;
        }

        format++;
    }

    *dst = '\0';

}
/**
 * @brief Convert data from integer type into an ASCII string
 *
 * Given an integer value, this will convert a provided integer to
 * an ASCII string data type regardless of the integer base (2-16),
 * and return the number of digits of the converted ASCII string.
 * The numerical system of the integer is determined by the provided
 * base value.
 *
 * @param data Integer value to convert to
 * @param ptr Pointer to the ASCII string
 * @param sign Interger value that indicates if data is signed or unsigned
 * @param base Base of the integer to convert to
 *
 * @return Length of the converted data.
 */
uint32_t utils_itoa(int32_t data, uint8_t *ptr, uint8_t sign, uint8_t base){

    uint8_t n_base;
    uint32_t curr_num;
    uint32_t len_str = 0u;
    uint32_t digit;

    switch(base){
        case BINARY:    n_base = 2u;    break;
        case OCTAL:     n_base = 8u;    break;
        case DECIMAL:   n_base = 10u;   break;
        case HEX:       n_base = 16u;   break;
        default:        return 0;
    }

    if(sign == SIGNED && data < 0){
        *(ptr + len_str) = '-';
        len_str++;
        curr_num = (uint32_t)(-(int64_t)data);
    } else {
        curr_num = (uint32_t)data;
    }

    do {
        digit = curr_num % n_base;
        curr_num /= n_base;

        if(digit < 10){
            *(ptr + len_str) = ASCII_NUM_OFFSET + digit;
        } else {
            *(ptr + len_str) = ASCII_CHAR_OFFSET + digit - 10;
        }

        len_str++;
    } while (curr_num != 0);

    uint32_t start = (sign == SIGNED && data < 0) ? 1u : 0u;
    uint32_t end = len_str - 1u;
    uint8_t temp;

    for(uint32_t i = start; i <= (start + end)/2u; i++){
        temp = *(ptr + i);
        *(ptr + i) = *(ptr + (end - (i - start)));
        *(ptr + (end - (i - start))) = temp;
    }

    *(ptr + len_str) = '\0';

    return len_str;
}


/**
 * @brief Convert data from an ASCII string into an integer type
 *
 * Given an unsigned integer pointer, this will convert a pointer to
 * a character string to an integer data type regardless of the integer
 * base (2-16), and return the converted integer data.
 * The numerical system of the integer is determined by the provided
 * base value and the number of characters is determined by the provided
 * digits value.
 *
 * @param ptr Pointer to the ASCII string
 * @param digits Number of digits in the ASCII string
 * @param sign Interger value that indicates if data is signed or unsigned
 * @param base Base of the converted integer
 *
 * @return Converted integer value.
 */
int32_t utils_atoi(uint8_t *ptr, uint32_t digits, uint8_t sign, uint8_t base){

    int32_t num = 0;
    uint32_t i = 0;
    uint8_t n_base;
    int8_t sign_flag = 1;
    uint32_t digit;

    switch(base){
        case BINARY:    n_base = 2u;    break;
        case OCTAL:     n_base = 8u;    break;
        case DECIMAL:   n_base = 10u;   break;
        case HEX:       n_base = 16u;   break;
        default:        return 0;
    }

    if(sign == SIGNED && *ptr == '-'){
        sign_flag = -1;
        ptr++;  
        digits--;
    }

    for(i = 0; i < digits && *(ptr + i) != '\0'; i++){

        uint8_t c = *(ptr + i);

        if(c >= '0' && c <= '9'){
            digit = c - '0';
        }
        else if(c >= 'A' && c <= 'F'){
            digit = c - 'A' + 10;
        }
        else if(c >= 'a' && c <= 'f'){
            digit = c - 'a' + 10;
        }
        else{
            return 0; 
        }

        if(digit >= n_base){
            return 0; 
        }

        num = num * n_base + digit;
    }

    return num * sign_flag;
}


/**
 * @brief Copy a block of memory from source to destination.
 *
 * This function copies a specified number of bytes from the source buffer
 * to the destination buffer. It assumes that the source and destination
 * buffers do not overlap.
 *
 * @param dst Pointer to the destination buffer where data will be copied.
 * @param src Pointer to the source buffer containing the data to copy.
 * @param length Number of bytes to copy from source to destination.
 *
 * @return Pointer to the destination buffer (dst).
 */
void * utils_memCpy(void *dst, void *src, size_t length){

    /* punteros de tipo byte para poder moverse caracter por caracter */
    uint8_t *d = (uint8_t *)dst;
    uint8_t *s = (uint8_t *)src;
    size_t i;

    /* copia byte por byte desde src hacia dst */
    for(i = 0; i < length; i++)
    {
        *(d + i) = *(s + i);
    }

    return dst;
}

/**
 * @brief Reverse the order of elements of a data set
 *
 * Given a pointer to an unsigned integer data set, this will reverse a number
 * of elements from a provided data set. The number of elements is determined
 * by the provided length parameter.
 *
 * @param src Pointer to source data set
 * @param length Number of elements to reverse on the data set
 *
 * @return Pointer to the source (src).
 */
void * utils_memReverse(void *src, size_t length){

    uint8_t *s = (uint8_t *)src;
    uint8_t temp;
    size_t i;

    /* intercambia el primer elemento con el ultimo, luego el segundo con el penultimo, etc */
    for(i = 0; i < length / 2; i++)
    {
        temp = *(s + i);
        *(s + i) = *(s + (length - 1 - i));
        *(s + (length - 1 - i)) = temp;
    }

    return src;

}

/**
 * @brief Copy a string into a destination buffer.
 *
 * This function calculates the length of the source string and copies
 * its contents into the destination buffer.
 *
 * @param dst Pointer to the destination buffer where the string will be copied.
 * @param src Pointer to the source string to copy.
 *
 * @return Length of the string copied (number of characters).
 */
static uint32_t utils_printString(char *dst, char *src)
{
    uint32_t len = 0;
    while (src[len]) len++;
 
    utils_memCpy(dst, src, len);
 
    return len;
}


/**
 * @brief Convert an integer to ASCII and copy it into a destination buffer.
 *
 * This function converts an integer into its ASCII representation
 * based on the specified base (2-16) and sign option.
 * The resulting string is copied into the destination buffer.
 *
 * @param dst Pointer to the destination buffer where the ASCII string will be stored.
 * @param num Integer number to convert.
 * @param sign Interger value that indicates if data is signed or unsigned.
 * @param base Numerical base for conversion.
 *
 * @return Length of the ASCII string copied into the destination buffer.
 */
static uint32_t utils_printInt(char *dst, int32_t num, uint8_t sign, uint8_t base)
{
    uint8_t buf[12];
 
    uint32_t len = utils_itoa(num, buf, sign, base);
 
    utils_memCpy(dst, buf, len);
 
    return len;
}

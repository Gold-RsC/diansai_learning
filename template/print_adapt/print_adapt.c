#include "print_adapt.h"

uint8_t uart_out_buffer[UART_OUT_BUFFER_SIZE];

#define __PRINT_ADAPT_FORMAT_STR__()                                                                                   \
    do {                                                                                                               \
        va_list args;                                                                                                  \
                                                                                                                       \
        va_start(args, format);                                                                                        \
        length += vsnprintf((char*)uart_out_buffer, UART_OUT_BUFFER_SIZE, format, args);                               \
        va_end(args);                                                                                                  \
                                                                                                                       \
        if (length >= UART_OUT_BUFFER_SIZE) {                                                                          \
            length = UART_OUT_BUFFER_SIZE - 1;                                                                         \
        }                                                                                                              \
    } while (0)
size_t printf_NORMAL(const char* format, ...) {
    size_t length = 0;
    __PRINT_ADAPT_FORMAT_STR__();


    HAL_UART_Transmit(UART_ADDR, uart_out_buffer, length, 0xFFFFFFFF);

    return length;
}

size_t printf_DMA(const char* format, ...) {
    size_t length = 0;
    __PRINT_ADAPT_FORMAT_STR__();

    HAL_UART_Transmit_DMA(UART_ADDR, uart_out_buffer, length);

    return length;
}

size_t printf_IT(const char* format, ...) {
    size_t length = 0;
    __PRINT_ADAPT_FORMAT_STR__();


    HAL_UART_Transmit_IT(UART_ADDR, uart_out_buffer, length);

    return length;
}

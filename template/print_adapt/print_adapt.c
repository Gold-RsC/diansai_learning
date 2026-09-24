#include "print_adapt.h"

int printf_NORMAL(const char* format, ...) {
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf((char*)uart_out_buffer, UART_OUT_BUFFER_SIZE, format, args);
    va_end(args);

    if (length >= UART_OUT_BUFFER_SIZE) {
        length = UART_OUT_BUFFER_SIZE - 1;
    }


    HAL_UART_Transmit(UART_ADDR, uart_out_buffer, length, 0xFFFFFFFF);

    return length;
}

int printf_DMA(const char* format, ...) {
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf((char*)uart_out_buffer, UART_OUT_BUFFER_SIZE, format, args);
    va_end(args);

    if (length >= UART_OUT_BUFFER_SIZE) {
        length = UART_OUT_BUFFER_SIZE - 1;
    }


    HAL_UART_Transmit_DMA(UART_ADDR, uart_out_buffer, length);

    return length;
}

int printf_IT(const char* format, ...) {
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf((char*)uart_out_buffer, UART_OUT_BUFFER_SIZE, format, args);
    va_end(args);

    if (length >= UART_OUT_BUFFER_SIZE) {
        length = UART_OUT_BUFFER_SIZE - 1;
    }


    HAL_UART_Transmit_IT(UART_ADDR, uart_out_buffer, length);

    return length;
}

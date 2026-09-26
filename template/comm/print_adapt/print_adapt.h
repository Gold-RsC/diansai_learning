#ifndef __PRINT_ADAPT_H__
#define __PRINT_ADAPT_H__

#include "mbase.h"
#include "stdarg.h"

/**
 * @brief UART地址
 */
#define UART_ADDR (&huart4)


/**
 * @brief UART输出缓冲区
 */
#define UART_OUT_BUFFER_SIZE 256
extern uint8_t uart_out_buffer[UART_OUT_BUFFER_SIZE];


/**
 * @brief 阻塞式打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return size_t 打印的字符节数
 */
size_t Printf_Normal(const char* format, ...);
/**
 * @brief DMA打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return size_t 打印的字符节数
 */
size_t Printf_DMA(const char* format, ...);
/**
 * @brief 中断式打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return size_t 打印的字符节数
 */
size_t Printf_IT(const char* format, ...);

#endif

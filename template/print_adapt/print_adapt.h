#ifndef __PRINT_ADAPT_H__
#define __PRINT_ADAPT_H__

#include "mbase.h"
#include "stdarg.h"

/**
 * @brief UART地址
 */
#define UART_ADDR (&huart4)

/**
 * @brief UART输出缓冲区大小
 */
#define UART_OUT_BUFFER_SIZE 256

/**
 * @brief 阻塞式打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return int 打印的字符节数
 */
int printf_NORMAL(const char* format, ...);
/**
 * @brief DMA打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return int 打印的字符节数
 */
int printf_DMA(const char* format, ...);
/**
 * @brief 中断式打印
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return int 打印的字符节数
 */
int printf_IT(const char* format, ...);

#endif

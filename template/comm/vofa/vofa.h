#ifndef __VOFA_H__
#define __VOFA_H__


#include "print_adapt.h"

#define Vofa_Printf(format, ...) (Printf_Normal(format, ##__VA_ARGS__))


/**
 * @brief FireWater协议
 * @param format 格式化字符串
 * @param ... 可变参数
 */
#define Vofa_FireWater(format, ...) (Vofa_Printf(format, ##__VA_ARGS__))

/**
 * @brief JustFloat协议
 * @param _data 数据指针
 * @param _num 数据数量
 * @return size_t 数据数量
 */
size_t Vofa_JustFloat(float* _data, size_t _num);

#endif

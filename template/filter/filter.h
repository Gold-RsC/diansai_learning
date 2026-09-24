#ifndef __FILTER_H__
#define __FILTER_H__

#include "kalman.h"
#include "harmonic.h"

typedef struct {
    struct {
        Kalman_Filter_t kalman_filter;
        Harmonic_Filter_t harmonic_filter;
    } _state;

    Float_t out;
} Filter_t;

/**
 * @brief 初始化过滤器
 *
 * @param filter 过滤器结构体指针
 * @param Q 系统噪声方差
 * @param R 测量噪声方差
 * @param gain_2 调谐系数 2
 * @param gain_3 调谐系数 3
 * @param gain_5 调谐系数 5
 * @param min 输出下限
 * @param max 输出上限
 * @param initial_value 初始值
 */
void Filter_Init(Filter_t* filter,
                 Float_t Q,
                 Float_t R,
                 Float_t gain_2,
                 Float_t gain_3,
                 Float_t gain_5,
                 Float_t min,
                 Float_t max,
                 Float_t initial_value);

/**
 * @brief 更新过滤器状态
 *
 * @param filter 过滤器结构体指针
 * @param measurement 测量值
 * @return Float_t 过滤后的值
 */
Float_t Filter_Update(Filter_t* filter, Float_t measurement);


#endif

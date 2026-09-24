#ifndef __HARMONIC_FILTER_H__
#define __HARMONIC_FILTER_H__

#include "mbase.h"
#include "sliding.h"

#define HARMONIC_FILTER_WINDOW_SIZE 5

typedef struct {

    struct {
        Float_t gain_2;
        Float_t gain_3;
        Float_t gain_5;

        Float_t out_min;
        Float_t out_max;
    } param;

    struct {
        Sliding_Filter_t sliding_filter;
    } _state;

    Float_t out;

} Harmonic_Filter_t;

/**
 * @brief 初始化谐波抑制过滤器
 *
 * @param filter 谐波抑制过滤器结构体指针
 * @param gain_2 2次谐波抑制增益
 * @param gain_3 3次谐波抑制增益
 * @param gain_5 5次谐波抑制增益
 * @param out_min 输出最小值
 * @param out_max 输出最大值
 * @param initial_value 初始值
 */
void Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                          Float_t gain_2,
                          Float_t gain_3,
                          Float_t gain_5,
                          Float_t out_min,
                          Float_t out_max,
                          Float_t initial_value);

/**
 * @brief 更新谐波抑制过滤器状态
 *
 * @param filter 谐波抑制过滤器结构体指针
 * @param measurement 测量值
 * @return Float_t 过滤后的值
 */
Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement);

#endif

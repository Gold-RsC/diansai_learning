#ifndef __SMOOTH_FILTER_H__
#define __SMOOTH_FILTER_H__

#include "mbase.h"

#define SLIDING_FILTER_WINDOW_SIZE 5

typedef struct {

    struct {
        Float_t out_min;
        Float_t out_max;
    } param;

    struct {
        Float_t window[SLIDING_FILTER_WINDOW_SIZE];
        size_t idx;
    } _state;

    Float_t out;

} Sliding_Filter_t;

/**
 * @brief 初始化滑动过滤器
 *
 * @param filter 滑动过滤器结构体指针
 * @param out_min 输出下限
 * @param out_max 输出上限
 * @param initial_value 初始值
 */
void Sliding_Filter_Init(Sliding_Filter_t* filter, Float_t out_min, Float_t out_max, Float_t initial_value);

/**
 * @brief 更新滑动过滤器状态
 *
 * @param filter 滑动过滤器结构体指针
 * @param measurement 测量值
 * @return Float_t 过滤后的值
 */
Float_t Sliding_Filter_Update(Sliding_Filter_t* filter, Float_t measurement);

#endif

#ifndef __KALMAN_H__
#define __KALMAN_H__

#include "mbase.h"

typedef struct {

    struct {
        Float_t Q;
        Float_t R;

        Float_t out_min;
        Float_t out_max;
    } param;

    struct {
        Float_t P;
    } _state;

    Float_t out;

} Kalman_Filter_t;

/**
 * @brief 初始化 Kalman 过滤器
 *
 * @param filter Kalman 过滤器 结构体指针
 * @param Q 系统噪声方差
 * @param R 测量噪声方差
 * @param out_min 输出下限
 * @param out_max 输出上限
 * @param initial_value 初始值
 */
void Kalman_Filter_Init(
    Kalman_Filter_t* filter, Float_t Q, Float_t R, Float_t out_min, Float_t out_max, Float_t initial_value);
/**
 * @brief 更新 Kalman 过滤器状态
 *
 * @param filter Kalman 过滤器 结构体指针
 * @param measurement 测量值
 * @return Kalman_Filter_t::Value_t 过滤后的值
 */
Float_t Kalman_Filter_Update(Kalman_Filter_t* filter, Float_t measurement);


#endif

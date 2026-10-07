#ifndef __NOTCH_H__
#define __NOTCH_H__

#include "mbase.h"

/**
 * @brief 二阶 IIR 陷波器
 *
 * @details 传递函数（系数已除以 a0，使用时无需再归一化）：
 *              H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 *          在中心频率处增益严格为 0，在直流与奈奎斯特频率处增益严格为 1。
 *          差分方程：
 *              y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
 */
typedef struct {

    struct {
        Float_t b0;  // 分子系数 b0
        Float_t b1;  // 分子系数 b1
        Float_t b2;  // 分子系数 b2

        Float_t a1;  // 分母系数 a1
        Float_t a2;  // 分母系数 a2
    } param;

    struct {
        Float_t x1;  // 输入历史 x[n-1]
        Float_t x2;  // 输入历史 x[n-2]

        Float_t y1;  // 输出历史 y[n-1]
        Float_t y2;  // 输出历史 y[n-2]
    } _state;

} Notch_Filter_t;

/**
 * @brief 初始化二阶陷波器
 *
 * @param filter 陷波器结构体指针
 * @param freq 陷波中心频率，单位：Hz
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 * @param q 品质因数，Q = 陷波中心频率 / 陷波带宽，建议 20 ~ 50，不要超过 100
 * @param initial_value 初始值
 *
 * @note 系数只与 freq、sample_freq、q 有关，三者任一改变都需重新初始化。
 *       需保证 freq < sample_freq / 2，否则陷波频率超过奈奎斯特频率，系数将无意义。
 */
void Notch_Filter_Init(Notch_Filter_t* filter,
                       Float_t freq,
                       Float_t sample_freq,
                       Float_t q,
                       Float_t initial_value);

/**
 * @brief 更新二阶陷波器状态
 *
 * @param filter 陷波器结构体指针
 * @param input 输入值
 * @return Float_t 陷波后的值
 *
 * @note 需以 sample_freq 指定的周期调用。
 */
Float_t Notch_Filter_Update(Notch_Filter_t* filter, Float_t input);

#endif

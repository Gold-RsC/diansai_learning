#ifndef __LPF_H__
#define __LPF_H__

#include "mbase.h"

/**
 * @brief 一阶 IIR 低通滤波器
 *
 * @details 差分方程：
 *              y[n] = y[n-1] + alpha * (x[n] - y[n-1])
 *          其中
 *              alpha = 1 - e^(-2*pi*fc/fs)
 *          fc 为截止频率，fs 为采样频率。
 *
 *          直流增益严格为 1，-3dB 点位于 fc。一阶意味着阻带以
 *          -20dB/十倍频衰减，需要的抑制越强，fc 就得取得越低。
 *
 * @note 累加器 _state.y1 保持未截断，限幅只作用于对外的 out。
 */
typedef struct {

    struct {
        Float_t sample_freq;  // 采样（ISR 调用）频率，单位：Hz
        Float_t cutoff_freq;  // 截止频率（-3dB 点），单位：Hz

        Float_t out_min;  // 输出下限
        Float_t out_max;  // 输出上限
    } param;

    struct {
        Float_t alpha;  // 滤波系数，只与 sample_freq、cutoff_freq 有关
        Float_t y1;     // 输出历史 y[n-1]
    } _state;

    Float_t out;

} Lpf_Filter_t;

/**
 * @brief 初始化一阶低通滤波器
 *
 * @param lpf 一阶低通滤波器结构体指针
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 * @param cutoff_freq 截止频率（-3dB 点），单位：Hz
 * @param out_min 输出下限
 * @param out_max 输出上限
 * @param initial_value 初始值
 *
 * @note 系数只与 sample_freq、cutoff_freq 有关，二者任一改变都需重新初始化。
 *       需保证 cutoff_freq < sample_freq / 2。
 */
void Lpf_Filter_Init(Lpf_Filter_t* lpf,
                     Float_t sample_freq,
                     Float_t cutoff_freq,
                     Float_t out_min,
                     Float_t out_max,
                     Float_t initial_value);

/**
 * @brief 更新一阶低通滤波器状态
 *
 * @param lpf 一阶低通滤波器结构体指针
 * @param input 输入值
 * @return Float_t 滤波后的值
 *
 * @note 需以 sample_freq 指定的周期调用。
 */
Float_t Lpf_Filter_Update(Lpf_Filter_t* lpf, Float_t input);

#endif

#ifndef __HARMONIC_H__
#define __HARMONIC_H__

#include "mbase.h"

/**
 * @brief 陷波器品质因数
 *
 * @details Q = 陷波中心频率 / 陷波带宽，决定陷波器的尖锐程度。
 *          Q 越大陷波越窄，对基波和相邻次谐波的侵蚀越小，但收敛越慢；
 *          Q 越小陷波越宽，收敛越快，但会开始侵蚀基波。
 *
 *          在 fs = 20kHz、f0 = 50Hz 下实测：
 *              Q = 5  ：基波增益 0.9877、相移 -14.3 度，谐波间互相干扰明显
 *              Q = 30 ：基波增益 0.9996、相移  -2.4 度（本项目取值）
 *              Q = 100：超出 float32 精度，极点过于贴近单位圆，陷波深度反而退化
 *          建议取值范围 20 ~ 50，不要超过 100。
 */
#define HARMONIC_FILTER_Q 30.0f

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

} Harmonic_Notch_t;

/**
 * @brief 谐波抑制滤波器
 *
 * @details 由三个二阶 IIR 陷波器级联构成，分别在 2、3、5 次谐波处陷波。
 *          2 次谐波对应单相逆变器母线电压的 2w 脉动，
 *          3、5 次谐波对应死区效应与非线性负载。
 *
 *          每一级为：
 *              y = x - gain_h * (x - notch_h(x))
 *          其中 gain_h = 0 时该级直通，gain_h = 1 时完全滤除该次谐波。
 *
 *          必须级联而非并联：级联时若某一级把某频率置零，其后各级输入恒为零，
 *          零经任何线性滤波器仍为零，故陷波点严格为 0，实测可达 -50 ~ -75dB；
 *          并联时三个陷波器的残差会互相叠加，实测仅 -25 ~ -40dB。
 *
 *          三个陷波器在基波频率处增益均约为 1、相移接近 0，故基波被基本保留。
 */
typedef struct {

    struct {
        Float_t gain_2;  // 2 次谐波抑制强度，范围 [0, 1]，0 为直通，1 为完全滤除
        Float_t gain_3;  // 3 次谐波抑制强度，范围 [0, 1]，0 为直通，1 为完全滤除
        Float_t gain_5;  // 5 次谐波抑制强度，范围 [0, 1]，0 为直通，1 为完全滤除

        Float_t sample_freq;  // 采样（ISR 调用）频率，单位：Hz
        Float_t grid_freq;    // 基波（电网）频率，单位：Hz

        Float_t out_min;  // 输出下限
        Float_t out_max;  // 输出上限
    } param;

    struct {
        Harmonic_Notch_t notch_2;  // 2 次谐波陷波器
        Harmonic_Notch_t notch_3;  // 3 次谐波陷波器
        Harmonic_Notch_t notch_5;  // 5 次谐波陷波器
    } _state;

    Float_t out;

} Harmonic_Filter_t;

/**
 * @brief 初始化谐波抑制滤波器
 *
 * @param filter 谐波抑制滤波器结构体指针
 * @param gain_2 2 次谐波抑制强度，范围 [0, 1]
 * @param gain_3 3 次谐波抑制强度，范围 [0, 1]
 * @param gain_5 5 次谐波抑制强度，范围 [0, 1]
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz，如 20000.0f
 * @param grid_freq 基波（电网）频率，单位：Hz，如 50.0f
 * @param out_min 输出下限
 * @param out_max 输出上限
 * @param initial_value 初始值
 *
 * @note 初始化时按 grid_freq 自动计算 2、3、5 次陷波器系数，无需手动配置。
 *       陷波器系数只与 sample_freq、grid_freq、HARMONIC_FILTER_Q 有关，
 *       三者任一改变都需重新调用本函数。
 *       需保证 5 * grid_freq < sample_freq / 2，否则陷波频率超过奈奎斯特频率，
 *       系数将无意义。
 */
void Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                          Float_t gain_2,
                          Float_t gain_3,
                          Float_t gain_5,
                          Float_t sample_freq,
                          Float_t grid_freq,
                          Float_t out_min,
                          Float_t out_max,
                          Float_t initial_value);

/**
 * @brief 更新谐波抑制滤波器状态
 *
 * @param filter 谐波抑制滤波器结构体指针
 * @param measurement 测量值
 * @return Float_t 滤波后的值
 *
 * @note 需以 sample_freq 指定的周期调用。本滤波器在基波处增益衰减小于 0.05%、
 *       相移约 -2.4 度，可直接置于控制环反馈通道中使用。
 */
Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement);

#endif

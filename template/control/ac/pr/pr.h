#ifndef __PR_H__
#define __PR_H__

#include "mbase.h"

/**
 * @brief 准比例谐振控制器（QPR）
 *
 * @details 传递函数：
 *              G(s) = kp + 2*kr*wc*s / (s^2 + 2*wc*s + w0^2)
 *          谐振角频率 w0 = 2*pi*f0，wc 决定谐振峰的宽度。
 *          在 w0 处谐振项增益恰为 kr，因此该点总增益为 kp + kr。
 *
 *          与 PI 的区别：PI 对交流量存在稳态幅值误差与相位误差，而 PR 在
 *          谐振频率处有无穷大增益（准谐振为有限大），可以实现交流量的
 *          零稳态误差跟踪。逆变器、并网、AC-AC 变换的电流环应优先用它。
 *
 *          离散化采用 Tustin 变换，系数在 Init 中一次算好：
 *              K  = 2 * sample_freq
 *              a0 = K^2 + 2*wc*K + w0^2
 *              a1 = 2*(w0^2 - K^2) / a0
 *              a2 = (K^2 - 2*wc*K + w0^2) / a0
 *              b0 = 2*kr*wc*K / a0
 *              b2 = -b0
 *          差分方程：
 *              r[n] = b0*e[n] + b2*e[n-2] - a1*r[n-1] - a2*r[n-2]
 *              y[n] = kp*e[n] + r[n]
 *
 * @note 直流增益为 kp，谐振项在直流处为 0 —— 即不放大直流偏置，这是
 *       PR 优于 PI 用在交流环的另一个原因。
 *       需保证 f0 < sample_freq / 2。
 */
typedef struct {

    struct {
        Float_t kp;  // 比例系数
        Float_t kr;  // 谐振系数，决定谐振峰高度

        Float_t w0;  // 谐振角频率 = 2*pi*f0，单位：rad/s
        Float_t wc;  // 谐振带宽角频率，越小谐振峰越尖

        Float_t out_min;  // 输出下限
        Float_t out_max;  // 输出上限
    } param;

    struct {
        Float_t b0;  // 分子系数 b0（已归一化）
        Float_t b2;  // 分子系数 b2（已归一化）

        Float_t a1;  // 分母系数 a1（已归一化）
        Float_t a2;  // 分母系数 a2（已归一化）

        Float_t e1;  // 误差历史 e[n-1]
        Float_t e2;  // 误差历史 e[n-2]

        Float_t r1;  // 谐振输出历史 r[n-1]
        Float_t r2;  // 谐振输出历史 r[n-2]
    } _state;

    Float_t out;

} Pr_t;

/**
 * @brief 初始化准比例谐振控制器
 *
 * @param controller 准比例谐振控制器结构体指针
 * @param kp 比例系数
 * @param kr 谐振系数
 * @param resonant_freq 谐振频率，单位：Hz，通常取基波频率
 * @param bandwidth 谐振带宽，单位：Hz，建议取 5 ~ 20Hz
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 * @param out_min 输出下限
 * @param out_max 输出上限
 *
 * @note 系数只与 resonant_freq、bandwidth、sample_freq、kp、kr 有关，
 *       任一改变都需重新初始化。kp 增大提高整体响应，kr 增大提高谐振点
 *       跟踪精度；带宽过窄时谐振频率稍有偏移就会失效。
 */
void Pr_Init(Pr_t* controller,
             Float_t kp,
             Float_t kr,
             Float_t resonant_freq,
             Float_t bandwidth,
             Float_t sample_freq,
             Float_t out_min,
             Float_t out_max);

/**
 * @brief 复位准比例谐振控制器
 *
 * @param controller 准比例谐振控制器结构体指针
 *
 * @note 清空误差与谐振输出的历史，系数保留。
 */
void Pr_Reset(Pr_t* controller);

/**
 * @brief 更新准比例谐振控制器状态
 *
 * @param controller 准比例谐振控制器结构体指针
 * @param error 误差 = 目标 - 实际
 * @return Float_t 控制量
 *
 * @note 需以 sample_freq 指定的周期调用。与 PI_Update 不同，本函数直接
 *       接收误差而不是实测值与目标值两个参数，便于在坐标变换后使用。
 */
Float_t Pr_Update(Pr_t* controller, Float_t error);

#endif

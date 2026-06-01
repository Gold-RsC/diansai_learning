#ifndef __SPLL_1PH_SOGI_H__
#define __SPLL_1PH_SOGI_H__

#include "mbase.h"

/**
 * @brief 单相 SOGI 锁相环结构体
 *
 * @details 基于二阶广义积分器(Second Order Generalized Integrator, SOGI)
 *          的正交信号发生器，结合 Park 变换与环路滤波器实现单相电网锁相。
 */
typedef struct {
    struct {
        float grid_freq;   // 电网额定频率，单位：Hz
        float isr_freq;    // ISR 调用频率（即控制周期频率），单位：Hz
        float lpf_b0;      // 环路滤波器系数 b0
        float lpf_b1;      // 环路滤波器系数 b1
    } param;

    struct {
        float fo;          // 输出频率（锁相结果），单位：Hz
        float theta;       // 输出相位角，单位：rad，范围 [0, 2π)
        float sine;        // 相位角正弦值
        float cosine;      // 相位角余弦值
        float u_Q;         // Q轴电压分量（误差信号，理想锁定时为0）
        float u_D;         // D轴电压分量
    } out;

    struct {
        float u[3];        // 输入电压数据缓冲区 [n, n-1, n-2]
        float osg_u[3];    // SOGI 直接通路输出缓冲区 [n, n-1, n-2]
        float osg_qu[3];   // SOGI 正交通路输出缓冲区 [n, n-1, n-2]
        float ylf[2];      // 环路滤波器数据存储 [n, n-1]
        float u_Q_prev;    // 上一时刻 Q 轴电压分量（环路滤波器历史值）

        // SOGI 系数（内部计算，无需用户配置）
        float osg_k;       // SOGI 增益系数 k，通常取 0.5
        float osg_x;       // 中间变量 x = ω * Ts
        float osg_y;       // 中间变量 y = (ω * Ts)^2
        float osg_b0;      // OSG 直接通路分子系数 b0
        float osg_b2;      // OSG 直接通路分子系数 b2
        float osg_a1;      // OSG 分母系数 a1
        float osg_a2;      // OSG 分母系数 a2
        float osg_qb0;     // OSG 正交通路分子系数 qb0
        float osg_qb1;     // OSG 正交通路分子系数 qb1
        float osg_qb2;     // OSG 正交通路分子系数 qb2

        float delta_t;     // 采样时间间隔 = 1 / isr_freq，单位：s
    } _state;
} SPLL_1ph_Sogi_t;

/**
 * @brief 初始化单相 SOGI 锁相环
 *
 * @param spll 单相 SOGI 锁相环结构体指针
 * @param grid_freq 电网额定频率，单位：Hz，如 50.0f 或 60.0f
 * @param isr_freq ISR 调用频率，单位：Hz，如 20000.0f
 * @param lpf_b0 环路滤波器系数 b0
 * @param lpf_b1 环路滤波器系数 b1
 *
 * @note 初始化时会自动计算 SOGI 内部系数，无需手动配置。
 *       环路滤波器系数 b0、b1 需根据带宽需求离线设计。
 */
void spll_1ph_sogi_init(SPLL_1ph_Sogi_t* spll,
                        float grid_freq,
                        float isr_freq,
                        float lpf_b0,
                        float lpf_b1);

/**
 * @brief 复位单相 SOGI 锁相环内部状态
 *
 * @param spll 单相 SOGI 锁相环结构体指针
 *
 * @note 复位后所有内部缓冲区清零，输出频率归零，相位角归零。
 *       通常在启动或故障恢复时调用。
 */
void spll_1ph_sogi_reset(SPLL_1ph_Sogi_t* spll);

/**
 * @brief 单步更新单相 SOGI 锁相环
 *
 * @param spll 单相 SOGI 锁相环结构体指针
 * @param ac_voltage 单相电网电压采样值（标幺值 pu 或实际值均可）
 *
 * @note 需在 ISR 或控制循环中以固定周期调用，周期应与 init 时指定的 isr_freq 一致。
 *       每次调用会更新 spll->out 中的频率、相位角、正余弦值等输出。
 */
void spll_1ph_sogi_update(SPLL_1ph_Sogi_t* spll, float ac_voltage);

/**
 * @brief 重新计算 SOGI 内部系数
 *
 * @param spll 单相 SOGI 锁相环结构体指针
 *
 * @note 当 grid_freq 或 isr_freq 发生变化时，需调用此函数更新内部系数。
 *       正常情况下由 init 自动调用，无需手动调用。
 */
void spll_1ph_sogi_coeff_calc(SPLL_1ph_Sogi_t* spll);

#endif // __SPLL_1PH_SOGI_H__

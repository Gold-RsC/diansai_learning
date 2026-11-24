#ifndef __SPWM_H__
#define __SPWM_H__

#include "mbase.h"

/* 占空比限幅：上下各留 2% 给驱动与死区 */
#define SPWM_DUTY_MIN 0.02f
#define SPWM_DUTY_MAX 0.98f

/**
 * @brief SPWM 调制器
 *
 * @details 纯调制：duty = 0.5 + 0.5 * amplitude * sin(theta)。
 *          模块内不含测量与控制器，闭环由调用方组合：
 *
 *          - 电压闭环：Sin_Analyzer 测基波有效值 -> PI / PR 控制器
 *            -> SPWM_Set_Amplitude -> SPWM_Update
 *          - 并网/并联：SPLL 锁相 -> SPWM_Set_Phase（可配合
 *            SPWM_Set_Frequency）-> SPWM_Update
 *
 *          每次 Update 推进一个 PWM 周期，必须按 param.pwm_freq 的节奏调用。
 *          占空比输出限幅 [0.02, 0.98]，死区与互补输出由下游驱动/定时器负责。
 */
typedef struct {
    struct {
        Float_t pwm_freq;     // PWM 载波频率，单位：Hz
        Float_t target_freq;  // 输出频率，单位：Hz
    } param;

    struct {
        Float_t theta;      // 相位累加器，单位：rad，范围 [0, 2π)
        Float_t delta;      // 每个 PWM 周期的相位增量，单位：rad
        Float_t amplitude;  // 调制比，范围 [0, 1]
    } _state;

    struct {
        Float_t duty;   // 占空比，范围 [0, 1]
        Float_t theta;  // 本拍相位，单位：rad（未推进前的值）
    } out;
} SPWM_t;

/**
 * @brief 初始化 SPWM 调制器
 *
 * @param controller SPWM 结构体指针
 * @param pwm_freq PWM 载波频率，单位：Hz
 * @param target_freq 输出频率，单位：Hz
 * @param amplitude 初始调制比，范围 [0, 1]
 */
void SPWM_Init(SPWM_t* controller, Float_t pwm_freq, Float_t target_freq, Float_t amplitude);

/**
 * @brief 复位相位与输出（调制比保留）
 *
 * @param controller SPWM 结构体指针
 */
void SPWM_Reset(SPWM_t* controller);

/**
 * @brief 设置调制比
 *
 * @param controller SPWM 结构体指针
 * @param amplitude 调制比，内部限幅到 [0, 1]
 *
 * @note 电压环的接入点：PI 或 PR 控制器的输出（以调制比为量纲）直接传入。
 */
void SPWM_Set_Amplitude(SPWM_t* controller, Float_t amplitude);

/**
 * @brief 设置输出频率
 *
 * @param controller SPWM 结构体指针
 * @param freq 输出频率，单位：Hz
 *
 * @note 同时更新 param.target_freq 与相位增量。频率必须低于 pwm_freq / 2。
 */
void SPWM_Set_Frequency(SPWM_t* controller, Float_t freq);

/**
 * @brief 直接设置相位（并网/并联时由 PLL 给定）
 *
 * @param controller SPWM 结构体指针
 * @param theta 相位，单位：rad，内部归一化到 [0, 2π)
 */
void SPWM_Set_Phase(SPWM_t* controller, Float_t theta);

/**
 * @brief 推进一步，计算占空比
 *
 * @param controller SPWM 结构体指针
 * @return Float_t 占空比，范围 [0.02, 0.98]
 *
 * @note 用当前调制比与相位算 duty，然后把相位推进 delta。
 */
Float_t SPWM_Update(SPWM_t* controller);


#endif

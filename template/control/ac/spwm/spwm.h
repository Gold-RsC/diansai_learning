#ifndef __SPWM_H__
#define __SPWM_H__

#include "pid.h"
#include "sin_analyzer.h"
#include "spll_1ph_sogi.h"

typedef struct {
    struct {
        Float_t pwm_freq;            // PWM载波频率，单位：Hz
        Float_t target_freq;         // 表现频率，单位：Hz
        Float_t target_rms_voltage;  // 目标电压方均根，单位：V
        Float_t max_amplitude;       // 最大调制比, 0~1
    } param;

    struct {
        PI_t pi_controller;
        Sin_Analyzer_t sin_analyzer;  // 正弦波分析器

        Float_t phase_accumulator;  // 相位累加器，单位：弧度
        Float_t phase_increment;    // 相位增量，单位：弧度
        // phase_increment = 2 * MATH_PI × target_freq / pwm_freq，每次 PWM 中断累加一次。
    } _state;


} SPWM_t;

/**
 * @brief 初始化 PWM
 *
 * @param controller PWM 结构体指针
 * @param pwm_freq PWM 载波频率，单位：Hz
 * @param target_freq 表现频率，单位：Hz
 * @param target_rms_voltage 目标电压方均根，单位：V
 */
void SPWM_Init(SPWM_t* controller,
               Float_t pwm_freq,           // PWM载波频率，单位：Hz
               Float_t target_freq,        // 表现频率，单位：Hz
               Float_t target_rms_voltage  // 目标电压方均根，单位：V
);


/**
 * @brief 更新 PWM 占空比 (开环)
 *
 * @param controller PWM 结构体指针
 * @return Float_t PWM 占空比，单位：0~1
 */
Float_t SPWM_Update_Open(SPWM_t* controller);

/**
 * @brief 更新 PWM 占空比 (闭环)
 *
 * @param controller PWM 结构体指针
 * @param voltage_sample 电压采样值，单位：V
 * @param current_sample 电流采样值，单位：A
 * @return Float_t PWM 占空比，单位：0~1
 */
Float_t SPWM_Update(SPWM_t* controller, Float_t voltage_sample, Float_t current_sample);


typedef struct {
    SPWM_t spwm;
    SPLL_1ph_Sogi_t* spll;
} SPWM_PLL_t;

/**
 * @brief 初始化 PWM PLL
 *
 * @param controller PWM PLL 结构体指针
 * @param pwm_freq PWM 载波频率，单位：Hz
 * @param target_freq 表现频率，单位：Hz
 * @param target_rms_voltage 目标电压方均根，单位：V
 * @param spll PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
 */

void SPWM_PLL_Init(SPWM_PLL_t* controller,
                   Float_t pwm_freq,            // PWM载波频率，单位：Hz
                   Float_t target_freq,         // 表现频率，单位：Hz
                   Float_t target_rms_voltage,  // 目标电压方均根，单位：V
                   SPLL_1ph_Sogi_t* spll      // PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
);

/**
 * @brief 更新 PWM PLL 占空比
 *
 * @param controller PWM PLL 结构体指针
 * @param grid_voltage 电网电压采样值，单位：V
 * @param voltage_sample 电压采样值，单位：V
 * @param current_sample 电流采样值，单位：A
 */
Float_t SPWM_PLL_Update(SPWM_PLL_t* controller, Float_t grid_voltage, Float_t voltage_sample, Float_t current_sample);

#endif

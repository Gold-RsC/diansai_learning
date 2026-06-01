#ifndef __SPWM_H__
#define __SPWM_H__

#include "pid.h"
#include "sin_analyzer.h"
#include "spll_1ph_sogi.h"

typedef struct {
    struct {
        float pwm_freq;            // PWM载波频率，单位：Hz
        float target_freq;         // 表现频率，单位：Hz
        float target_rms_voltage;  // 目标电压方均根，单位：V
        float max_amplitude;       // 最大调制比, 0~1
    } param;

    struct {
        PI_t pi_controller;
        Sin_Analyzer_t sin_analyzer;  // 正弦波分析器

        float phase_accumulator;  // 相位累加器，单位：弧度
        float phase_increment;    // 相位增量，单位：弧度
        // phase_increment = 2 * MATH_PI × target_freq / pwm_freq，每次 PWM 中断累加一次。
    } _state;


} SPWM_t;

/**
 * @brief 初始化 PWM
 *
 * @param spwm PWM 结构体指针
 * @param pwm_freq PWM 载波频率，单位：Hz
 * @param target_freq 表现频率，单位：Hz
 * @param target_rms_voltage 目标电压方均根，单位：V
 */
void spwm_init(SPWM_t* spwm,
               float pwm_freq,           // PWM载波频率，单位：Hz
               float target_freq,        // 表现频率，单位：Hz
               float target_rms_voltage  // 目标电压方均根，单位：V
);


/**
 * @brief 更新 PWM 占空比 (开环)
 *
 * @param spwm PWM 结构体指针
 * @return float PWM 占空比，单位：0~1
 */
float spwm_update_open(SPWM_t* spwm);

/**
 * @brief 更新 PWM 占空比 (闭环)
 *
 * @param spwm PWM 结构体指针
 * @param voltage_sample 电压采样值，单位：V
 * @param current_sample 电流采样值，单位：A
 * @return float PWM 占空比，单位：0~1
 */
float spwm_update(SPWM_t* spwm, float voltage_sample, float current_sample);


typedef struct {
    SPWM_t spwm;
    SPLL_1ph_Sogi_t* spll;
} SPWM_PLL_t;

/**
 * @brief 初始化 PWM PLL
 *
 * @param spwm_pll PWM PLL 结构体指针
 * @param pwm_freq PWM 载波频率，单位：Hz
 * @param target_freq 表现频率，单位：Hz
 * @param target_rms_voltage 目标电压方均根，单位：V
 * @param spll PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
 */

void spwm_pll_init(SPWM_PLL_t* spwm_pll,
                   float pwm_freq,            // PWM载波频率，单位：Hz
                   float target_freq,         // 表现频率，单位：Hz
                   float target_rms_voltage,  // 目标电压方均根，单位：V
                   SPLL_1ph_Sogi_t* spll      // PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
);

/**
 * @brief 更新 PWM PLL 占空比
 *
 * @param spwm_pll PWM PLL 结构体指针
 * @param grid_voltage 电网电压采样值，单位：V
 * @param voltage_sample 电压采样值，单位：V
 * @param current_sample 电流采样值，单位：A
 */
void spwm_pll_update(SPWM_PLL_t* spwm_pll, float grid_voltage, float voltage_sample, float current_sample);

#endif

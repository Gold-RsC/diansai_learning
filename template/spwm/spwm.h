#ifndef __SPWM_H__
#define __SPWM_H__

#include "pid.h"
#include "sin_analyzer.h"
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

void spwm_init(SPWM_t* spwm,
               float pwm_freq,           // PWM载波频率，单位：Hz
               float target_freq,        // 表现频率，单位：Hz
               float target_rms_voltage  // 目标电压方均根，单位：V
);


// 开环
float spwm_update_open(SPWM_t* spwm);
// 闭环
float spwm_update(SPWM_t* spwm, float voltage_sample, float current_sample);

#endif

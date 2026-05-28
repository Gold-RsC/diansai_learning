#ifndef __SPWM_H__
#define __SPWM_H__

#include "pid.h"
#include "sin_analyzer.h"
typedef struct {
    struct {
        float pwm_freq;       // PWM载波频率，单位：Hz
        float target_freq;    // 表现频率，单位：Hz
        float max_amplitude;  // 最大幅值
    } param;

    struct {
        PI_t pi_controller;
        Sin_Analyzer_t sin_analyzer;  // 正弦波分析器

    } _state;


} SPWM_t;
#endif

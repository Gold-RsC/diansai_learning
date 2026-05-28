#ifndef __SIN_ANALYZER_H__
#define __SIN_ANALYZER_H__

#include "mbase.h"

typedef struct {
    struct {
        float measure_freq;  // 测量频率，单位：Hz
        float min_freq;      // 最小频率，单位：Hz
        float max_freq;      // 最大频率，单位：Hz
    } param;

    struct {
        float rms_voltage;  // 电压方均根，单位：V
        float rms_current;  // 电流方均根，单位：A

        float active_power;    // 有功功率，单位：W
        float apparent_power;  // 视在功率，单位：W

        float power_factor;  // 功率因数

        float freq;  // 频率，单位：Hz

        bool data_ready;  // 数据是否准备就绪
    } out;

    struct {
        uint32_t sample_count;    // 采样次数
        float voltage_squre_sum;  // 电压平方总和，单位：V^2
        float current_squre_sum;  // 电流平方总和，单位：A^2
        float power_sum;          // 功率总和，单位：W
        bool prev_sign;           // 上一个采样的符号
        bool half_cycle_flag;     // 半周标志标志位
    } _state;
} Sin_Analyzer_t;


void sin_analyzer_init(Sin_Analyzer_t* analyzer, float measure_freq, float min_freq, float max_freq);
// 在ADC采样中断中使用
void sin_analyzer_update(Sin_Analyzer_t* analyzer,
                         float voltage_sample,  // 采样电压，单位：V
                         float current_sample   // 采样电流，单位：A
);


#endif

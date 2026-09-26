#ifndef __SIN_ANALYZER_H__
#define __SIN_ANALYZER_H__

#include "mbase.h"

typedef struct {
    struct {
        Float_t measure_freq;   // 测量频率，单位：Hz
        Float_t min_freq;       // 最小频率，单位：Hz
        Float_t max_freq;       // 最大频率，单位：Hz
        Float_t zcd_threshold;  // ZCD阈值，单位：V
    } param;

    struct {
        Float_t rms_voltage;  // 电压方均根，单位：V
        Float_t rms_current;  // 电流方均根，单位：A

        Float_t active_power;    // 有功功率，单位：W
        Float_t apparent_power;  // 视在功率，单位：W

        Float_t power_factor;  // 功率因数

        Float_t freq;  // 频率，单位：Hz

        bool data_ready;  // 数据是否准备就绪
    } out;

    struct {
        uint32_t sample_count;    // 采样次数
        Float_t voltage_squre_sum;  // 电压平方总和，单位：V^2
        Float_t current_squre_sum;  // 电流平方总和，单位：A^2
        Float_t power_sum;          // 功率总和，单位：W
        bool prev_sign;           // 上一个采样的符号
        bool half_cycle_flag;     // 半周标志标志位
    } _state;
} Sin_Analyzer_t;


/**
 * @brief 初始化正弦波分析器
 *
 * @param analyzer 正弦波分析器 结构体指针
 * @param measure_freq 测量频率，单位：Hz
 * @param min_freq 最小频率，单位：Hz
 * @param max_freq 最大频率，单位：Hz
 */
void Sin_Analyzer_Init(Sin_Analyzer_t* analyzer, Float_t measure_freq, Float_t min_freq, Float_t max_freq);

/**
 * @brief 更新正弦波分析器
 *
 * @param analyzer 正弦波分析器 结构体指针
 * @param voltage_sample 采样电压，单位：V
 * @param current_sample 采样电流，单位：A
 * @note 在ADC采样中断中使用，更新正弦波分析器的输入数据
 */
void Sin_Analyzer_Update(Sin_Analyzer_t* analyzer,
                         Float_t voltage_sample,  // 采样电压，单位：V
                         Float_t current_sample   // 采样电流，单位：A
);


#endif

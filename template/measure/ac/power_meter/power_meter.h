#ifndef __POWER_METER_H__
#define __POWER_METER_H__

#include "mbase.h"
#include "spll_1ph_sogi.h"

/**
 * @brief 单相功率测量结构体
 *
 * @details 对电压、电流各做一路 SOGI 正交分解，用 αβ 分量计算瞬时功率，
 *          在电压重构基波的过零点之间按整周期平均，得到基波口径的
 *          有功 / 无功 / 视在功率与功率因数。
 */
typedef struct {
    struct {
        Float_t sample_freq;  // 采样频率，单位：Hz
        Float_t min_freq;     // 最小频率，单位：Hz
        Float_t max_freq;     // 最大频率，单位：Hz
    } param;

    struct {
        SPLL_1ph_Sogi_t spll_v;  // 电压 SOGI-PLL：只用其 SOGI 正交分量
        SPLL_1ph_Sogi_t spll_i;  // 电流 SOGI-PLL：只用其 SOGI 正交分量
        Float_t alpha_prev;      // 上一拍电压 SOGI 同相分量 α（用于检测过零）
        Float_t p_sum;           // 本周期内有功瞬时值的累加
        Float_t q_sum;           // 本周期内无功瞬时值的累加
        Float_t v_sq_sum;        // 本周期内电压正交分量平方和的累加
        Float_t i_sq_sum;        // 本周期内电流正交分量平方和的累加
        Float_t freq_avg;        // 频率块平均输出，单位：Hz
        Float_t freq_sum;        // 频率统计窗口内的采样点数累加
        uint32_t sample_count;   // 本周期采样点数
        uint32_t freq_cycles;    // 频率统计窗口内的周期数
        uint32_t cycle_count;    // 已完成的估计周期数（启动屏蔽用）
        bool freq_valid;         // 频率块平均是否已完成首次更新
    } _state;

    struct {
        Float_t active_power;    // 有功功率，单位：W
        Float_t reactive_power;  // 无功功率，单位：var（感性为正）
        Float_t apparent_power;  // 视在功率，单位：VA
        Float_t power_factor;    // 功率因数，范围 [-1, 1]
        Float_t rms_voltage;     // 电压有效值，单位：V
        Float_t rms_current;     // 电流有效值，单位：A
        Float_t freq;            // 跟踪频率，单位：Hz

        bool data_ready;  // 数据是否就绪（电平式，每个基波周期置位一次）
        bool locked;      // 输出是否可信（频率与幅值均在有效范围内）
    } out;
} Power_Meter_t;

/**
 * @brief 初始化单相功率测量
 *
 * @param meter 功率测量结构体指针
 * @param sample_freq 采样频率，单位：Hz（必须与实际调用频率一致）
 * @param min_freq 最小频率，单位：Hz
 * @param max_freq 最大频率，单位：Hz
 *
 * @note 两路 SOGI 的额定频率取 GRID_FREQUENCY 并限幅到 [min_freq, max_freq]，
 *       与 sin_analyzer 相同，额定频率附近精度最好。
 */
void Power_Meter_Init(Power_Meter_t* meter, Float_t sample_freq, Float_t min_freq, Float_t max_freq);

/**
 * @brief 复位功率测量（重新锁定）
 *
 * @param meter 功率测量结构体指针
 *
 * @note 保留 param，复位两路 SOGI、累加器与所有输出。
 */
void Power_Meter_Reset(Power_Meter_t* meter);

/**
 * @brief 更新单相功率测量
 *
 * @param meter 功率测量结构体指针
 * @param voltage_sample 电压采样值，单位：V
 * @param current_sample 电流采样值，单位：A
 *
 * @note 需在 ADC 中断中按 sample_freq 的节奏调用。out.data_ready 为电平式，
 *       每个基波周期（电压重构 α 的上升过零）置位一次，由消费者清零。
 */
void Power_Meter_Update(Power_Meter_t* meter, Float_t voltage_sample, Float_t current_sample);


#endif

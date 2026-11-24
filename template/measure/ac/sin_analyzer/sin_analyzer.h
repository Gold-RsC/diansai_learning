#ifndef __SIN_ANALYZER_H__
#define __SIN_ANALYZER_H__

#include "mbase.h"
#include "spll_1ph_sogi.h"

typedef struct {
    struct {
        Float_t measure_freq;   // 采样频率，单位：Hz
        Float_t min_freq;       // 最小频率，单位：Hz
        Float_t max_freq;       // 最大频率，单位：Hz
        Float_t zcd_threshold;  // 最小基波有效值门限，低于该值不输出
    } param;

    struct {
        Float_t rms;        // 基波有效值，单位：与输入同量纲
        Float_t freq;       // 跟踪频率，单位：Hz
        Float_t amplitude;  // 基波峰值，单位：与输入同量纲
        Float_t theta;      // 基波相位，单位：rad，范围 [0, 2π)
        Float_t rms_total;  // 真有效值（含谐波与直流），单位：与输入同量纲
        Float_t dc_offset;  // 输入直流分量，单位：与输入同量纲

        bool data_ready;  // 数据是否就绪（电平式，每个基波周期置位一次）
        bool locked;      // 输出是否可信（频率与幅值均在有效范围内）
    } out;

    struct {
        SPLL_1ph_Sogi_t spll;   // 内部 SOGI-PLL：只用其 SOGI 正交分量（α/β）
        Float_t alpha_prev;     // 上一拍 SOGI 同相分量 α（用于检测过零）
        Float_t amp_sq_sum;     // 本周期内基波幅值平方的累加
        Float_t sq_sum;         // 本周期内输入平方的累加（真有效值）
        Float_t dc_sum;         // 本周期内输入的累加（直流分量）
        Float_t freq_avg;       // 频率块平均输出，单位：Hz
        Float_t freq_sum;       // 频率统计窗口内的采样点数累加
        uint32_t sample_count;  // 本周期采样点数
        uint32_t freq_cycles;   // 频率统计窗口内的周期数
        uint32_t cycle_count;   // 已完成的估计周期数（启动屏蔽用）
        bool freq_valid;        // 频率块平均是否已完成首次更新
    } _state;
} Sin_Analyzer_t;


/**
 * @brief 初始化正弦分析器
 *
 * @param analyzer 正弦分析器结构体指针
 * @param measure_freq 采样频率，单位：Hz（必须与实际调用频率一致）
 * @param min_freq 最小频率，单位：Hz
 * @param max_freq 最大频率，单位：Hz
 * @param zcd_threshold 最小基波有效值门限，低于该值不更新输出
 *
 * @note 内部用 SOGI 相位跟踪提取基波正交分量：幅值取 αβ 的平方和，频率由
 *       重构基波 α 的过零间隔反推，相位由 atan2(α, -β) 得到。
 *       PLL 额定频率取 GRID_FREQUENCY 并限幅到 [min_freq, max_freq]。
 */
void Sin_Analyzer_Init(
    Sin_Analyzer_t* analyzer, Float_t measure_freq, Float_t min_freq, Float_t max_freq, Float_t zcd_threshold);

/**
 * @brief 复位正弦分析器（重新锁定）
 *
 * @param analyzer 正弦分析器结构体指针
 *
 * @note 保留 param，复位 SOGI、累加器与所有输出，用于异常后重新同步。
 */
void Sin_Analyzer_Reset(Sin_Analyzer_t* analyzer);

/**
 * @brief 更新正弦分析器
 *
 * @param analyzer 正弦分析器结构体指针
 * @param sample 采样值
 *
 * @note 需在 ADC 中断中按 measure_freq 的节奏调用。out.theta 每个采样点更新；
 *       out.data_ready 为电平式，每个基波周期（重构 α 的上升过零）置位一次，
 *       由消费者清零；out.locked 表示当前输出是否可信。
 */
void Sin_Analyzer_Update(Sin_Analyzer_t* analyzer, Float_t sample);


#endif

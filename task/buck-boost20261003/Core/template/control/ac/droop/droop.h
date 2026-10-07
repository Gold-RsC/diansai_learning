#ifndef __DROOP_H__
#define __DROOP_H__

#include "mbase.h"
#include "lpf.h"

/**
 * @brief 下垂控制
 *
 * @details 按有功与无功调节输出频率与电压幅值：
 *              f = f0 - m * P
 *              V = V0 - n * Q
 *
 *          多台逆变器并联时，各机按各自容量设置 m、n，即可在没有通信线的
 *          前提下自动分摊负载：输出功率大的机器频率/幅值下降更多，自然
 *          让出负载。这是并联运行类赛题（2023 单相逆变器并联、2024 AC-AC
 *          变换电路并联）的核心算法。
 *
 *          P、Q 在进入下垂计算前先经一阶低通滤波。这一步不可省略：
 *          功率的瞬时脉动会直接调制频率，形成正反馈导致系统振荡。
 *
 * @note m 的量纲是 Hz/W，n 的量纲是 V/var。容量大的机器 m、n 应取小值。
 *       P、Q 需由测量模块（如 sin_analyzer）提供。
 */
typedef struct {

    struct {
        Float_t freq_noload;  // 空载频率 f0，单位：Hz
        Float_t amp_noload;   // 空载电压幅值 V0，单位：V

        Float_t m;  // 有功-频率下垂系数，单位：Hz/W
        Float_t n;  // 无功-电压下垂系数，单位：V/var

        Float_t p_max;  // 有功限幅，单位：W
        Float_t q_max;  // 无功限幅，单位：var

        Float_t filter_cutoff;  // P/Q 低通截止频率，单位：Hz
        Float_t sample_freq;    // 采样（ISR 调用）频率，单位：Hz
    } param;

    struct {
        Lpf_Filter_t lpf_p;  // 有功低通
        Lpf_Filter_t lpf_q;  // 无功低通
    } _state;

    struct {
        Float_t freq;       // 输出频率，单位：Hz
        Float_t amplitude;  // 输出电压幅值，单位：V
    } out;

} Droop_t;

/**
 * @brief 初始化下垂控制
 *
 * @param controller 下垂控制结构体指针
 * @param freq_noload 空载频率 f0，单位：Hz
 * @param amp_noload 空载电压幅值 V0，单位：V
 * @param m 有功-频率下垂系数，单位：Hz/W
 * @param n 无功-电压下垂系数，单位：V/var
 * @param filter_cutoff P/Q 低通截止频率，单位：Hz，一般取 5 ~ 20Hz
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 *
 * @note p_max / q_max 默认取下垂线的完整量程（即 f0/m 与 V0/n，频率与幅值
 *       最低降到 0），不设人为的百分比上限。若需按变流器容量收紧，
 *       初始化后直接改 param.p_max / param.q_max。
 */
void Droop_Init(Droop_t* controller,
                Float_t freq_noload,
                Float_t amp_noload,
                Float_t m,
                Float_t n,
                Float_t filter_cutoff,
                Float_t sample_freq);

/**
 * @brief 复位下垂控制
 *
 * @param controller 下垂控制结构体指针
 *
 * @note 输出回到空载值，P/Q 低通状态清零。
 */
void Droop_Reset(Droop_t* controller);

/**
 * @brief 更新下垂控制状态
 *
 * @param controller 下垂控制结构体指针
 * @param p 实测有功功率，单位：W
 * @param q 实测无功功率，单位：var
 *
 * @note 需以 sample_freq 指定的周期调用。结果在 out.freq 与 out.amplitude。
 */
void Droop_Update(Droop_t* controller, Float_t p, Float_t q);

#endif

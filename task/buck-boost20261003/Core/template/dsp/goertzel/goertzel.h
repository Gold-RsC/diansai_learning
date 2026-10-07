#ifndef __GOERTZEL_H__
#define __GOERTZEL_H__

#include "mbase.h"

/**
 * @brief 单频点 DFT（Goertzel 算法）
 *
 * @details 只计算一个指定频率处的频谱分量。复杂度 O(N) 单次遍历，
 *          不需要 twiddle 因子表，也不需要存储整个采样数组。
 *
 *          递推式：
 *              s[n] = x[n] + 2*cos(w)*s[n-1] - s[n-2]
 *          窗口结束时由状态量还原频谱分量：
 *              X = s[N-1] * e^(jw) - s[N-2]
 *          相位基准与 X[k] = sum x[n]*e^(-jwn) 一致。
 *              A = 2*|X| / N
 *          其中 w = 2*pi*target_freq/sample_freq，N = window_size。
 *
 *          每累加满 window_size 个采样点输出一次幅值与相位。
 *
 * @note 若 target_freq 不是 sample_freq/window_size 的整数倍，会产生频谱泄漏。
 *       做谐波分析时，window_size 应取【基波】的整数个周期（N = fs/f0），
 *       这样各次谐波都落在整数 bin 上；若按目标谐波自身的周期取窗口，
 *       高次谐波反而会泄漏。
 */
typedef struct {

    struct {
        Float_t sample_freq;  // 采样（ISR 调用）频率，单位：Hz
        Float_t target_freq;  // 目标频率，单位：Hz

        uint32_t window_size;  // 窗口长度，单位：采样点数
    } param;

    struct {
        Float_t w;      // 目标角频率，w = 2*pi*target_freq/sample_freq，单位：rad
        Float_t coeff;  // 递推系数 2*cos(w)

        Float_t s1;  // 状态量 s[n-1]
        Float_t s2;  // 状态量 s[n-2]

        uint32_t count;  // 当前窗口已累加的采样点数
    } _state;

    struct {
        Float_t magnitude;  // 幅值
        Float_t phase;      // 相位，单位：rad，范围 (-pi, pi]

        bool ready;  // 本拍是否产出新结果（单拍脉冲）
    } out;

} Goertzel_t;

/**
 * @brief 初始化单频点 DFT
 *
 * @param goertzel 单频点 DFT 结构体指针
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 * @param target_freq 目标频率，单位：Hz
 * @param window_size 窗口长度，单位：采样点数
 *
 * @note 需保证 target_freq < sample_freq / 2，否则目标频率超过奈奎斯特频率。
 */
void Goertzel_Init(Goertzel_t* goertzel, Float_t sample_freq, Float_t target_freq, uint32_t window_size);

/**
 * @brief 复位单频点 DFT
 *
 * @param goertzel 单频点 DFT 结构体指针
 *
 * @note 清空累加状态与输出，重新开始一个窗口。系数无需重算。
 */
void Goertzel_Reset(Goertzel_t* goertzel);

/**
 * @brief 更新单频点 DFT 状态
 *
 * @param goertzel 单频点 DFT 结构体指针
 * @param sample 采样值
 * @return Float_t 最新一次的幅值
 *
 * @note 需以 sample_freq 指定的周期逐点调用。每满 window_size 个点，
 *       out.magnitude / out.phase 更新一次，同时 out.ready 置位一拍。
 *       ready 是单拍脉冲、不跨拍残留，因此可供多个消费者读取，这一点
 *       与 Sin_Analyzer_t 的电平式 data_ready 不同。
 */
Float_t Goertzel_Update(Goertzel_t* goertzel, Float_t sample);

#endif

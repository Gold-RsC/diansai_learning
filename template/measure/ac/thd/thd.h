#ifndef __THD_H__
#define __THD_H__

#include "mbase.h"
#include "goertzel.h"

/**
 * @brief 最高分析次数
 *
 * @details THD 内部为 1 ~ THD_MAX_ORDER 次各建一个 Goertzel 单元，
 *          修改此宏会改变 Thd_t 的体积（每增加一次约多 40 字节）。
 */
#define THD_MAX_ORDER 15

/**
 * @brief 总谐波畸变率
 *
 * @details 窗口长度固定取【一个基波周期】N = fs/f0，此时第 h 次谐波
 *          恰好落在第 h 号 bin 上（整数 bin），全部谐波都不会泄漏。
 *
 *          定义：
 *              THD = sqrt( sum_{h=2..max_order} V_h^2 ) / V_1
 *          输出为小数，0.05 表示 5%。
 *
 * @note 基波幅值过小时（低于 1e-9）判定为无信号，THD 输出 0。
 */
typedef struct {

    struct {
        Float_t sample_freq;       // 采样（ISR 调用）频率，单位：Hz
        Float_t fundamental_freq;  // 基波频率，单位：Hz

        uint32_t window_size;  // 窗口长度 = sample_freq / fundamental_freq，单位：采样点数

        uint8_t max_order;  // 实际分析的最高次数，上限 THD_MAX_ORDER
    } param;

    struct {
        Goertzel_t gz[THD_MAX_ORDER + 1];  // [0] 空置，[1] 基波，[2..] 各次谐波
    } _state;

    struct {
        Float_t fundamental;  // 基波幅值
        Float_t thd;          // 总谐波畸变率，小数

        bool ready;  // 本拍是否产出新结果（单拍脉冲）
    } out;

} Thd_t;

/**
 * @brief 初始化总谐波畸变率测量
 *
 * @param analyzer 总谐波畸变率结构体指针
 * @param sample_freq 采样（ISR 调用）频率，单位：Hz
 * @param fundamental_freq 基波频率，单位：Hz
 * @param max_order 最高分析次数，超过 THD_MAX_ORDER 时按上限截断
 *
 * @note 窗口长度由 sample_freq / fundamental_freq 自动算出，无需指定。
 *       需保证 max_order * fundamental_freq < sample_freq / 2。
 */
void Thd_Init(Thd_t* analyzer, Float_t sample_freq, Float_t fundamental_freq, uint8_t max_order);

/**
 * @brief 复位总谐波畸变率测量
 *
 * @param analyzer 总谐波畸变率结构体指针
 *
 * @note 清空全部 Goertzel 单元的累加状态，重新开始一个窗口。
 */
void Thd_Reset(Thd_t* analyzer);

/**
 * @brief 更新总谐波畸变率测量状态
 *
 * @param analyzer 总谐波畸变率结构体指针
 * @param sample 采样值
 * @return Float_t 最新一次的 THD
 *
 * @note 需以 sample_freq 指定的周期逐点调用。每满一个基波周期，
 *       out.fundamental / out.thd 更新一次，同时 out.ready 置位一拍。
 */
Float_t Thd_Update(Thd_t* analyzer, Float_t sample);

#endif

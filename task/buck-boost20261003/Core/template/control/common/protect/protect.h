#ifndef __PROTECT_H__
#define __PROTECT_H__

#include "mbase.h"

/**
 * @brief 阈值保护
 *
 * @details 过压/过流（上阈值）与欠压（下阈值）保护，含两级防护：
 *          1. 数字滤波：连续越限满 trip_count 拍才认为有效，抑制尖峰误触发
 *          2. 锁存：一旦 out.tripped 置位就保持，直到 Protect_Reset 清除
 *
 *          过压/欠压标志带回差，避免在阈值附近抖动：
 *              置位：value > over_threshold
 *              复位：value < over_threshold - hysteresis
 *          欠压方向同理，复位条件为 value > under_threshold + hysteresis。
 *
 * @note tripped 是锁存位，需要调用者主动响应（切断 PWM 等）并调
 *       Protect_Reset 清除，本模块不会自动恢复。
 */
typedef struct {

    struct {
        Float_t over_threshold;   // 上阈值（过压/过流）
        Float_t under_threshold;  // 下阈值（欠压）

        Float_t hysteresis;  // 回差，应大于 0

        uint16_t trip_count;  // 连续越限多少拍才跳闸，1 表示不滤波
    } param;

    struct {
        uint16_t over_count;   // 上阈值连续越限拍数
        uint16_t under_count;  // 下阈值连续越限拍数

        bool tripped;  // 锁存的跳闸状态
    } _state;

    struct {
        bool over;     // 当前是否越上阈值
        bool under;    // 当前是否越下阈值
        bool tripped;  // 是否已跳闸（锁存）
    } out;

} Protect_t;

/**
 * @brief 初始化阈值保护
 *
 * @param controller 阈值保护结构体指针
 * @param over_threshold 上阈值（过压/过流）
 * @param under_threshold 下阈值（欠压）
 * @param hysteresis 回差，应大于 0
 * @param trip_count 连续越限多少拍才跳闸，1 表示不滤波
 *
 * @note 需保证 under_threshold < over_threshold 且 hysteresis < 两者之差，
 *       否则回差区间会重叠。
 */
void Protect_Init(Protect_t* controller,
                  Float_t over_threshold,
                  Float_t under_threshold,
                  Float_t hysteresis,
                  uint16_t trip_count);

/**
 * @brief 复位阈值保护
 *
 * @param controller 阈值保护结构体指针
 *
 * @note 清除锁存的跳闸位与越限计数。若输入仍处于越限状态，下一拍会重新跳闸。
 */
void Protect_Reset(Protect_t* controller);

/**
 * @brief 更新阈值保护状态
 *
 * @param controller 阈值保护结构体指针
 * @param value 被监视量（电压或电流）
 *
 * @note 需以固定周期调用。调用者应在每次更新后检查 out.tripped。
 */
void Protect_Update(Protect_t* controller, Float_t value);

#endif

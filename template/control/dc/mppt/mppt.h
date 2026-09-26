#ifndef __MPPT_H__
#define __MPPT_H__

#include "mbase.h"

/**
 * @brief 扰动观察法 MPPT
 *
 * @details 每隔 period 拍扰动一次占空比，比较扰动前后的功率：
 *              功率上升 -> 保持原方向继续扰动
 *              功率下降 -> 反向扰动
 *
 *          占空比始终限制在 [min_duty, max_duty] 内，到达边界后方向自然
 *          反转，不需要额外处理。
 *
 * @note 输入电压/电流应先经过低通滤波（可用 lpf 模块），否则采样噪声会
 *       让功率比较结果随机翻转，导致工作点来回抖。
 *       本模块同时适用光伏与风力等最大功率点跟踪场合。
 */
typedef struct {

    struct {
        Float_t step;  // 每轮扰动步长，应大于 0

        Float_t min_duty;  // 占空比下限
        Float_t max_duty;  // 占空比上限

        uint32_t period;  // 每多少拍扰动一次
    } param;

    struct {
        Float_t duty;       // 当前占空比
        Float_t direction;  // 上次扰动方向，+1 或 -1
        Float_t prev_power; // 上次扰动后的功率

        uint32_t count;  // 距上次扰动的拍数
    } _state;

    Float_t out;  // 输出占空比

} Mppt_t;

/**
 * @brief 初始化扰动观察法 MPPT
 *
 * @param mppt 扰动观察法 MPPT 结构体指针
 * @param step 每轮扰动步长
 * @param min_duty 占空比下限
 * @param max_duty 占空比上限
 * @param period 每多少拍扰动一次
 *
 * @note 扰动周期越短跟踪越快，但过短时功率尚未稳定就开始比较，
 *       容易误判方向。一般取被测功率低通时间常数的 2 ~ 3 倍。
 */
void Mppt_Init(Mppt_t* mppt, Float_t step, Float_t min_duty, Float_t max_duty, uint32_t period);

/**
 * @brief 复位扰动观察法 MPPT
 *
 * @param mppt 扰动观察法 MPPT 结构体指针
 *
 * @note 占空比回到下限，扰动方向设为正向。
 */
void Mppt_Reset(Mppt_t* mppt);

/**
 * @brief 更新扰动观察法 MPPT 状态
 *
 * @param mppt 扰动观察法 MPPT 结构体指针
 * @param v_in 输入电压，单位：V
 * @param i_in 输入电流，单位：A
 * @return Float_t 占空比
 *
 * @note 需以固定周期逐点调用，不要在中间跳过调用，否则扰动周期不准确。
 */
Float_t Mppt_Update(Mppt_t* mppt, Float_t v_in, Float_t i_in);

#endif

#ifndef __DC_METER_H__
#define __DC_METER_H__

#include "mbase.h"

/**
 * @brief 直流量测量
 *
 * @details 按块统计直流量的平均值与纹波峰峰值。每累加满 window_size 个
 *          采样点输出一次：
 *              average = 窗口内均值
 *              ripple  = 窗口内最大值 - 最小值
 *
 *          纹波峰峰值是 DC-DC 类赛题的直接评分项。窗口应覆盖整数个
 *          开关周期，否则纹波会被截断而偏小。
 */
typedef struct {

    struct {
        uint32_t window_size;  // 窗口长度，单位：采样点数，建议取整数个开关周期
    } param;

    struct {
        Float_t sum;  // 窗口内累加和

        Float_t min_value;  // 窗口内最小值
        Float_t max_value;  // 窗口内最大值

        uint32_t count;  // 当前窗口已累加的采样点数
    } _state;

    struct {
        Float_t average;  // 平均值
        Float_t ripple;   // 纹波峰峰值 = max_value - min_value

        bool ready;  // 本拍是否产出新结果（单拍脉冲）
    } out;

} Dc_Meter_t;

/**
 * @brief 初始化直流量测量
 *
 * @param analyzer 直流量测量结构体指针
 * @param window_size 窗口长度，单位：采样点数
 *
 * @note window_size 不能为 0。
 */
void Dc_Meter_Init(Dc_Meter_t* analyzer, uint32_t window_size);

/**
 * @brief 复位直流量测量
 *
 * @param analyzer 直流量测量结构体指针
 *
 * @note 清空窗口累加状态，重新开始一个窗口。
 */
void Dc_Meter_Reset(Dc_Meter_t* analyzer);

/**
 * @brief 更新直流量测量状态
 *
 * @param analyzer 直流量测量结构体指针
 * @param sample 采样值
 * @return Float_t 最新一次的平均值
 *
 * @note 需以固定的采样周期逐点调用。每满 window_size 个点，
 *       out.average / out.ripple 更新一次，同时 out.ready 置位一拍。
 */
Float_t Dc_Meter_Update(Dc_Meter_t* analyzer, Float_t sample);

#endif

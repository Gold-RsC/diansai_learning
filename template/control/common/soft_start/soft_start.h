#ifndef __SOFT_START_H__
#define __SOFT_START_H__

#include "mbase.h"

/**
 * @brief 软启动斜坡限制器
 *
 * @details 让输出以每拍最多 step 的速率逼近目标值，把阶跃目标变成斜坡。
 *          上下行都受限，因此也兼作变化率限制器。
 *
 *          典型用途：DC-DC 上电时限住占空比爬升，避免冲击电流；
 *          逆变器启动时限住调制比爬升，避免输出过冲。
 *
 * @note 本模块只做限速，不做闭环。它与 PI 的配合方式是：
 *          目标 = 软启动输出，PI 输入 = 该目标与实际值之差。
 */
typedef struct {

    struct {
        Float_t step;   // 每拍最大变化量
        Float_t start;  // 起始值
    } param;

    struct {
        Float_t value;  // 当前值
    } _state;

    Float_t out;

} Soft_Start_t;

/**
 * @brief 初始化软启动斜坡限制器
 *
 * @param soft_start 软启动结构体指针
 * @param step 每拍最大变化量，应大于 0
 * @param start 起始值
 *
 * @note 达到目标所需拍数 = |目标 - start| / step，据此换算软启动时间：
 *       软启动时间 = 拍数 * 调用周期。
 */
void Soft_Start_Init(Soft_Start_t* soft_start, Float_t step, Float_t start);

/**
 * @brief 复位软启动斜坡限制器
 *
 * @param soft_start 软启动结构体指针
 *
 * @note 当前值回到 param.start，重新开始爬升。
 */
void Soft_Start_Reset(Soft_Start_t* soft_start);

/**
 * @brief 更新软启动斜坡限制器状态
 *
 * @param soft_start 软启动结构体指针
 * @param target 目标值
 * @return Float_t 限速后的值
 *
 * @note 需以固定周期调用。
 */
Float_t Soft_Start_Update(Soft_Start_t* soft_start, Float_t target);

#endif

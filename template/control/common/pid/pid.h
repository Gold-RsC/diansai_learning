#ifndef __PID_H__
#define __PID_H__

#include "mbase.h"
#include <math.h>

typedef struct {
    struct {
        Float_t kp;
        Float_t ki;

        Float_t out_min;
        Float_t out_max;
    } param;

    Float_t out;

    struct {
        Float_t previous_error;
    } _state;
} PI_t;

/**
 * @brief 初始化 PID 控制器
 *
 * @param analyzer PID 控制器 结构体指针
 * @param kp Proportional Gain
 * @param ki Integral Gain
 * @param outmin 输出最小值
 * @param outmax 输出最大值
 */
void PI_Init(PI_t* analyzer, Float_t kp, Float_t ki, Float_t outmin, Float_t outmax);

/**
 * @brief 更新 PID 控制器
 *
 * @param analyzer PID 控制器 结构体指针
 * @param now 实际值
 * @param target 目标值
 * @return Float_t PID 输出值
 */
Float_t PI_Update(PI_t* analyzer, Float_t now, Float_t target);

/**
 * @brief 复位 PI 控制器
 *
 * @param analyzer PI 控制器 结构体指针
 *
 * @note 清零误差历史与输出。用于启动、模式切换、故障恢复，
 *       避免上一阶段遗留的 previous_error 造成首拍冲击。
 */
void PI_Reset(PI_t* analyzer);

/**
 * @brief 直接设定 PI 输出
 *
 * @param analyzer PI 控制器 结构体指针
 * @param output 目标输出值
 *
 * @note 供双环无扰切换使用：让闲置环的输出持续跟踪当前生效的输出，
 *       切换瞬间不产生跳变。写入值同样受 out_min / out_max 限幅。
 */
void PI_Set_Output(PI_t* analyzer, Float_t output);

#endif

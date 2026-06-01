#ifndef __PID_H__
#define __PID_H__

#include "mbase.h"
#include <math.h>

typedef struct {
    struct {
        float kp;
        float ki;

        float out_min;
        float out_max;
    } param;

    float out;

    struct {
        float previous_error;
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
void pi_init(PI_t* analyzer, float kp, float ki, float outmin, float outmax);

/**
 * @brief 更新 PID 控制器
 *
 * @param analyzer PID 控制器 结构体指针
 * @param now 实际值
 * @param target 目标值
 * @return float PID 输出值
 */
float pi_update(PI_t* analyzer, float now, float target);

#endif

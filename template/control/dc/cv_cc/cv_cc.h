#ifndef __CV_CC_H__
#define __CV_CC_H__

#include "mbase.h"
#include "pid.h"

/**
 * @brief CV/CC 模式
 */
typedef enum {
    CV_CC_MODE_CV,  // 恒压
    CV_CC_MODE_CC,  // 恒流
} CV_CC_Mode_t;

/**
 * @brief CV/CC 双环控制器
 *
 * @details 电压环与电流环各一个 PI，同时运行，按仲裁规则选用其中一个
 *          的输出作为占空比：
 *              电流超过 i_target        -> 切到恒流
 *              电流低于 i_target - 回差 -> 切回恒压
 *
 *          切换带回差，避免在切换点反复抖动。
 *
 *          无扰切换：闲置环每拍都用 PI_Set_Output 把自己的输出拉到当前
 *          生效的占空比，因此切换瞬间占空比不会跳变。这是本模块必须在
 *          PI 里补 Set_Output 原语的原因。
 *
 * @note 两个环的 PI 输出上下限都应设为占空比安全区，本模块最后还会再限一次。
 */
typedef struct {

    struct {
        Float_t switch_margin;  // 模式切换回差，单位：A

        Float_t out_min;  // 占空比安全下限
        Float_t out_max;  // 占空比安全上限
    } param;

    struct {
        PI_t voltage_loop;  // 电压环
        PI_t current_loop;  // 电流环

        CV_CC_Mode_t mode;  // 当前生效的模式
    } _state;

    Float_t out;  // 输出占空比

} CV_CC_t;

/**
 * @brief 初始化 CV/CC 双环控制器
 *
 * @param controller CV/CC 双环控制器结构体指针
 * @param kp_v 电压环比例系数
 * @param ki_v 电压环积分系数
 * @param kp_i 电流环比例系数
 * @param ki_i 电流环积分系数
 * @param switch_margin 模式切换回差，单位：A
 * @param out_min 占空比安全下限
 * @param out_max 占空比安全上限
 * @param init_value 初始占空比
 *
 * @note 两个 PI 的输出上下限都设为 [out_min, out_max]。
 */
void CV_CC_Init(CV_CC_t* controller,
                Float_t kp_v,
                Float_t ki_v,
                Float_t kp_i,
                Float_t ki_i,
                Float_t switch_margin,
                Float_t out_min,
                Float_t out_max,
                Float_t init_value);

/**
 * @brief 复位 CV/CC 双环控制器
 *
 * @param controller CV/CC 双环控制器结构体指针
 * @param init_value 初始占空比
 *
 * @note 返回恒压模式，两个 PI 状态清零。
 */
void CV_CC_Reset(CV_CC_t* controller, Float_t init_value);

/**
 * @brief 更新 CV/CC 双环控制器状态
 *
 * @param controller CV/CC 双环控制器结构体指针
 * @param v_out 实测输出电压，单位：V
 * @param i_out 实测输出电流，单位：A
 * @param v_target 恒压目标，单位：V
 * @param i_target 恒流目标，单位：A
 * @return Float_t 占空比
 */
Float_t CV_CC_Update(CV_CC_t* controller, Float_t v_out, Float_t i_out, Float_t v_target, Float_t i_target);


#endif

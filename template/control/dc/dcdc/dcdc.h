#ifndef __DCDC_H__
#define __DCDC_H__

#include "mbase.h"

/**
 * @brief DC-DC 拓扑类型
 */
typedef enum {
    DCDC_BUCK,        // 降压：Vo = D * Vin
    DCDC_BOOST,       // 升压：Vo = Vin / (1 - D)
    DCDC_BUCK_BOOST,  // 升降压：|Vo| = Vin * D / (1 - D)
} Dcdc_Topology_t;

/**
 * @brief DC-DC 拓扑模型
 *
 * @details 只提供拓扑的稳态关系、前馈占空比、增益与占空比安全区，
 *          **不含控制环路**。控制环路由 pid 与 cv_cc 组合完成，本模块
 *          与拓扑无关的部分不在这里。
 *
 *          前馈的作用：由输入电压与目标电压直接算出工作点占空比，
 *          PI 只需修正误差而不必从零把占空比积上去，负载突变恢复更快。
 *
 *          占空比安全区按拓扑给出：
 *              Buck        [0.02, 0.95]  受限于高边自举，上限可放宽
 *              Boost       [0.02, 0.90]  存在右半平面零点，带宽受限
 *              Buck-Boost  [0.02, 0.90]  同上
 *
 * @note Boost 与 Buck-Boost 存在右半平面零点，闭环带宽不应超过
 *       开关频率的 1/10，否则会出现先反向后正向的异常响应。
 */
typedef struct {

    struct {
        Dcdc_Topology_t topology;

        Float_t duty_min;  // 占空比安全下限
        Float_t duty_max;  // 占空比安全上限
    } param;

    Float_t out;  // 最近一次算出的前馈占空比

} Dcdc_t;

/**
 * @brief 初始化 DC-DC 拓扑模型
 *
 * @param dcdc DC-DC 拓扑模型结构体指针
 * @param topology 拓扑类型
 *
 * @note 占空比安全区按拓扑自动设定。
 */
void Dcdc_Init(Dcdc_t* dcdc, Dcdc_Topology_t topology);

/**
 * @brief 计算前馈占空比
 *
 * @param dcdc DC-DC 拓扑模型结构体指针
 * @param v_in 输入电压，单位：V
 * @param v_target 目标输出电压，单位：V
 * @return Float_t 前馈占空比，已按拓扑安全区限幅
 *
 * @note 结果同时写入 out。v_in 为 0 或公式无解时返回安全下限。
 *       Boost 要求 v_target > v_in，否则返回安全上限。
 */
Float_t Dcdc_Feedforward(Dcdc_t* dcdc, Float_t v_in, Float_t v_target);

/**
 * @brief 占空比安全限幅
 *
 * @param dcdc DC-DC 拓扑模型结构体指针
 * @param duty 待限幅的占空比
 * @return Float_t 限幅后的占空比
 */
Float_t Dcdc_Clamp_Duty(Dcdc_t* dcdc, Float_t duty);

/**
 * @brief 计算当前工作点的被控对象增益
 *
 * @param dcdc DC-DC 拓扑模型结构体指针
 * @param v_in 输入电压，单位：V
 * @param duty 当前占空比
 * @return Float_t 增益 dVo/dD
 *
 * @note 三个拓扑的增益分别为：
 *              Buck        Vin
 *              Boost       Vin / (1-D)^2
 *              Buck-Boost  Vin / (1-D)^2
 *       用它来按工作点调整 PI 参数：增益越大，kp 应越小。
 */
Float_t Dcdc_Plant_Gain(Dcdc_t* dcdc, Float_t v_in, Float_t duty);

#endif

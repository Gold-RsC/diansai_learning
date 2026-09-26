#include "cv_cc.h"

void Cv_Cc_Init(Cv_Cc_t* controller,
                Float_t v_target,
                Float_t i_target,
                Float_t kp_v,
                Float_t ki_v,
                Float_t kp_i,
                Float_t ki_i,
                Float_t duty_min,
                Float_t duty_max,
                Float_t switch_margin) {
    controller->param.v_target      = v_target;
    controller->param.i_target      = i_target;
    controller->param.switch_margin = switch_margin;
    controller->param.duty_min      = duty_min;
    controller->param.duty_max      = duty_max;

    PI_Init(&controller->_state.voltage_loop, kp_v, ki_v, duty_min, duty_max);
    PI_Init(&controller->_state.current_loop, kp_i, ki_i, duty_min, duty_max);

    Cv_Cc_Reset(controller);
}

void Cv_Cc_Reset(Cv_Cc_t* controller) {
    PI_Reset(&controller->_state.voltage_loop);
    PI_Reset(&controller->_state.current_loop);

    controller->_state.mode = CV_CC_MODE_CV;
    controller->out         = controller->param.duty_min;
}

Float_t Cv_Cc_Update(Cv_Cc_t* controller, Float_t v_out, Float_t i_out) {
    /* 两个环始终运行，闲置环的输出随后被拉到当前占空比上 */
    Float_t duty_v = PI_Update(&controller->_state.voltage_loop, v_out, controller->param.v_target);
    Float_t duty_i = PI_Update(&controller->_state.current_loop, i_out, controller->param.i_target);

    /* 模式仲裁，带回差 */
    if (controller->_state.mode == CV_CC_MODE_CV) {
        if (i_out > controller->param.i_target) {
            controller->_state.mode = CV_CC_MODE_CC;
        }
    }
    else {
        if (i_out < controller->param.i_target - controller->param.switch_margin) {
            controller->_state.mode = CV_CC_MODE_CV;
        }
    }

    /* 无扰切换：闲置环跟踪当前生效的占空比 */
    Float_t duty;

    if (controller->_state.mode == CV_CC_MODE_CC) {
        duty = duty_i;
        PI_Set_Output(&controller->_state.voltage_loop, duty_i);
    }
    else {
        duty = duty_v;
        PI_Set_Output(&controller->_state.current_loop, duty_v);
    }

    controller->out = clamp(duty, controller->param.duty_min, controller->param.duty_max);

    return controller->out;
}

Cv_Cc_Mode_t Cv_Cc_Get_Mode(Cv_Cc_t* controller) {
    return controller->_state.mode;
}

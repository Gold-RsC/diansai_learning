#include "cv_cc.h"

void CV_CC_Init(CV_CC_t* controller,
                Float_t kp_v,
                Float_t ki_v,
                Float_t kp_i,
                Float_t ki_i,
                Float_t switch_margin,
                Float_t out_min,
                Float_t out_max,
                Float_t init_value) {
    controller->param.switch_margin = switch_margin;
    controller->param.out_min       = out_min;
    controller->param.out_max       = out_max;
    controller->_state.mode         = CV_CC_MODE_CV;
    controller->out                 = init_value;

    PI_Init(&controller->_state.voltage_loop, kp_v, ki_v, out_min, out_max, init_value);
    PI_Init(&controller->_state.current_loop, kp_i, ki_i, out_min, out_max, init_value);

    CV_CC_Reset(controller, init_value);
}

void CV_CC_Reset(CV_CC_t* controller, Float_t init_value) {
    PI_Reset(&controller->_state.voltage_loop, init_value);
    PI_Reset(&controller->_state.current_loop, init_value);

    controller->_state.mode = CV_CC_MODE_CV;
    controller->out         = init_value;
}

Float_t CV_CC_Update(CV_CC_t* controller, Float_t v_out, Float_t i_out, Float_t v_target, Float_t i_target) {
    /* 两个环始终运行，闲置环的输出随后被拉到当前占空比上 */
    Float_t duty_v = PI_Update(&controller->_state.voltage_loop, v_out, v_target);
    Float_t duty_i = PI_Update(&controller->_state.current_loop, i_out, i_target);

    /* 模式仲裁，带回差 */
    if (controller->_state.mode == CV_CC_MODE_CV) {
        if (i_out > i_target) {
            controller->_state.mode = CV_CC_MODE_CC;
        }
    }
    else {
        if (i_out < i_target - controller->param.switch_margin) {
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

    controller->out = clamp(duty, controller->param.out_min, controller->param.out_max);

    return controller->out;
}

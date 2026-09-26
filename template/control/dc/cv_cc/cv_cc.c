#include "cv_cc.h"

void Cv_Cc_Init(Cv_Cc_t* cv_cc,
                Float_t v_target,
                Float_t i_target,
                Float_t kp_v,
                Float_t ki_v,
                Float_t kp_i,
                Float_t ki_i,
                Float_t duty_min,
                Float_t duty_max,
                Float_t switch_margin) {
    cv_cc->param.v_target      = v_target;
    cv_cc->param.i_target      = i_target;
    cv_cc->param.switch_margin = switch_margin;
    cv_cc->param.duty_min      = duty_min;
    cv_cc->param.duty_max      = duty_max;

    PI_Init(&cv_cc->_state.voltage_loop, kp_v, ki_v, duty_min, duty_max);
    PI_Init(&cv_cc->_state.current_loop, kp_i, ki_i, duty_min, duty_max);

    Cv_Cc_Reset(cv_cc);
}

void Cv_Cc_Reset(Cv_Cc_t* cv_cc) {
    PI_Reset(&cv_cc->_state.voltage_loop);
    PI_Reset(&cv_cc->_state.current_loop);

    cv_cc->_state.mode = CV_CC_MODE_CV;
    cv_cc->out         = cv_cc->param.duty_min;
}

Float_t Cv_Cc_Update(Cv_Cc_t* cv_cc, Float_t v_out, Float_t i_out) {
    /* 两个环始终运行，闲置环的输出随后被拉到当前占空比上 */
    Float_t duty_v = PI_Update(&cv_cc->_state.voltage_loop, v_out, cv_cc->param.v_target);
    Float_t duty_i = PI_Update(&cv_cc->_state.current_loop, i_out, cv_cc->param.i_target);

    /* 模式仲裁，带回差 */
    if (cv_cc->_state.mode == CV_CC_MODE_CV) {
        if (i_out > cv_cc->param.i_target) {
            cv_cc->_state.mode = CV_CC_MODE_CC;
        }
    }
    else {
        if (i_out < cv_cc->param.i_target - cv_cc->param.switch_margin) {
            cv_cc->_state.mode = CV_CC_MODE_CV;
        }
    }

    /* 无扰切换：闲置环跟踪当前生效的占空比 */
    Float_t duty;

    if (cv_cc->_state.mode == CV_CC_MODE_CC) {
        duty = duty_i;
        PI_Set_Output(&cv_cc->_state.voltage_loop, duty_i);
    }
    else {
        duty = duty_v;
        PI_Set_Output(&cv_cc->_state.current_loop, duty_v);
    }

    cv_cc->out = clamp(duty, cv_cc->param.duty_min, cv_cc->param.duty_max);

    return cv_cc->out;
}

Cv_Cc_Mode_t Cv_Cc_Get_Mode(Cv_Cc_t* cv_cc) {
    return cv_cc->_state.mode;
}

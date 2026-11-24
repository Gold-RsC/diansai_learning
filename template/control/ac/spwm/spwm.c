#include "spwm.h"


/* 调制比上限 */
#define SPWM_AMPLITUDE_MAX 1.0f

void SPWM_Init(SPWM_t* controller, Float_t pwm_freq, Float_t target_freq, Float_t amplitude) {
    memset(controller, 0, sizeof(*controller));
    controller->param.pwm_freq    = pwm_freq;
    controller->param.target_freq = target_freq;

    controller->_state.theta     = 0.0f;
    controller->_state.delta     = 2.0f * MATH_PI * target_freq / pwm_freq;
    controller->_state.amplitude = clamp(amplitude, 0.0f, SPWM_AMPLITUDE_MAX);

    controller->out.duty  = 0.5f;
    controller->out.theta = 0.0f;
}

void SPWM_Reset(SPWM_t* controller) {
    controller->_state.theta  = 0.0f;
    controller->out.duty  = 0.5f;
    controller->out.theta = 0.0f;
}

void SPWM_Set_Amplitude(SPWM_t* controller, Float_t amplitude) {
    controller->_state.amplitude = clamp(amplitude, 0.0f, SPWM_AMPLITUDE_MAX);
}

void SPWM_Set_Frequency(SPWM_t* controller, Float_t freq) {
    controller->param.target_freq = freq;
    controller->_state.delta      = 2.0f * MATH_PI * freq / controller->param.pwm_freq;
}

void SPWM_Set_Phase(SPWM_t* controller, Float_t theta) {
    theta = fmodf(theta, 2.0f * MATH_PI);
    if (theta < 0.0f) {
        theta += 2.0f * MATH_PI;
    }
    controller->_state.theta = theta;
}

Float_t SPWM_Update(SPWM_t* controller) {
    Float_t duty = 0.5f + 0.5f * controller->_state.amplitude * sinf(controller->_state.theta);

    controller->out.duty  = clamp(duty, SPWM_DUTY_MIN, SPWM_DUTY_MAX);
    controller->out.theta = controller->_state.theta;

    /* 相位推进并回绕，避免 fmodf 的额外开销（delta 必须小于 2π） */
    controller->_state.theta += controller->_state.delta;
    if (controller->_state.theta >= 2.0f * MATH_PI) {
        controller->_state.theta -= 2.0f * MATH_PI;
    }

    return controller->out.duty;
}

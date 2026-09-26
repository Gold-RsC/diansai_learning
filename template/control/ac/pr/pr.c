#include "pr.h"

void Pr_Init(Pr_t* controller,
             Float_t kp,
             Float_t kr,
             Float_t resonant_freq,
             Float_t bandwidth,
             Float_t sample_freq,
             Float_t out_min,
             Float_t out_max) {
    controller->param.kp = kp;
    controller->param.kr = kr;

    controller->param.w0 = 2.0f * MATH_PI * resonant_freq;
    controller->param.wc = 2.0f * MATH_PI * bandwidth;

    controller->param.out_min = out_min;
    controller->param.out_max = out_max;

    /* Tustin 离散化，系数一次算好 */
    Float_t k  = 2.0f * sample_freq;
    Float_t a0 = k * k + 2.0f * controller->param.wc * k + controller->param.w0 * controller->param.w0;
    Float_t inv_a0 = 1.0f / a0;
    Float_t b0 = 2.0f * kr * controller->param.wc * k * inv_a0;

    controller->_state.b0 = b0;
    controller->_state.b2 = -b0;

    controller->_state.a1 = 2.0f * (controller->param.w0 * controller->param.w0 - k * k) * inv_a0;
    controller->_state.a2 = (k * k - 2.0f * controller->param.wc * k + controller->param.w0 * controller->param.w0) * inv_a0;

    Pr_Reset(controller);
}

void Pr_Reset(Pr_t* controller) {
    controller->_state.e1 = 0.0f;
    controller->_state.e2 = 0.0f;

    controller->_state.r1 = 0.0f;
    controller->_state.r2 = 0.0f;

    controller->out = 0.0f;
}

Float_t Pr_Update(Pr_t* controller, Float_t error) {
    /* 谐振通路 r[n] = b0*e[n] + b2*e[n-2] - a1*r[n-1] - a2*r[n-2] */
    Float_t resonance = controller->_state.b0 * error + controller->_state.b2 * controller->_state.e2
                        - controller->_state.a1 * controller->_state.r1 - controller->_state.a2 * controller->_state.r2;

    controller->_state.e2 = controller->_state.e1;
    controller->_state.e1 = error;

    controller->_state.r2 = controller->_state.r1;
    controller->_state.r1 = resonance;

    controller->out = clamp(controller->param.kp * error + resonance, controller->param.out_min, controller->param.out_max);

    return controller->out;
}

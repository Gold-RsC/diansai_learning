#include "pr.h"

void Pr_Init(Pr_t* pr,
             Float_t kp,
             Float_t kr,
             Float_t resonant_freq,
             Float_t bandwidth,
             Float_t sample_freq,
             Float_t out_min,
             Float_t out_max) {
    pr->param.kp = kp;
    pr->param.kr = kr;

    pr->param.w0 = 2.0f * MATH_PI * resonant_freq;
    pr->param.wc = 2.0f * MATH_PI * bandwidth;

    pr->param.out_min = out_min;
    pr->param.out_max = out_max;

    /* Tustin 离散化，系数一次算好 */
    Float_t k  = 2.0f * sample_freq;
    Float_t a0 = k * k + 2.0f * pr->param.wc * k + pr->param.w0 * pr->param.w0;
    Float_t inv_a0 = 1.0f / a0;
    Float_t b0 = 2.0f * kr * pr->param.wc * k * inv_a0;

    pr->_state.b0 = b0;
    pr->_state.b2 = -b0;

    pr->_state.a1 = 2.0f * (pr->param.w0 * pr->param.w0 - k * k) * inv_a0;
    pr->_state.a2 = (k * k - 2.0f * pr->param.wc * k + pr->param.w0 * pr->param.w0) * inv_a0;

    Pr_Reset(pr);
}

void Pr_Reset(Pr_t* pr) {
    pr->_state.e1 = 0.0f;
    pr->_state.e2 = 0.0f;

    pr->_state.r1 = 0.0f;
    pr->_state.r2 = 0.0f;

    pr->out = 0.0f;
}

Float_t Pr_Update(Pr_t* pr, Float_t error) {
    /* 谐振通路 r[n] = b0*e[n] + b2*e[n-2] - a1*r[n-1] - a2*r[n-2] */
    Float_t resonance = pr->_state.b0 * error + pr->_state.b2 * pr->_state.e2
                        - pr->_state.a1 * pr->_state.r1 - pr->_state.a2 * pr->_state.r2;

    pr->_state.e2 = pr->_state.e1;
    pr->_state.e1 = error;

    pr->_state.r2 = pr->_state.r1;
    pr->_state.r1 = resonance;

    pr->out = clamp(pr->param.kp * error + resonance, pr->param.out_min, pr->param.out_max);

    return pr->out;
}

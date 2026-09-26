#include "goertzel.h"

void Goertzel_Init(Goertzel_t* goertzel, Float_t sample_freq, Float_t target_freq, uint32_t window_size) {
    goertzel->param.sample_freq = sample_freq;
    goertzel->param.target_freq = target_freq;
    goertzel->param.window_size = window_size;

    goertzel->_state.w     = 2.0f * MATH_PI * target_freq / sample_freq;
    goertzel->_state.coeff = 2.0f * cosf(goertzel->_state.w);

    Goertzel_Reset(goertzel);
}

void Goertzel_Reset(Goertzel_t* goertzel) {
    goertzel->_state.s1    = 0.0f;
    goertzel->_state.s2    = 0.0f;
    goertzel->_state.count = 0;

    goertzel->out.magnitude = 0.0f;
    goertzel->out.phase     = 0.0f;
    goertzel->out.ready     = false;
}

Float_t Goertzel_Update(Goertzel_t* goertzel, Float_t sample) {
    goertzel->out.ready = false;

    Float_t s0 = sample + goertzel->_state.coeff * goertzel->_state.s1 - goertzel->_state.s2;

    goertzel->_state.s2 = goertzel->_state.s1;
    goertzel->_state.s1 = s0;

    goertzel->_state.count++;

    if (goertzel->_state.count >= goertzel->param.window_size) {
        // X = s[N-1] * e^(jw) - s[N-2]
        // 标准写法 s[N-1] - e^(-jw)*s[N-2] 得到的相位比真实谱相位迟 w，此处已修正
        Float_t real = goertzel->_state.s1 * cosf(goertzel->_state.w) - goertzel->_state.s2;
        Float_t imag = goertzel->_state.s1 * sinf(goertzel->_state.w);

        goertzel->out.magnitude = 2.0f * sqrtf(real * real + imag * imag) / (Float_t)goertzel->param.window_size;
        goertzel->out.phase     = atan2f(imag, real);
        goertzel->out.ready     = true;

        goertzel->_state.s1    = 0.0f;
        goertzel->_state.s2    = 0.0f;
        goertzel->_state.count = 0;
    }

    return goertzel->out.magnitude;
}

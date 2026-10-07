#include "droop.h"

void Droop_Init(Droop_t* controller,
                Float_t freq_noload,
                Float_t amp_noload,
                Float_t m,
                Float_t n,
                Float_t filter_cutoff,
                Float_t sample_freq) {
    controller->param.freq_noload = freq_noload;
    controller->param.amp_noload  = amp_noload;
    controller->param.m            = m;
    controller->param.n            = n;

    controller->param.filter_cutoff = filter_cutoff;
    controller->param.sample_freq   = sample_freq;

    /* 默认限幅取下垂线的完整量程（频率/幅值最低降到 0），
     * 不加人为的百分比限制，避免实测时电压还没掉到位就被截断。
     * 需要按变流器容量收紧时，初始化后直接改 param。 */
    controller->param.p_max = (m > 0.0f) ? (freq_noload / m) : 0.0f;
    controller->param.q_max = (n > 0.0f) ? (amp_noload / n) : 0.0f;

    Lpf_Filter_Init(&controller->_state.lpf_p, sample_freq, filter_cutoff, -1e9f, 1e9f, 0.0f);
    Lpf_Filter_Init(&controller->_state.lpf_q, sample_freq, filter_cutoff, -1e9f, 1e9f, 0.0f);

    Droop_Reset(controller);
}

void Droop_Reset(Droop_t* controller) {
    Lpf_Filter_Init(&controller->_state.lpf_p,
                    controller->param.sample_freq,
                    controller->param.filter_cutoff,
                    -1e9f,
                    1e9f,
                    0.0f);
    Lpf_Filter_Init(&controller->_state.lpf_q,
                    controller->param.sample_freq,
                    controller->param.filter_cutoff,
                    -1e9f,
                    1e9f,
                    0.0f);

    controller->out.freq      = controller->param.freq_noload;
    controller->out.amplitude = controller->param.amp_noload;
}

void Droop_Update(Droop_t* controller, Float_t p, Float_t q) {
    Float_t p_filt = Lpf_Filter_Update(&controller->_state.lpf_p, p);
    Float_t q_filt = Lpf_Filter_Update(&controller->_state.lpf_q, q);

    p_filt = clamp(p_filt, -controller->param.p_max, controller->param.p_max);
    q_filt = clamp(q_filt, -controller->param.q_max, controller->param.q_max);

    controller->out.freq      = controller->param.freq_noload - controller->param.m * p_filt;
    controller->out.amplitude = controller->param.amp_noload - controller->param.n * q_filt;
}

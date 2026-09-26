#include "droop.h"

void Droop_Init(Droop_t* droop,
                Float_t freq_noload,
                Float_t amp_noload,
                Float_t m,
                Float_t n,
                Float_t filter_cutoff,
                Float_t sample_freq) {
    droop->param.freq_noload = freq_noload;
    droop->param.amp_noload  = amp_noload;
    droop->param.m            = m;
    droop->param.n            = n;

    droop->param.filter_cutoff = filter_cutoff;
    droop->param.sample_freq   = sample_freq;

    /* 默认限幅取下垂线的完整量程（频率/幅值最低降到 0），
     * 不加人为的百分比限制，避免实测时电压还没掉到位就被截断。
     * 需要按变流器容量收紧时，初始化后直接改 param。 */
    droop->param.p_max = (m > 0.0f) ? (freq_noload / m) : 0.0f;
    droop->param.q_max = (n > 0.0f) ? (amp_noload / n) : 0.0f;

    Lpf_Filter_Init(&droop->_state.lpf_p, sample_freq, filter_cutoff, -1e9f, 1e9f, 0.0f);
    Lpf_Filter_Init(&droop->_state.lpf_q, sample_freq, filter_cutoff, -1e9f, 1e9f, 0.0f);

    Droop_Reset(droop);
}

void Droop_Reset(Droop_t* droop) {
    Lpf_Filter_Init(&droop->_state.lpf_p,
                    droop->param.sample_freq,
                    droop->param.filter_cutoff,
                    -1e9f,
                    1e9f,
                    0.0f);
    Lpf_Filter_Init(&droop->_state.lpf_q,
                    droop->param.sample_freq,
                    droop->param.filter_cutoff,
                    -1e9f,
                    1e9f,
                    0.0f);

    droop->out.freq      = droop->param.freq_noload;
    droop->out.amplitude = droop->param.amp_noload;
}

void Droop_Update(Droop_t* droop, Float_t p, Float_t q) {
    Float_t p_filt = Lpf_Filter_Update(&droop->_state.lpf_p, p);
    Float_t q_filt = Lpf_Filter_Update(&droop->_state.lpf_q, q);

    p_filt = clamp(p_filt, -droop->param.p_max, droop->param.p_max);
    q_filt = clamp(q_filt, -droop->param.q_max, droop->param.q_max);

    droop->out.freq      = droop->param.freq_noload - droop->param.m * p_filt;
    droop->out.amplitude = droop->param.amp_noload - droop->param.n * q_filt;
}

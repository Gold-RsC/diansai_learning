#include "thd.h"

void Thd_Init(Thd_t* analyzer, Float_t sample_freq, Float_t fundamental_freq, uint8_t max_order) {
    analyzer->param.sample_freq      = sample_freq;
    analyzer->param.fundamental_freq = fundamental_freq;
    analyzer->param.window_size      = (uint32_t)(sample_freq / fundamental_freq);

    analyzer->param.max_order = (max_order > THD_MAX_ORDER) ? THD_MAX_ORDER : max_order;

    for (uint8_t h = 1; h <= THD_MAX_ORDER; h++) {
        Goertzel_Init(&analyzer->_state.gz[h], sample_freq, fundamental_freq * (Float_t)h, analyzer->param.window_size);
    }

    Thd_Reset(analyzer);
}

void Thd_Reset(Thd_t* analyzer) {
    for (uint8_t h = 1; h <= THD_MAX_ORDER; h++) {
        Goertzel_Reset(&analyzer->_state.gz[h]);
    }

    analyzer->out.fundamental = 0.0f;
    analyzer->out.thd         = 0.0f;
    analyzer->out.ready       = false;
}

Float_t Thd_Update(Thd_t* analyzer, Float_t sample) {
    analyzer->out.ready = false;

    for (uint8_t h = 1; h <= analyzer->param.max_order; h++) {
        Goertzel_Update(&analyzer->_state.gz[h], sample);
    }

    if (analyzer->_state.gz[1].out.ready) {
        Float_t sum_square = 0.0f;

        for (uint8_t h = 2; h <= analyzer->param.max_order; h++) {
            sum_square += analyzer->_state.gz[h].out.magnitude * analyzer->_state.gz[h].out.magnitude;
        }

        analyzer->out.fundamental = analyzer->_state.gz[1].out.magnitude;
        analyzer->out.thd = (analyzer->out.fundamental > 1e-9f) ? sqrtf(sum_square) / analyzer->out.fundamental : 0.0f;

        analyzer->out.ready = true;
    }

    return analyzer->out.thd;
}

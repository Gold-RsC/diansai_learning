#include "thd.h"

void Thd_Init(Thd_t* thd, Float_t sample_freq, Float_t fundamental_freq, uint8_t max_order) {
    thd->param.sample_freq      = sample_freq;
    thd->param.fundamental_freq = fundamental_freq;
    thd->param.window_size      = (uint32_t)(sample_freq / fundamental_freq);

    thd->param.max_order = (max_order > THD_MAX_ORDER) ? THD_MAX_ORDER : max_order;

    for (uint8_t h = 1; h <= THD_MAX_ORDER; h++) {
        Goertzel_Init(&thd->_state.gz[h], sample_freq, fundamental_freq * (Float_t)h, thd->param.window_size);
    }

    Thd_Reset(thd);
}

void Thd_Reset(Thd_t* thd) {
    for (uint8_t h = 1; h <= THD_MAX_ORDER; h++) {
        Goertzel_Reset(&thd->_state.gz[h]);
    }

    thd->out.fundamental = 0.0f;
    thd->out.thd         = 0.0f;
    thd->out.ready       = false;
}

Float_t Thd_Update(Thd_t* thd, Float_t sample) {
    thd->out.ready = false;

    for (uint8_t h = 1; h <= thd->param.max_order; h++) {
        Goertzel_Update(&thd->_state.gz[h], sample);
    }

    if (thd->_state.gz[1].out.ready) {
        Float_t sum_square = 0.0f;

        for (uint8_t h = 2; h <= thd->param.max_order; h++) {
            sum_square += thd->_state.gz[h].out.magnitude * thd->_state.gz[h].out.magnitude;
        }

        thd->out.fundamental = thd->_state.gz[1].out.magnitude;
        thd->out.thd = (thd->out.fundamental > 1e-9f) ? sqrtf(sum_square) / thd->out.fundamental : 0.0f;

        thd->out.ready = true;
    }

    return thd->out.thd;
}

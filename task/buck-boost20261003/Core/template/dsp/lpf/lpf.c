#include "lpf.h"

void Lpf_Filter_Init(Lpf_Filter_t* lpf,
                     Float_t sample_freq,
                     Float_t cutoff_freq,
                     Float_t out_min,
                     Float_t out_max,
                     Float_t initial_value) {
    lpf->param.sample_freq = sample_freq;
    lpf->param.cutoff_freq = cutoff_freq;
    lpf->param.out_min     = out_min;
    lpf->param.out_max     = out_max;

    lpf->_state.alpha = 1.0f - expf(-2.0f * MATH_PI * cutoff_freq / sample_freq);
    lpf->_state.y1    = initial_value;

    lpf->out = initial_value;
}

Float_t Lpf_Filter_Update(Lpf_Filter_t* lpf, Float_t input) {
    lpf->_state.y1 += lpf->_state.alpha * (input - lpf->_state.y1);

    lpf->out = clamp(lpf->_state.y1, lpf->param.out_min, lpf->param.out_max);

    return lpf->out;
}

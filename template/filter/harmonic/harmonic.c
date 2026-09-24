#include "harmonic.h"

void Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                          Float_t gain_2,
                          Float_t gain_3,
                          Float_t gain_5,
                          Float_t out_min,
                          Float_t out_max,
                          Float_t initial_value) {

    filter->param.gain_2  = gain_2;
    filter->param.gain_3  = gain_3;
    filter->param.gain_5  = gain_5;
    filter->param.out_min = out_min;
    filter->param.out_max = out_max;

    Sliding_Filter_Init(&filter->_state.sliding_filter, out_min, out_max, initial_value);

    filter->out = initial_value;
}


Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement) {

    Sliding_Filter_Update(&filter->_state.sliding_filter, measurement);

    Float_t avg = filter->_state.sliding_filter.out;

    Float_t harmonic_comp = 0.0f;
    harmonic_comp += (filter->_state.sliding_filter._state.window[0] - avg) * filter->param.gain_2;
    harmonic_comp += (filter->_state.sliding_filter._state.window[1] - avg) * filter->param.gain_3;
    harmonic_comp += (filter->_state.sliding_filter._state.window[4] - avg) * filter->param.gain_5;

    filter->out = avg - harmonic_comp * 0.5f;

    return filter->out;
}

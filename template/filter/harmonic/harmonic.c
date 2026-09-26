#include "harmonic.h"

void Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                          Float_t gain_2,
                          Float_t gain_3,
                          Float_t gain_5,
                          Float_t sample_freq,
                          Float_t grid_freq,
                          Float_t out_min,
                          Float_t out_max,
                          Float_t initial_value) {

    filter->param.gain_2      = gain_2;
    filter->param.gain_3      = gain_3;
    filter->param.gain_5      = gain_5;
    filter->param.sample_freq = sample_freq;
    filter->param.grid_freq   = grid_freq;
    filter->param.out_min     = out_min;
    filter->param.out_max     = out_max;

    Notch_Filter_Init(&filter->_state.notch_2, 2.0f * grid_freq, sample_freq, HARMONIC_FILTER_Q, initial_value);
    Notch_Filter_Init(&filter->_state.notch_3, 3.0f * grid_freq, sample_freq, HARMONIC_FILTER_Q, initial_value);
    Notch_Filter_Init(&filter->_state.notch_5, 5.0f * grid_freq, sample_freq, HARMONIC_FILTER_Q, initial_value);

    filter->out = initial_value;
}

Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement) {

    Float_t y = measurement;

    y -= filter->param.gain_2 * (y - Notch_Filter_Update(&filter->_state.notch_2, y));
    y -= filter->param.gain_3 * (y - Notch_Filter_Update(&filter->_state.notch_3, y));
    y -= filter->param.gain_5 * (y - Notch_Filter_Update(&filter->_state.notch_5, y));

    filter->out = clamp(y, filter->param.out_min, filter->param.out_max);

    return filter->out;
}

#include "filter.h"

void Filter_Init(Filter_t* filter,
                 Float_t Q,
                 Float_t R,
                 Float_t gain_2,
                 Float_t gain_3,
                 Float_t gain_5,
                 Float_t min,
                 Float_t max,
                 Float_t initial_value) {
    Kalman_Filter_Init(&filter->_state.kalman_filter, Q, R, min, max, initial_value);
    Harmonic_Filter_Init(&filter->_state.harmonic_filter, gain_2, gain_3, gain_5, min, max, initial_value);
}

Float_t Filter_Update(Filter_t* filter, Float_t measurement) {
    filter->out = Harmonic_Filter_Update(&filter->_state.harmonic_filter,
                                         Kalman_Filter_Update(&filter->_state.kalman_filter, measurement));

    return filter->out;
}

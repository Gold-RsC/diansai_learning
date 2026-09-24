#include "sliding.h"

void Sliding_Filter_Init(Sliding_Filter_t* filter, Float_t out_min, Float_t out_max, Float_t initial_value) {
    filter->param.out_min = out_min;
    filter->param.out_max = out_max;
    filter->out           = initial_value;

    for (size_t i = 0; i < SLIDING_FILTER_WINDOW_SIZE; ++i) {
        filter->_state.window[i] = initial_value;
    }
    filter->_state.idx = 0;
}

Float_t Sliding_Filter_Update(Sliding_Filter_t* filter, Float_t measurement) {

    filter->out += (measurement - filter->_state.window[filter->_state.idx]) / SLIDING_FILTER_WINDOW_SIZE;

    filter->_state.idx = (filter->_state.idx + 1) % SLIDING_FILTER_WINDOW_SIZE;

    filter->out = clamp(filter->out, filter->param.out_min, filter->param.out_max);

    return filter->out;
}

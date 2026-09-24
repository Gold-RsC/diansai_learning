#include "kalman.h"

void Kalman_Filter_Init(
    Kalman_Filter_t* filter, Float_t Q, Float_t R, Float_t out_min, Float_t out_max, Float_t initial_value) {
    filter->param.Q = Q;
    filter->param.R = R;

    filter->_state.P = 1.0;

    filter->out = initial_value;

    filter->param.out_min = out_min;
    filter->param.out_max = out_max;
}

Float_t Kalman_Filter_Update(Kalman_Filter_t* filter, Float_t measurement) {
    filter->_state.P = filter->_state.P + filter->param.Q;
    Float_t K        = filter->_state.P / (filter->_state.P + filter->param.R);
    filter->out      = filter->out + K * (measurement - filter->out);
    filter->_state.P = (1.0 - K) * filter->_state.P;

    filter->out = clamp(filter->out, filter->param.out_min, filter->param.out_max);

    return filter->out;
}

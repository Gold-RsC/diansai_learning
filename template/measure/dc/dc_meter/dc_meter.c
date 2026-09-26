#include "dc_meter.h"

void Dc_Meter_Init(Dc_Meter_t* meter, uint32_t window_size) {
    meter->param.window_size = window_size;

    Dc_Meter_Reset(meter);
}

void Dc_Meter_Reset(Dc_Meter_t* meter) {
    meter->_state.sum       = 0.0f;
    meter->_state.min_value = 0.0f;
    meter->_state.max_value = 0.0f;
    meter->_state.count     = 0;

    meter->out.average = 0.0f;
    meter->out.ripple  = 0.0f;
    meter->out.ready   = false;
}

Float_t Dc_Meter_Update(Dc_Meter_t* meter, Float_t sample) {
    meter->out.ready = false;

    if (meter->_state.count == 0) {
        meter->_state.min_value = sample;
        meter->_state.max_value = sample;
    }
    else {
        if (sample < meter->_state.min_value) meter->_state.min_value = sample;
        if (sample > meter->_state.max_value) meter->_state.max_value = sample;
    }

    meter->_state.sum += sample;
    meter->_state.count++;

    if (meter->_state.count >= meter->param.window_size) {
        meter->out.average = meter->_state.sum / (Float_t)meter->param.window_size;
        meter->out.ripple  = meter->_state.max_value - meter->_state.min_value;
        meter->out.ready   = true;

        meter->_state.sum       = 0.0f;
        meter->_state.min_value = 0.0f;
        meter->_state.max_value = 0.0f;
        meter->_state.count     = 0;
    }

    return meter->out.average;
}

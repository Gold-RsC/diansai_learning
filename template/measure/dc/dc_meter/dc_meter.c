#include "dc_meter.h"

void Dc_Meter_Init(Dc_Meter_t* analyzer, uint32_t window_size) {
    analyzer->param.window_size = window_size;

    Dc_Meter_Reset(analyzer);
}

void Dc_Meter_Reset(Dc_Meter_t* analyzer) {
    analyzer->_state.sum       = 0.0f;
    analyzer->_state.min_value = 0.0f;
    analyzer->_state.max_value = 0.0f;
    analyzer->_state.count     = 0;

    analyzer->out.average = 0.0f;
    analyzer->out.ripple  = 0.0f;
    analyzer->out.ready   = false;
}

Float_t Dc_Meter_Update(Dc_Meter_t* analyzer, Float_t sample) {
    analyzer->out.ready = false;

    if (analyzer->_state.count == 0) {
        analyzer->_state.min_value = sample;
        analyzer->_state.max_value = sample;
    }
    else {
        if (sample < analyzer->_state.min_value) analyzer->_state.min_value = sample;
        if (sample > analyzer->_state.max_value) analyzer->_state.max_value = sample;
    }

    analyzer->_state.sum += sample;
    analyzer->_state.count++;

    if (analyzer->_state.count >= analyzer->param.window_size) {
        analyzer->out.average = analyzer->_state.sum / (Float_t)analyzer->param.window_size;
        analyzer->out.ripple  = analyzer->_state.max_value - analyzer->_state.min_value;
        analyzer->out.ready   = true;

        analyzer->_state.sum       = 0.0f;
        analyzer->_state.min_value = 0.0f;
        analyzer->_state.max_value = 0.0f;
        analyzer->_state.count     = 0;
    }

    return analyzer->out.average;
}

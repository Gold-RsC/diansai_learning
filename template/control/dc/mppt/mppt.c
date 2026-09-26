#include "mppt.h"

void Mppt_Init(Mppt_t* mppt, Float_t step, Float_t min_duty, Float_t max_duty, uint32_t period) {
    mppt->param.step     = step;
    mppt->param.min_duty = min_duty;
    mppt->param.max_duty = max_duty;
    mppt->param.period   = period;

    Mppt_Reset(mppt);
}

void Mppt_Reset(Mppt_t* mppt) {
    mppt->_state.duty       = mppt->param.min_duty;
    mppt->_state.direction  = 1.0f;
    mppt->_state.prev_power = 0.0f;
    mppt->_state.count      = 0;

    mppt->out = mppt->param.min_duty;
}

Float_t Mppt_Update(Mppt_t* mppt, Float_t v_in, Float_t i_in) {
    Float_t power = v_in * i_in;

    mppt->_state.count++;

    if (mppt->_state.count >= mppt->param.period) {
        mppt->_state.count = 0;

        /* 功率下降则反向扰动 */
        if (power < mppt->_state.prev_power) {
            mppt->_state.direction = -mppt->_state.direction;
        }

        mppt->_state.duty += mppt->_state.direction * mppt->param.step;
        mppt->_state.duty = clamp(mppt->_state.duty, mppt->param.min_duty, mppt->param.max_duty);

        mppt->_state.prev_power = power;
    }

    mppt->out = mppt->_state.duty;

    return mppt->out;
}

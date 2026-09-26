#include "soft_start.h"

void Soft_Start_Init(Soft_Start_t* soft_start, Float_t step, Float_t start) {
    soft_start->param.step  = step;
    soft_start->param.start = start;

    Soft_Start_Reset(soft_start);
}

void Soft_Start_Reset(Soft_Start_t* soft_start) {
    soft_start->_state.value = soft_start->param.start;
    soft_start->out          = soft_start->param.start;
}

Float_t Soft_Start_Update(Soft_Start_t* soft_start, Float_t target) {
    Float_t delta = target - soft_start->_state.value;

    if (delta > soft_start->param.step) {
        soft_start->_state.value += soft_start->param.step;
    }
    else if (delta < -soft_start->param.step) {
        soft_start->_state.value -= soft_start->param.step;
    }
    else {
        soft_start->_state.value = target;
    }

    soft_start->out = soft_start->_state.value;

    return soft_start->out;
}

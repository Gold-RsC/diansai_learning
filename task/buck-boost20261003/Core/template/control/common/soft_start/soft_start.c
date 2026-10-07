#include "soft_start.h"

void Soft_Start_Init(Soft_Start_t* controller, Float_t step, Float_t start) {
    controller->param.step  = step;
    controller->param.start = start;

    Soft_Start_Reset(controller);
}

void Soft_Start_Reset(Soft_Start_t* controller) {
    controller->_state.value = controller->param.start;
    controller->out          = controller->param.start;
}

Float_t Soft_Start_Update(Soft_Start_t* controller, Float_t target) {
    Float_t delta = target - controller->_state.value;

    if (delta > controller->param.step) {
        controller->_state.value += controller->param.step;
    }
    else if (delta < -controller->param.step) {
        controller->_state.value -= controller->param.step;
    }
    else {
        controller->_state.value = target;
    }

    controller->out = controller->_state.value;

    return controller->out;
}

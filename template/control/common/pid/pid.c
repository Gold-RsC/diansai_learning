#include "pid.h"

void PI_Init(PI_t* controller, Float_t kp, Float_t ki, Float_t outmin, Float_t outmax) {
    controller->param.kp      = kp;
    controller->param.ki      = ki;
    controller->param.out_min = outmin;
    controller->param.out_max = outmax;
}

Float_t PI_Update(PI_t* controller, Float_t now, Float_t target) {

    Float_t error     = target - now;
    Float_t delta_out = controller->param.kp * (error - controller->_state.previous_error)  // p
                      + controller->param.ki * error;                                   // i

    controller->_state.previous_error = error;
    controller->out += delta_out;
    controller->out = clamp(controller->out, controller->param.out_min, controller->param.out_max);
    return controller->out;
}

void PI_Reset(PI_t* controller) {
    controller->_state.previous_error = 0.0f;
    controller->out                   = 0.0f;
}

void PI_Set_Output(PI_t* controller, Float_t output) {
    controller->out = clamp(output, controller->param.out_min, controller->param.out_max);
}

#include "pid.h"

void PI_Init(PI_t* analyzer, Float_t kp, Float_t ki, Float_t outmin, Float_t outmax) {
    analyzer->param.kp      = kp;
    analyzer->param.ki      = ki;
    analyzer->param.out_min = outmin;
    analyzer->param.out_max = outmax;
}

Float_t PI_Update(PI_t* analyzer, Float_t now, Float_t target) {

    Float_t error     = target - now;
    Float_t delta_out = analyzer->param.kp * (error - analyzer->_state.previous_error)  // p
                      + analyzer->param.ki * error;                                   // i

    analyzer->_state.previous_error = error;
    analyzer->out += delta_out;
    analyzer->out = clamp(analyzer->out, analyzer->param.out_min, analyzer->param.out_max);
    return analyzer->out;
}

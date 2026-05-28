#include "pid.h"

void pi_init(PI_t* analyzer, float kp, float ki, float outmin, float outmax) {
    analyzer->param.kp     = kp;
    analyzer->param.ki     = ki;
    analyzer->param.outmin = outmin;
    analyzer->param.outmax = outmax;
}

float pi_update(PI_t* analyzer, float now, float target) {

    float error     = target - now;
    float delta_out = analyzer->param.kp * (error - analyzer->_state.previous_error)  // p
                      + analyzer->param.ki * error;                                   // i

    analyzer->_state.previous_error = error;
    analyzer->out += delta_out;
    analyzer->out = clamp(analyzer->out, analyzer->param.outmin, analyzer->param.outmax);
    return analyzer->out;
}
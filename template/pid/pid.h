#ifndef __PID_H__
#define __PID_H__

#include "mbase.h"
#include <math.h>

typedef struct {
    struct {
        float kp;
        float ki;

        float outmin;
        float outmax;
    } param;

    float out;

    struct {
        float previous_error;
    } _state;
} PI_t;

void pi_init(PI_t* analyzer, float kp, float ki, float outmin, float outmax);
float pi_update(PI_t* analyzer, float now, float target);

#endif

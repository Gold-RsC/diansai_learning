#include "dcdc.h"

void Dcdc_Init(Dcdc_t* controller, Dcdc_Topology_t topology) {
    controller->param.topology = topology;
    controller->param.out_min  = 0.02f;

    /* Buck 无右半平面零点，上限可放宽；Boost 与 Buck-Boost 有，需留余量 */
    controller->param.out_max = (topology == DCDC_BUCK) ? 0.95f : 0.90f;

    controller->out = controller->param.out_min;
}

Float_t Dcdc_Feedforward(Dcdc_t* controller, Float_t v_in, Float_t v_target) {
    Float_t duty;

    if (v_in <= 0.0f) {
        controller->out = controller->param.out_min;
        return controller->out;
    }

    switch (controller->param.topology) {
        case DCDC_BUCK:
            /* Vo = D * Vin  ->  D = Vo / Vin */
            duty = v_target / v_in;
            break;

        case DCDC_BOOST:
            /* Vo = Vin / (1 - D)  ->  D = 1 - Vin / Vo */
            duty = 1.0f - v_in / v_target;
            break;

        case DCDC_BUCK_BOOST:
            /* |Vo| = Vin * D / (1 - D)  ->  D = Vo / (Vin + Vo) */
            duty = v_target / (v_in + v_target);
            break;

        default:
            duty = controller->param.out_min;
            break;
    }

    controller->out = Dcdc_Clamp_Duty(controller, duty);

    return controller->out;
}

Float_t Dcdc_Clamp_Duty(Dcdc_t* controller, Float_t duty) {
    return clamp(duty, controller->param.out_min, controller->param.out_max);
}

Float_t Dcdc_Plant_Gain(Dcdc_t* controller, Float_t v_in, Float_t duty) {
    if (controller->param.topology == DCDC_BUCK) {
        return v_in;
    }

    /* Boost 与 Buck-Boost 的增益同为 Vin / (1-D)^2 */
    Float_t one_minus_d = 1.0f - duty;

    if (one_minus_d < 1e-4f) {
        one_minus_d = 1e-4f;
    }

    return v_in / (one_minus_d * one_minus_d);
}

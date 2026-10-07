#include "mppt.h"

void MPPT_Init(MPPT_t* controller, Float_t step, Float_t min_duty, Float_t max_duty, uint32_t period) {
    controller->param.step     = step;
    controller->param.min_duty = min_duty;
    controller->param.max_duty = max_duty;
    controller->param.period   = period;

    MPPT_Reset(controller);
}

void MPPT_Reset(MPPT_t* controller) {
    controller->_state.duty       = controller->param.min_duty;
    controller->_state.direction  = 1.0f;
    controller->_state.prev_power = 0.0f;
    controller->_state.count      = 0;

    controller->out = controller->param.min_duty;
}

Float_t MPPT_Update(MPPT_t* controller, Float_t v_in, Float_t i_in) {
    Float_t power = v_in * i_in;

    controller->_state.count++;

    if (controller->_state.count >= controller->param.period) {
        controller->_state.count = 0;

        /* 功率下降则反向扰动 */
        if (power < controller->_state.prev_power) {
            controller->_state.direction = -controller->_state.direction;
        }

        controller->_state.duty += controller->_state.direction * controller->param.step;
        controller->_state.duty =
            clamp(controller->_state.duty, controller->param.min_duty, controller->param.max_duty);

        controller->_state.prev_power = power;
    }

    controller->out = controller->_state.duty;

    return controller->out;
}

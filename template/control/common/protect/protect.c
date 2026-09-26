#include "protect.h"

void Protect_Init(Protect_t* controller,
                  Float_t over_threshold,
                  Float_t under_threshold,
                  Float_t hysteresis,
                  uint16_t trip_count) {
    controller->param.over_threshold  = over_threshold;
    controller->param.under_threshold = under_threshold;
    controller->param.hysteresis      = hysteresis;
    controller->param.trip_count      = trip_count;

    Protect_Reset(controller);
}

void Protect_Reset(Protect_t* controller) {
    controller->_state.over_count  = 0;
    controller->_state.under_count = 0;
    controller->_state.tripped     = false;

    controller->out.over    = false;
    controller->out.under   = false;
    controller->out.tripped = false;
}

void Protect_Update(Protect_t* controller, Float_t value) {
    /* 带回差的阈值比较 */
    if (value > controller->param.over_threshold) {
        controller->out.over = true;
    }
    else if (value < controller->param.over_threshold - controller->param.hysteresis) {
        controller->out.over = false;
    }

    if (value < controller->param.under_threshold) {
        controller->out.under = true;
    }
    else if (value > controller->param.under_threshold + controller->param.hysteresis) {
        controller->out.under = false;
    }

    /* 越限计数，用于抑制尖峰误触发 */
    if (controller->out.over) {
        if (controller->_state.over_count < controller->param.trip_count) controller->_state.over_count++;
    }
    else {
        controller->_state.over_count = 0;
    }

    if (controller->out.under) {
        if (controller->_state.under_count < controller->param.trip_count) controller->_state.under_count++;
    }
    else {
        controller->_state.under_count = 0;
    }

    /* 锁存跳闸 */
    if (controller->_state.over_count >= controller->param.trip_count
        || controller->_state.under_count >= controller->param.trip_count) {
        controller->_state.tripped = true;
    }

    controller->out.tripped = controller->_state.tripped;
}

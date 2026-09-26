#include "protect.h"

void Protect_Init(Protect_t* protect,
                  Float_t over_threshold,
                  Float_t under_threshold,
                  Float_t hysteresis,
                  uint16_t trip_count) {
    protect->param.over_threshold  = over_threshold;
    protect->param.under_threshold = under_threshold;
    protect->param.hysteresis      = hysteresis;
    protect->param.trip_count      = trip_count;

    Protect_Reset(protect);
}

void Protect_Reset(Protect_t* protect) {
    protect->_state.over_count  = 0;
    protect->_state.under_count = 0;
    protect->_state.tripped     = false;

    protect->out.over    = false;
    protect->out.under   = false;
    protect->out.tripped = false;
}

void Protect_Update(Protect_t* protect, Float_t value) {
    /* 带回差的阈值比较 */
    if (value > protect->param.over_threshold) {
        protect->out.over = true;
    }
    else if (value < protect->param.over_threshold - protect->param.hysteresis) {
        protect->out.over = false;
    }

    if (value < protect->param.under_threshold) {
        protect->out.under = true;
    }
    else if (value > protect->param.under_threshold + protect->param.hysteresis) {
        protect->out.under = false;
    }

    /* 越限计数，用于抑制尖峰误触发 */
    if (protect->out.over) {
        if (protect->_state.over_count < protect->param.trip_count) protect->_state.over_count++;
    }
    else {
        protect->_state.over_count = 0;
    }

    if (protect->out.under) {
        if (protect->_state.under_count < protect->param.trip_count) protect->_state.under_count++;
    }
    else {
        protect->_state.under_count = 0;
    }

    /* 锁存跳闸 */
    if (protect->_state.over_count >= protect->param.trip_count
        || protect->_state.under_count >= protect->param.trip_count) {
        protect->_state.tripped = true;
    }

    protect->out.tripped = protect->_state.tripped;
}

#include "config_hrtim.h"

static HRTIM_IT_Callbacks_t hrtim_it_callbacks;


void CHRTIM_IT_Callbacks_Register(HRTIM_IT_Callbacks_t* _cbks) {
    if (_cbks) {
        hrtim_it_callbacks = *_cbks;
    }
}


#define __EXCLUDE_INVALID_HRTIM__(hhrtim)                                                                              \
    do {                                                                                                               \
        if (hhrtim != HRTIM_HANDLER) {                                                                                 \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)
#define __EXCLUDE_INVALID_TIMERINDEX__(timer_index)                                                                    \
    do {                                                                                                               \
        if (timer_index >= HRTIM_TIMERINDEX_NUM) {                                                                     \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)
#define __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index)                                                                 \
    do {                                                                                                               \
        __EXCLUDE_INVALID_HRTIM__(hhrtim);                                                                             \
        __EXCLUDE_INVALID_TIMERINDEX__(timer_index);                                                                   \
    } while (0)
#define __CALLBACK__(cbk, timer_index)                                                                                 \
    do {                                                                                                               \
        if (cbk) {                                                                                                     \
            cbk(timer_index);                                                                                          \
        }                                                                                                              \
    } while (0)

void HAL_HRTIM_RegistersUpdateCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.registers_update_cbk, timer_index);
}
void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.repetition_event_cbk, timer_index);
}
void HAL_HRTIM_Compare1EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.compare1_cbk, timer_index);
}
void HAL_HRTIM_Compare2EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.compare2_cbk, timer_index);
}
void HAL_HRTIM_Compare3EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.compare3_cbk, timer_index);
}
void HAL_HRTIM_Compare4EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.compare4_cbk, timer_index);
}
void HAL_HRTIM_Capture1EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.capture1_cbk, timer_index);
}
void HAL_HRTIM_Capture2EventCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.capture2_cbk, timer_index);
}
void HAL_HRTIM_DelayedProtectionCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.delayed_protection_cbk, timer_index);
}
void HAL_HRTIM_CounterResetCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.counter_reset_cbk, timer_index);
}
void HAL_HRTIM_Output1SetCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.output1_set_cbk, timer_index);
}
void HAL_HRTIM_Output1ResetCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.output1_reset_cbk, timer_index);
}
void HAL_HRTIM_Output2SetCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.output2_set_cbk, timer_index);
}
void HAL_HRTIM_Output2ResetCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.output2_reset_cbk, timer_index);
}
void HAL_HRTIM_BurstDMATransferCallback(HRTIM_HandleTypeDef* hhrtim, HRTIM_Timer_Index_t timer_index) {
    __EXCLUDE_INVALID_PARAM__(hhrtim, timer_index);
    __CALLBACK__(hrtim_it_callbacks.burst_dma_transfer_cbk, timer_index);
}
void HAL_HRTIM_ErrorCallback(HRTIM_HandleTypeDef* hhrtim) {
    __EXCLUDE_INVALID_HRTIM__(hhrtim);
    __CALLBACK__(hrtim_it_callbacks.error_cbk, HRTIM_TIMERINDEX_NUM);
}

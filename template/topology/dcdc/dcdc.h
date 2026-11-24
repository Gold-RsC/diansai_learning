#ifndef __DCDC_H__
#define __DCDC_H__

#include "mbase.h"
#include "config_hrtim.h"
#include "config_adc.h"
#include "cv_cc.h"
#include "sliding.h"
#include "kalman.h"

/********************* USER CONFIG BEGIN *************************/

#define HRTIM_TIMERID             (HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B)
#define HRTIM_OUTPUT_IDENTIFIER   (HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2)
#define HRTIM_PWMCMP1_TIMERINDEX  (HRTIM_TIMERINDEX_MASTER)
#define HRTIM_PWMCMP1_COMPAREUNIT (HRTIM_COMPAREUNIT_1)
#define HRTIM_PWMCMP2_TIMERINDEX  (HRTIM_TIMERINDEX_MASTER)
#define HRTIM_PWMCMP2_COMPAREUNIT (HRTIM_COMPAREUNIT_2)

#define TARGET_VOLTAGE (30.0f)
#define TARGET_CURRENT (0.5f)


/********************* USER CONFIG END ***************************/

typedef struct {
    Float_t voltage;
    Float_t current;
    Float_t power;
} Power_t;

extern Power_t in;
extern Power_t out;

/**
 * @brief 初始化DCDC
 */
void DCDC_Init();
/**
 * @brief 启动发波
 */
void DCDC_PWM_Start();
/**
 * @brief 停止发波
 */
void DCDC_PWM_Stop();


#endif

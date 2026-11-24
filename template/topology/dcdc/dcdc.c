#include "dcdc.h"

Kalman_t kalman[5];
Sliding_Filter_t sliding_filter[5];
CV_CC_t cv_cc;

void set_pwm(float pwm) {
    CHRTIM_Compare_Set(HRTIM_PWMCMP1_TIMERINDEX, HRTIM_PWMCMP1_COMPAREUNIT, 5000 * (1 - pwm));
    CHRTIM_Compare_Set(HRTIM_PWMCMP2_TIMERINDEX, HRTIM_PWMCMP2_COMPAREUNIT, 5000 * (1 + pwm));
}
void ADC_IT_cbk(void) {
    // 校准电压和电流，得到引脚上的电压值(mv)
    CADC_Calibrate_VDDA();
    float vin_pin_mv, vout_pin_mv, iin_pin_mv, iout_pin_mv;
    vin_pin_mv  = CADC_Calibrate_mv(adc_dma_buffer[1]);
    vout_pin_mv = CADC_Calibrate_mv(adc_dma_buffer[2]);
    iin_pin_mv  = CADC_Calibrate_mv(adc_dma_buffer[3]);
    iout_pin_mv = CADC_Calibrate_mv(adc_dma_buffer[4]);

    // 滤波
    vin_pin_mv  = Sliding_Filter_Update(&sliding_filter[1], Kalman_Update(&kalman[1], vin_pin_mv));
    vout_pin_mv = Sliding_Filter_Update(&sliding_filter[2], Kalman_Update(&kalman[2], vout_pin_mv));
    iin_pin_mv  = Sliding_Filter_Update(&sliding_filter[3], Kalman_Update(&kalman[3], iin_pin_mv));
    iout_pin_mv = Sliding_Filter_Update(&sliding_filter[4], Kalman_Update(&kalman[4], iout_pin_mv));

    // 拟合
    in.voltage  = vin_pin_mv * 1.0f + 0.0f;
    out.voltage = vout_pin_mv * 1.0f + 0.0f;
    in.current  = iin_pin_mv * 1.0f + 0.0f;
    out.current = iout_pin_mv * 1.0f + 0.0f;

    // 计算功率
    in.power  = in.voltage * in.current;
    out.power = out.voltage * out.current;

    // 控制输出
    DCDC_Update();
}

void DCDC_Init() {
    CV_CC_Init(&cv_cc, 0.002, 0.0055, 0.002, 0.0055, 0.2f, 0.1f, 0.9f, 0.5f);

    CADC_Calibration_Start();
    CADC_Start_DMA();

    ADC_IT_Callbacks_t cbks = {.conv_cplt_cbk = ADC_IT_cbk, .conv_half_cplt_cbk = NULL};
    ADC_IT_Callbacks_Register(&cbks);

    CHRTIM_Timer_Start(HRTIM_TIMERID);
}


void DCDC_PWM_Start() {
    CHRTIM_Output_Start(HRTIM_OUTPUT_IDENTIFIER);
}
void DCDC_PWM_Stop() {
    CHRTIM_Output_Stop(HRTIM_OUTPUT_IDENTIFIER);
}

void DCDC_Update() {
    CV_CC_Update(&cv_cc, out.voltage, out.current, TARGET_VOLTAGE, TARGET_CURRENT);
    set_pwm(cv_cc.out);
}

#include "spwm.h"

void spwm_init(SPWM_t* spwm,
               float pwm_freq,           // PWM载波频率，单位：Hz
               float target_freq,        // 表现频率，单位：Hz
               float target_rms_voltage  // 目标电压方均根，单位：V
) {
    memset(spwm, 0, sizeof(SPWM_t));
    spwm->param.pwm_freq           = pwm_freq;
    spwm->param.target_freq        = target_freq;
    spwm->param.target_rms_voltage = target_rms_voltage;
    spwm->param.max_amplitude      = 0.8f;

    sin_analyzer_init(&spwm->_state.sin_analyzer, target_freq, 0, 1000);

    pi_init(&spwm->_state.pi_controller, 0.08f, 0.01f, 0.3f, 0.3f);

    spwm->_state.phase_accumulator = 0.0f;
    spwm->_state.phase_increment   = 2.0f * MATH_PI * target_freq / pwm_freq;
}


// 开环
float spwm_update_open(SPWM_t* spwm) {
    spwm->_state.phase_accumulator += spwm->_state.phase_increment;

    float phase = fmodf(spwm->_state.phase_accumulator, MATH_PI * 2);

    float duty = 0.5f + 0.5f * spwm->param.max_amplitude * sinf(phase);

    return clamp(duty, 0.02f, 0.98f);
}
// 闭环
float spwm_update(SPWM_t* spwm, float voltage_sample, float current_sample) {
    sin_analyzer_update(&spwm->_state.sin_analyzer, voltage_sample, current_sample);

    if (spwm->_state.sin_analyzer.out.data_ready) {
        float pid_out = pi_update(
            &spwm->_state.pi_controller, spwm->_state.sin_analyzer.out.rms_voltage, spwm->param.target_rms_voltage);

        float new_amplitude                      = pid_out + spwm->param.max_amplitude;
        spwm->param.max_amplitude                = clamp(new_amplitude, 0.02f, 0.98f);
        spwm->_state.sin_analyzer.out.data_ready = false;
    }

    spwm->_state.phase_accumulator += spwm->_state.phase_increment;

    float phase = fmodf(spwm->_state.phase_accumulator, MATH_PI * 2);

    float duty = 0.5f + 0.5f * spwm->param.max_amplitude * sinf(phase);

    return clamp(duty, 0.02f, 0.98f);
}
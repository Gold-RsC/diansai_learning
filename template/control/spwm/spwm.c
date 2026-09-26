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

void spwm_pll_init(SPWM_PLL_t* spwm_pll,
                   float pwm_freq,            // PWM载波频率，单位：Hz
                   float target_freq,         // 表现频率，单位：Hz
                   float target_rms_voltage,  // 目标电压方均根，单位：V
                   SPLL_1ph_Sogi_t* spll      // PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
) {
    spwm_init(&spwm_pll->spwm, pwm_freq, target_freq, target_rms_voltage);

    spwm_pll->spll = spll;

    spll_1ph_sogi_init(spll, target_freq, pwm_freq, 0.01f, 0.01f);
    spll_1ph_sogi_reset(spll);
}
float spwm_pll_update(SPWM_PLL_t* spwm_pll, float grid_voltage, float voltage_sample, float current_sample) {
    // 获取电网相位和频率
    spll_1ph_sogi_update(spwm_pll->spll, grid_voltage);

    // 正弦分析
    sin_analyzer_update(&spwm_pll->spwm._state.sin_analyzer, voltage_sample, current_sample);

    // 闭环分析
    if (spwm_pll->spwm._state.sin_analyzer.out.data_ready) {
        float pid_out                      = pi_update(&spwm_pll->spwm._state.pi_controller,
                                                       spwm_pll->spwm._state.sin_analyzer.out.rms_voltage,
                                                       spwm_pll->spwm.param.target_rms_voltage);
        float new_amplitude                = pid_out + spwm_pll->spwm.param.max_amplitude;
        spwm_pll->spwm.param.max_amplitude = clamp(new_amplitude, 0.02f, 0.98f);
        spwm_pll->spwm._state.sin_analyzer.out.data_ready = false;
    }

    // 更新相位
    float phase = spwm_pll->spll->out.theta;

    // 调整目标频率为PLL锁定的电网频率
    spwm_pll->spwm.param.target_freq = spwm_pll->spll->out.fo;

    // PLL失锁保护
    if (diff(spwm_pll->spll->out.fo, GRID_FREQUENCY) < 0.01f) {
        return 0.5f;
    }

    // 生成SPWM占空比（与电网同步）
    float duty = 0.5f + 0.5f * spwm_pll->spwm.param.max_amplitude * sinf(phase);

    return clamp(duty, 0.05f, 0.95f);
}

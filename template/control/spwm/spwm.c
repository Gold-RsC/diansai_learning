#include "spwm.h"

void SPWM_Init(SPWM_t* spwm,
               Float_t pwm_freq,           // PWM载波频率，单位：Hz
               Float_t target_freq,        // 表现频率，单位：Hz
               Float_t target_rms_voltage  // 目标电压方均根，单位：V
) {
    memset(spwm, 0, sizeof(SPWM_t));
    spwm->param.pwm_freq           = pwm_freq;
    spwm->param.target_freq        = target_freq;
    spwm->param.target_rms_voltage = target_rms_voltage;
    spwm->param.max_amplitude      = 0.8f;

    Sin_Analyzer_Init(&spwm->_state.sin_analyzer, target_freq, 0, 1000);

    PI_Init(&spwm->_state.pi_controller, 0.08f, 0.01f, 0.3f, 0.3f);

    spwm->_state.phase_accumulator = 0.0f;
    spwm->_state.phase_increment   = 2.0f * MATH_PI * target_freq / pwm_freq;
}


// 开环
Float_t SPWM_Update_Open(SPWM_t* spwm) {
    spwm->_state.phase_accumulator += spwm->_state.phase_increment;

    Float_t phase = fmodf(spwm->_state.phase_accumulator, MATH_PI * 2);

    Float_t duty = 0.5f + 0.5f * spwm->param.max_amplitude * sinf(phase);

    return clamp(duty, 0.02f, 0.98f);
}
// 闭环
Float_t SPWM_Update(SPWM_t* spwm, Float_t voltage_sample, Float_t current_sample) {
    Sin_Analyzer_Update(&spwm->_state.sin_analyzer, voltage_sample, current_sample);

    if (spwm->_state.sin_analyzer.out.data_ready) {
        Float_t pid_out = PI_Update(
            &spwm->_state.pi_controller, spwm->_state.sin_analyzer.out.rms_voltage, spwm->param.target_rms_voltage);

        Float_t new_amplitude                      = pid_out + spwm->param.max_amplitude;
        spwm->param.max_amplitude                = clamp(new_amplitude, 0.02f, 0.98f);
        spwm->_state.sin_analyzer.out.data_ready = false;
    }

    spwm->_state.phase_accumulator += spwm->_state.phase_increment;

    Float_t phase = fmodf(spwm->_state.phase_accumulator, MATH_PI * 2);

    Float_t duty = 0.5f + 0.5f * spwm->param.max_amplitude * sinf(phase);

    return clamp(duty, 0.02f, 0.98f);
}

void SPWM_PLL_Init(SPWM_PLL_t* spwm_pll,
                   Float_t pwm_freq,            // PWM载波频率，单位：Hz
                   Float_t target_freq,         // 表现频率，单位：Hz
                   Float_t target_rms_voltage,  // 目标电压方均根，单位：V
                   SPLL_1ph_Sogi_t* spll      // PLL 参数指针，指向 SPLL_1ph_Sogi_t 结构体的指针
) {
    SPWM_Init(&spwm_pll->spwm, pwm_freq, target_freq, target_rms_voltage);

    spwm_pll->spll = spll;

    SPLL_1Ph_Sogi_Init(spll, target_freq, pwm_freq, 0.01f, 0.01f);
    SPLL_1Ph_Sogi_Reset(spll);
}
Float_t SPWM_PLL_Update(SPWM_PLL_t* spwm_pll, Float_t grid_voltage, Float_t voltage_sample, Float_t current_sample) {
    // 获取电网相位和频率
    SPLL_1Ph_Sogi_Update(spwm_pll->spll, grid_voltage);

    // 正弦分析
    Sin_Analyzer_Update(&spwm_pll->spwm._state.sin_analyzer, voltage_sample, current_sample);

    // 闭环分析
    if (spwm_pll->spwm._state.sin_analyzer.out.data_ready) {
        Float_t pid_out                      = PI_Update(&spwm_pll->spwm._state.pi_controller,
                                                       spwm_pll->spwm._state.sin_analyzer.out.rms_voltage,
                                                       spwm_pll->spwm.param.target_rms_voltage);
        Float_t new_amplitude                = pid_out + spwm_pll->spwm.param.max_amplitude;
        spwm_pll->spwm.param.max_amplitude = clamp(new_amplitude, 0.02f, 0.98f);
        spwm_pll->spwm._state.sin_analyzer.out.data_ready = false;
    }

    // 更新相位
    Float_t phase = spwm_pll->spll->out.theta;

    // 调整目标频率为PLL锁定的电网频率
    spwm_pll->spwm.param.target_freq = spwm_pll->spll->out.fo;

    // PLL失锁保护
    if (diff(spwm_pll->spll->out.fo, GRID_FREQUENCY) < 0.01f) {
        return 0.5f;
    }

    // 生成SPWM占空比（与电网同步）
    Float_t duty = 0.5f + 0.5f * spwm_pll->spwm.param.max_amplitude * sinf(phase);

    return clamp(duty, 0.05f, 0.95f);
}

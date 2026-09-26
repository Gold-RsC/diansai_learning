#include "sin_analyzer.h"

void sin_analyzer_init(Sin_Analyzer_t* analyzer, float measure_freq, float min_freq, float max_freq) {
    memset(analyzer, 0, sizeof(*analyzer));
    analyzer->param.measure_freq  = measure_freq;
    analyzer->param.min_freq      = min_freq;
    analyzer->param.max_freq      = max_freq;
    analyzer->param.zcd_threshold = 0.01f;
}

void sin_analyzer_update(Sin_Analyzer_t* analyzer, float voltage_sample, float current_sample) {
    if (analyzer->_state.sample_count == 0) {
        analyzer->_state.prev_sign = voltage_sample > analyzer->param.zcd_threshold;
        analyzer->_state.sample_count++;
    }
    analyzer->_state.voltage_squre_sum += voltage_sample * voltage_sample;
    analyzer->_state.current_squre_sum += current_sample * current_sample;
    analyzer->_state.power_sum += voltage_sample * current_sample;
    analyzer->_state.sample_count++;

    bool current_sign;
    if (current_sample > analyzer->param.zcd_threshold) {
        current_sign = true;
    }
    else if (current_sample < -analyzer->param.zcd_threshold) {
        current_sign = false;
    }
    else {
        current_sign = analyzer->_state.prev_sign;
    }
    if (current_sign ^ analyzer->_state.prev_sign) {
        // 半周期标记位为set，说明此时是周期的结束
        if (analyzer->_state.half_cycle_flag) {
            uint32_t N = analyzer->_state.sample_count / 2;
            float freq = analyzer->param.measure_freq / (2 * N);

            if (freq >= analyzer->param.min_freq && freq <= analyzer->param.max_freq) {
                float inv_N = 1.0f / (2 * N);

                analyzer->out.rms_voltage = sqrt(inv_N * analyzer->_state.voltage_squre_sum);
                analyzer->out.rms_current = sqrt(inv_N * analyzer->_state.current_squre_sum);

                analyzer->out.active_power   = inv_N * analyzer->_state.power_sum;
                analyzer->out.apparent_power = analyzer->out.rms_voltage * analyzer->out.rms_current;

                analyzer->out.power_factor = analyzer->out.active_power / analyzer->out.apparent_power;

                analyzer->out.freq = freq;

                analyzer->out.data_ready = true;
            }
            else {
                // 频率超出范围，不处理
            }
            analyzer->_state.voltage_squre_sum = 0.0f;
            analyzer->_state.current_squre_sum = 0.0f;
            analyzer->_state.power_sum         = 0.0f;
            analyzer->_state.sample_count      = 0;
        }
        else {
            // 半周期不处理
        }
        analyzer->_state.half_cycle_flag = !analyzer->_state.half_cycle_flag;
        analyzer->_state.prev_sign       = current_sign;
    }
}

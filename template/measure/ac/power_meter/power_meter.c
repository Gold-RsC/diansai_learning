#include "power_meter.h"

/* 内部 SOGI-PLL 环路参数：自然频率与阻尼比（误差已归一化，与输入幅值无关） */
#define POWER_METER_PLL_OMEGA_N (2.0f * MATH_PI * 20.0f)
#define POWER_METER_PLL_ZETA 0.707f

/* 启动屏蔽周期数：SOGI 收敛前不对外输出 */
#define POWER_METER_SETTLE_CYCLES 2

/* 频率块平均的周期数：每 N 个周期用总时长反推一次频率 */
#define POWER_METER_FREQ_CYCLES 4

/* 判定幅值有效的最小值，避免除零与启动瞬态输出 */
#define POWER_METER_MIN_AMP 1e-6f

void Power_Meter_Init(Power_Meter_t* meter, Float_t sample_freq, Float_t min_freq, Float_t max_freq) {
    memset(meter, 0, sizeof(*meter));
    meter->param.sample_freq = sample_freq;
    meter->param.min_freq    = min_freq;
    meter->param.max_freq    = max_freq;

    /* 额定频率取工频并限幅到调用方给定范围 */
    Float_t grid_freq = clamp(GRID_FREQUENCY, min_freq, max_freq);
    meter->_state.freq_avg = grid_freq;

    SPLL_1Ph_Sogi_Init(
        &meter->_state.spll_v, grid_freq, sample_freq, POWER_METER_PLL_OMEGA_N, POWER_METER_PLL_ZETA);
    SPLL_1Ph_Sogi_Init(
        &meter->_state.spll_i, grid_freq, sample_freq, POWER_METER_PLL_OMEGA_N, POWER_METER_PLL_ZETA);
}

void Power_Meter_Reset(Power_Meter_t* meter) {
    Float_t sample_freq = meter->param.sample_freq;
    Float_t min_freq    = meter->param.min_freq;
    Float_t max_freq    = meter->param.max_freq;

    Power_Meter_Init(meter, sample_freq, min_freq, max_freq);
}

void Power_Meter_Update(Power_Meter_t* meter, Float_t voltage_sample, Float_t current_sample) {
    /* 1. 电压、电流各做一路 SOGI，取出基波正交分量 */
    SPLL_1Ph_Sogi_Update(&meter->_state.spll_v, voltage_sample);
    SPLL_1Ph_Sogi_Update(&meter->_state.spll_i, current_sample);

    Float_t v_alpha = meter->_state.spll_v._state.osg_u[0];
    Float_t v_beta  = meter->_state.spll_v._state.osg_qu[0];
    Float_t i_alpha = meter->_state.spll_i._state.osg_u[0];
    Float_t i_beta  = meter->_state.spll_i._state.osg_qu[0];

    /* 2. 瞬时功率：p = vα*iα + vβ*iβ，q = vβ*iα - vα*iβ（峰值口径） */
    Float_t p_inst = v_alpha * i_alpha + v_beta * i_beta;
    Float_t q_inst = v_beta * i_alpha - v_alpha * i_beta;

    /* 3. 电压重构基波 α 的上升过零，作为整周期结算节拍 */
    bool zc = (meter->_state.alpha_prev < 0.0f && v_alpha > 0.0f);
    meter->_state.alpha_prev = v_alpha;

    if (zc && meter->_state.sample_count > 0) {
        uint32_t n = meter->_state.sample_count;

        /* 4. 启动屏蔽：SOGI 收敛前的周期不对外输出，也不计入频率窗口 */
        if (meter->_state.cycle_count < POWER_METER_SETTLE_CYCLES) {
            meter->_state.cycle_count++;
        }
        else {
            /* 5. 频率：每 N 个周期用总采样点数反推 */
            meter->_state.freq_sum += (Float_t)n;
            meter->_state.freq_cycles++;
            if (meter->_state.freq_cycles >= POWER_METER_FREQ_CYCLES) {
                meter->_state.freq_avg = meter->param.sample_freq * (Float_t)meter->_state.freq_cycles
                                       / meter->_state.freq_sum;
                meter->_state.freq_sum    = 0.0f;
                meter->_state.freq_cycles = 0;
                meter->_state.freq_valid  = true;
            }

            /* 6. 整周期平均：峰值口径乘 0.5 换算为有效值口径 */
            Float_t p     = 0.5f * meter->_state.p_sum / (Float_t)n;
            Float_t q     = 0.5f * meter->_state.q_sum / (Float_t)n;
            Float_t v_rms = sqrtf(meter->_state.v_sq_sum / (2.0f * (Float_t)n));
            Float_t i_rms = sqrtf(meter->_state.i_sq_sum / (2.0f * (Float_t)n));
            Float_t s     = v_rms * i_rms;

            bool valid = meter->_state.freq_valid && meter->_state.freq_avg >= meter->param.min_freq
                      && meter->_state.freq_avg <= meter->param.max_freq && v_rms > POWER_METER_MIN_AMP
                      && i_rms > POWER_METER_MIN_AMP;

            meter->out.locked = valid;
            if (valid) {
                meter->out.active_power   = p;
                meter->out.reactive_power = q;
                meter->out.apparent_power = s;
                meter->out.power_factor   = (s > POWER_METER_MIN_AMP) ? p / s : 0.0f;
                meter->out.rms_voltage    = v_rms;
                meter->out.rms_current    = i_rms;
                meter->out.freq           = meter->_state.freq_avg;
                meter->out.data_ready     = true;
            }
        }

        meter->_state.p_sum        = 0.0f;
        meter->_state.q_sum        = 0.0f;
        meter->_state.v_sq_sum     = 0.0f;
        meter->_state.i_sq_sum     = 0.0f;
        meter->_state.sample_count = 0;
    }

    /* 7. 累加当前采样点 */
    meter->_state.p_sum += p_inst;
    meter->_state.q_sum += q_inst;
    meter->_state.v_sq_sum += v_alpha * v_alpha + v_beta * v_beta;
    meter->_state.i_sq_sum += i_alpha * i_alpha + i_beta * i_beta;
    meter->_state.sample_count++;
}

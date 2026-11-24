#include "sin_analyzer.h"

/* 内部 SOGI-PLL 环路参数：自然频率与阻尼比（误差已归一化，与输入幅值无关） */
#define SIN_ANALYZER_PLL_OMEGA_N (2.0f * MATH_PI * 20.0f)
#define SIN_ANALYZER_PLL_ZETA 0.707f

/* 启动屏蔽周期数：SOGI 收敛前不对外输出，避免首拍数据偏差 */
#define SIN_ANALYZER_SETTLE_CYCLES 2

/* 频率块平均的周期数：每 N 个周期用总时长反推一次频率，抑制逐周期量化误差 */
#define SIN_ANALYZER_FREQ_CYCLES 4

/* 基波幅值到有效值的换算系数：rms = amplitude / sqrt(2) */
#define SIN_ANALYZER_INV_SQRT2 0.70710678f

/* 相位计算的幅值下限：αβ 模长太小时不更新相位，避免 atan2 抖动 */
#define SIN_ANALYZER_MIN_MAG_SQ 1e-9f

void Sin_Analyzer_Init(
    Sin_Analyzer_t* analyzer, Float_t measure_freq, Float_t min_freq, Float_t max_freq, Float_t zcd_threshold) {
    memset(analyzer, 0, sizeof(*analyzer));
    analyzer->param.measure_freq  = measure_freq;
    analyzer->param.min_freq      = min_freq;
    analyzer->param.max_freq      = max_freq;
    analyzer->param.zcd_threshold = zcd_threshold;

    /* 额定频率取工频并限幅到调用方给定范围，作为 PLL 初始频率与频率块平均初值 */
    Float_t grid_freq = clamp(GRID_FREQUENCY, min_freq, max_freq);
    analyzer->_state.freq_avg = grid_freq;

    SPLL_1Ph_Sogi_Init(
        &analyzer->_state.spll, grid_freq, measure_freq, SIN_ANALYZER_PLL_OMEGA_N, SIN_ANALYZER_PLL_ZETA);
}

void Sin_Analyzer_Reset(Sin_Analyzer_t* analyzer) {
    Float_t measure_freq  = analyzer->param.measure_freq;
    Float_t min_freq      = analyzer->param.min_freq;
    Float_t max_freq      = analyzer->param.max_freq;
    Float_t zcd_threshold = analyzer->param.zcd_threshold;

    Sin_Analyzer_Init(analyzer, measure_freq, min_freq, max_freq, zcd_threshold);
}

void Sin_Analyzer_Update(Sin_Analyzer_t* analyzer, Float_t sample) {
    /* 1. SOGI-PLL 生成基波正交分量；PLL 环路输出不参与测量 */
    SPLL_1Ph_Sogi_Update(&analyzer->_state.spll, sample);

    Float_t alpha  = analyzer->_state.spll._state.osg_u[0];
    Float_t beta   = analyzer->_state.spll._state.osg_qu[0];
    Float_t mag_sq = alpha * alpha + beta * beta;

    /* 2. 基波相位：α = A*sin(theta)，β = -A*cos(theta)，每个采样点更新 */
    if (mag_sq > SIN_ANALYZER_MIN_MAG_SQ) {
        Float_t theta = atan2f(alpha, -beta);
        if (theta < 0.0f) {
            theta += 2.0f * MATH_PI;
        }
        analyzer->out.theta = theta;
    }

    /* 3. 重构基波 α 的上升过零检测；SOGI 输出平滑，直接判符号即可 */
    bool zc = (analyzer->_state.alpha_prev < 0.0f && alpha > 0.0f);
    analyzer->_state.alpha_prev = alpha;

    if (zc && analyzer->_state.sample_count > 0) {
        uint32_t n  = analyzer->_state.sample_count;
        Float_t amp = sqrtf(analyzer->_state.amp_sq_sum / (Float_t)n);

        /* 4. 启动屏蔽：SOGI 收敛前的周期不对外输出，也不计入频率窗口 */
        if (analyzer->_state.cycle_count < SIN_ANALYZER_SETTLE_CYCLES) {
            analyzer->_state.cycle_count++;
        }
        else {
            /* 5. 频率：每 N 个周期用总采样点数反推，块平均抑制逐周期量化误差 */
            analyzer->_state.freq_sum += (Float_t)n;
            analyzer->_state.freq_cycles++;
            if (analyzer->_state.freq_cycles >= SIN_ANALYZER_FREQ_CYCLES) {
                analyzer->_state.freq_avg = analyzer->param.measure_freq * (Float_t)analyzer->_state.freq_cycles
                                          / analyzer->_state.freq_sum;
                analyzer->_state.freq_sum    = 0.0f;
                analyzer->_state.freq_cycles = 0;
                analyzer->_state.freq_valid  = true;
            }

            /* 6. 频率越界或幅值低于门限时，保持上一次的有效输出 */
            bool valid = analyzer->_state.freq_valid && analyzer->_state.freq_avg >= analyzer->param.min_freq
                      && analyzer->_state.freq_avg <= analyzer->param.max_freq
                      && amp * SIN_ANALYZER_INV_SQRT2 >= analyzer->param.zcd_threshold;

            analyzer->out.locked = valid;
            if (valid) {
                analyzer->out.rms        = amp * SIN_ANALYZER_INV_SQRT2;
                analyzer->out.amplitude  = amp;
                analyzer->out.freq       = analyzer->_state.freq_avg;
                analyzer->out.rms_total  = sqrtf(analyzer->_state.sq_sum / (Float_t)n);
                analyzer->out.dc_offset  = analyzer->_state.dc_sum / (Float_t)n;
                analyzer->out.data_ready = true;
            }
        }

        analyzer->_state.amp_sq_sum   = 0.0f;
        analyzer->_state.sq_sum       = 0.0f;
        analyzer->_state.dc_sum       = 0.0f;
        analyzer->_state.sample_count = 0;
    }

    /* 7. 当前采样点计入下一周期 */
    analyzer->_state.amp_sq_sum += mag_sq;
    analyzer->_state.sq_sum += sample * sample;
    analyzer->_state.dc_sum += sample;
    analyzer->_state.sample_count++;
}

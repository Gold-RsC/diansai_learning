#include "kalman.h"

void Kalman_Init(Kamlman_t* analyzer, float Q, float R, float initial_value) {
    analyzer->param.Q      = Q;
    analyzer->param.R      = R;
    analyzer->_state.index = 0;

    analyzer->param.harmonic_gain[0] = KALMAN_HARMONIC_GAIN_2nd;
    analyzer->param.harmonic_gain[1] = KALMAN_HARMONIC_GAIN_3rd;
    analyzer->param.harmonic_gain[2] = KALMAN_HARMONIC_GAIN_5th;

    analyzer->X = initial_value;
    for (int i = 0; i < 5; ++i) {
        analyzer->_state.window[i] = initial_value;
    }
}
float Kalman_Update(Kamlman_t* analyzer, float measurement) {

    // 基础方案
    // analyzer->_state.P = analyzer->_state.P + analyzer->param.Q;
    // analyzer->_state.K = analyzer->_state.P / (analyzer->_state.P + analyzer->param.R);
    // analyzer->X        = analyzer->X + analyzer->_state.K * (measurement - analyzer->X);
    // analyzer->_state.P = (1 - analyzer->_state.K) * analyzer->_state.P;


    // 考虑谐波项
    float residual  = measurement - analyzer->X;
    float R_dynamic = analyzer->param.R + fabs(residual) * 0.5f;

    float K = (analyzer->param.Q + R_dynamic) / (analyzer->param.Q + R_dynamic + analyzer->param.R);

    return analyzer->X;
}

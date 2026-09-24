#include "harmonic.h"

/**
 * @brief 初始化单个二阶陷波器（内部函数）
 *
 * @param notch 陷波器结构体指针
 * @param freq 陷波中心频率，单位：Hz
 * @param sample_freq 采样频率，单位：Hz
 * @param initial_value 初始值
 */
static void Harmonic_Notch_Init(Harmonic_Notch_t* notch,
                                Float_t freq,
                                Float_t sample_freq,
                                Float_t initial_value) {
    Float_t w0     = 2.0f * MATH_PI * freq / sample_freq;
    Float_t alpha  = sinf(w0) / (2.0f * HARMONIC_FILTER_Q);
    Float_t inv_a0 = 1.0f / (1.0f + alpha);
    Float_t b1     = -2.0f * cosf(w0) * inv_a0;

    notch->param.b0 = inv_a0;
    notch->param.b1 = b1;
    notch->param.b2 = inv_a0;
    notch->param.a1 = b1;
    notch->param.a2 = (1.0f - alpha) * inv_a0;

    notch->_state.x1 = initial_value;
    notch->_state.x2 = initial_value;
    notch->_state.y1 = initial_value;
    notch->_state.y2 = initial_value;
}

/**
 * @brief 单步更新单个二阶陷波器（内部函数）
 *
 * @param notch 陷波器结构体指针
 * @param input 输入值
 * @return Float_t 陷波后的值
 */
static Float_t Harmonic_Notch_Update(Harmonic_Notch_t* notch, Float_t input) {
    Float_t output = notch->param.b0 * input + notch->param.b1 * notch->_state.x1
                     + notch->param.b2 * notch->_state.x2 - notch->param.a1 * notch->_state.y1
                     - notch->param.a2 * notch->_state.y2;

    notch->_state.x2 = notch->_state.x1;
    notch->_state.x1 = input;

    notch->_state.y2 = notch->_state.y1;
    notch->_state.y1 = output;

    return output;
}

void Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                          Float_t gain_2,
                          Float_t gain_3,
                          Float_t gain_5,
                          Float_t sample_freq,
                          Float_t grid_freq,
                          Float_t out_min,
                          Float_t out_max,
                          Float_t initial_value) {

    filter->param.gain_2      = gain_2;
    filter->param.gain_3      = gain_3;
    filter->param.gain_5      = gain_5;
    filter->param.sample_freq = sample_freq;
    filter->param.grid_freq   = grid_freq;
    filter->param.out_min     = out_min;
    filter->param.out_max     = out_max;

    Harmonic_Notch_Init(&filter->_state.notch_2, 2.0f * grid_freq, sample_freq, initial_value);
    Harmonic_Notch_Init(&filter->_state.notch_3, 3.0f * grid_freq, sample_freq, initial_value);
    Harmonic_Notch_Init(&filter->_state.notch_5, 5.0f * grid_freq, sample_freq, initial_value);

    filter->out = initial_value;
}

Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement) {

    Float_t y = measurement;

    y -= filter->param.gain_2 * (y - Harmonic_Notch_Update(&filter->_state.notch_2, y));
    y -= filter->param.gain_3 * (y - Harmonic_Notch_Update(&filter->_state.notch_3, y));
    y -= filter->param.gain_5 * (y - Harmonic_Notch_Update(&filter->_state.notch_5, y));

    filter->out = clamp(y, filter->param.out_min, filter->param.out_max);

    return filter->out;
}

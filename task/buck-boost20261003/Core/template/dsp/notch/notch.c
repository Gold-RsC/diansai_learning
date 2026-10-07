#include "notch.h"

/**
 * @brief 初始化二阶陷波器
 *
 * @param filter 陷波器结构体指针
 * @param freq 陷波中心频率，单位：Hz
 * @param sample_freq 采样频率，单位：Hz
 * @param q 品质因数
 * @param initial_value 初始值
 */
void Notch_Filter_Init(Notch_Filter_t* filter,
                       Float_t freq,
                       Float_t sample_freq,
                       Float_t q,
                       Float_t initial_value) {
    Float_t w0     = 2.0f * MATH_PI * freq / sample_freq;
    Float_t alpha  = sinf(w0) / (2.0f * q);
    Float_t inv_a0 = 1.0f / (1.0f + alpha);
    Float_t b1     = -2.0f * cosf(w0) * inv_a0;

    filter->param.b0 = inv_a0;
    filter->param.b1 = b1;
    filter->param.b2 = inv_a0;
    filter->param.a1 = b1;
    filter->param.a2 = (1.0f - alpha) * inv_a0;

    filter->_state.x1 = initial_value;
    filter->_state.x2 = initial_value;
    filter->_state.y1 = initial_value;
    filter->_state.y2 = initial_value;
}

/**
 * @brief 更新二阶陷波器状态
 *
 * @param filter 陷波器结构体指针
 * @param input 输入值
 * @return Float_t 陷波后的值
 */
Float_t Notch_Filter_Update(Notch_Filter_t* filter, Float_t input) {
    Float_t output = filter->param.b0 * input + filter->param.b1 * filter->_state.x1
                     + filter->param.b2 * filter->_state.x2 - filter->param.a1 * filter->_state.y1
                     - filter->param.a2 * filter->_state.y2;

    filter->_state.x2 = filter->_state.x1;
    filter->_state.x1 = input;

    filter->_state.y2 = filter->_state.y1;
    filter->_state.y1 = output;

    return output;
}
#include "config_adc.h"

ADC_DMA_Buffer_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];

void CADC_Calibration_Start(void) {
#if ADC1_CHANNEL_NUM > 0
    HAL_ADCEx_Calibration_Start(ADC1_HANDLER, ADC_SINGLE_ENDED);
#endif
#if ADC2_CHANNEL_NUM > 0
    HAL_ADCEx_Calibration_Start(ADC2_HANDLER, ADC_SINGLE_ENDED);
#endif
#if ADC3_CHANNEL_NUM > 0
    HAL_ADCEx_Calibration_Start(ADC3_HANDLER, ADC_SINGLE_ENDED);
#endif
#if ADC4_CHANNEL_NUM > 0
    HAL_ADCEx_Calibration_Start(ADC4_HANDLER, ADC_SINGLE_ENDED);
#endif
#if ADC5_CHANNEL_NUM > 0
    HAL_ADCEx_Calibration_Start(ADC5_HANDLER, ADC_SINGLE_ENDED);
#endif
}

void CADC_Start_DMA(void) {
#if ADC1_CHANNEL_NUM > 0
    HAL_ADC_Start_DMA(ADC1_HANDLER, (uint32_t*)(adc_dma_buffer + ADC_DMA1_BUFFER_OFFSET), ADC1_CHANNEL_NUM);
#endif
#if ADC2_CHANNEL_NUM > 0
    HAL_ADC_Start_DMA(ADC2_HANDLER, (uint32_t*)(adc_dma_buffer + ADC_DMA2_BUFFER_OFFSET), ADC2_CHANNEL_NUM);
#endif
#if ADC3_CHANNEL_NUM > 0
    HAL_ADC_Start_DMA(ADC3_HANDLER, (uint32_t*)(adc_dma_buffer + ADC_DMA3_BUFFER_OFFSET), ADC3_CHANNEL_NUM);
#endif
#if ADC4_CHANNEL_NUM > 0
    HAL_ADC_Start_DMA(ADC4_HANDLER, (uint32_t*)(adc_dma_buffer + ADC_DMA4_BUFFER_OFFSET), ADC4_CHANNEL_NUM);
#endif
#if ADC5_CHANNEL_NUM > 0
    HAL_ADC_Start_DMA(ADC5_HANDLER, (uint32_t*)(adc_dma_buffer + ADC_DMA5_BUFFER_OFFSET), ADC5_CHANNEL_NUM);
#endif
}

void CADC_Stop_DMA(void) {
#if ADC1_CHANNEL_NUM > 0
    HAL_ADC_Stop_DMA(ADC1_HANDLER);
#endif
#if ADC2_CHANNEL_NUM > 0
    HAL_ADC_Stop_DMA(ADC2_HANDLER);
#endif
#if ADC3_CHANNEL_NUM > 0
    HAL_ADC_Stop_DMA(ADC3_HANDLER);
#endif
#if ADC4_CHANNEL_NUM > 0
    HAL_ADC_Stop_DMA(ADC4_HANDLER);
#endif
#if ADC5_CHANNEL_NUM > 0
    HAL_ADC_Stop_DMA(ADC5_HANDLER);
#endif
}


static ADC_IT_Callbacks_t adc_it_callbacks;

#define __ADC_CALLBACK_TRIGGER_MASK(adcidx) (1 << (adcidx - 1))

#define __ALL_ADC_CALLBACK_TRIGGER_MASK                                                                                \
    ((ADC1_CHANNEL_NUM > 0 ? __ADC_CALLBACK_TRIGGER_MASK(1) : 0) |                                                     \
     (ADC2_CHANNEL_NUM > 0 ? __ADC_CALLBACK_TRIGGER_MASK(2) : 0) |                                                     \
     (ADC3_CHANNEL_NUM > 0 ? __ADC_CALLBACK_TRIGGER_MASK(3) : 0) |                                                     \
     (ADC4_CHANNEL_NUM > 0 ? __ADC_CALLBACK_TRIGGER_MASK(4) : 0) |                                                     \
     (ADC5_CHANNEL_NUM > 0 ? __ADC_CALLBACK_TRIGGER_MASK(5) : 0))


uint8_t adc_conv_callback_trigger      = 0;
uint8_t adc_conv_half_callback_trigger = 0;

#define ADC_HANDLETYPEDEF_TO_ADCIDX(hadc)                                                                              \
    ((hadc)->Instance == ADC1   ? 1                                                                                    \
     : (hadc)->Instance == ADC2 ? 2                                                                                    \
     : (hadc)->Instance == ADC3 ? 3                                                                                    \
     : (hadc)->Instance == ADC4 ? 4                                                                                    \
     : (hadc)->Instance == ADC5 ? 5                                                                                    \
                                : -1)

void ADC_IT_Callbacks_Register(ADC_IT_Callbacks_t* _cbks) {
    if (_cbks) {
        adc_it_callbacks = *_cbks;
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (adc_it_callbacks.conv_cplt_cbk) {
        uint8_t adcidx = ADC_HANDLETYPEDEF_TO_ADCIDX(hadc);
        if (adcidx > 0) {
            adc_conv_callback_trigger |= __ADC_CALLBACK_TRIGGER_MASK(adcidx);
        }
        if (adc_conv_callback_trigger == __ALL_ADC_CALLBACK_TRIGGER_MASK) {
            adc_conv_callback_trigger = 0;
            adc_it_callbacks.conv_cplt_cbk();
        }
    }
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    if (adc_it_callbacks.conv_half_cplt_cbk) {
        uint8_t adcidx = ADC_HANDLETYPEDEF_TO_ADCIDX(hadc);
        if (adcidx > 0) {
            adc_conv_half_callback_trigger |= __ADC_CALLBACK_TRIGGER_MASK(adcidx);
        }
        if (adc_conv_half_callback_trigger == __ALL_ADC_CALLBACK_TRIGGER_MASK) {
            adc_conv_half_callback_trigger = 0;
            adc_it_callbacks.conv_half_cplt_cbk();
        }
    }
}
Float_t VDDA_mv;
void CADC_Calibrate_VDDA(void) {
    VDDA_mv = (Float_t)VREFINT_CAL_VREF * (Float_t)VREFINT_CAL / (Float_t)adc_dma_buffer[VREFINT_RAW_POS];
}
Float_t CADC_Calibrate_mv(ADC_DMA_Buffer_t raw) {
    return (Float_t)raw * VDDA_mv / 4095.0f;
}

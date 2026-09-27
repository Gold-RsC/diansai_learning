/**
 * @note CubeMX 配置
 * 1. Pinout & Configuration -> ADC1 -> Mode
 *   - IN1: IN1 Single-ended
 *   - IN2: IN2 Single-ended
 *   ...
 *   - Vrefint Channel [*]
 * 2. Pinout & Configuration -> ADC1 -> Configuration -> Parameter Settings
 *   - ADC_Settings
 *     - Continuous Conversion Mode: disabled
 *   - ADC_Regular_ConversionMode
 *     - Number of Conversion: n
 *     - Rank 1
 *       - Channel: Channel Vrefint
 *       - Sample Time: 640.5 Cycles
 *     - Rank 2
 *       - Channel: Channel 1
 *       - Sample Time: 640.5 Cycles
 *     - Rank 3
 *       - Channel: Channel 2
 *       - Sample Time: 640.5 Cycles
 *     ...
 *     - External Trigger Conversion Source: High Resolution Timer Trigger 1 event
 *     - External Trigger Conversion Edge: rising
 * 3. ADC1 -> Configuration -> DMA Settings
 *   - Add
 *   - DMA Request: ADC1
 *   - DMA Request Settings
 *     - Mode: Circular
 *     - Data Width: Half Word,Half Word(16-bit)
 * 4. HRTIM1 -> Configuration -> Timer Master(或者别的)
 *   - Compare Unit 3:
 *     - Compare Unit 3 Configuration: Enable
 *     - Compare Value: 2000
 * 5. HRTIM1 -> Configuration -> ADC Triggers Configuration
 *   - ADC Trigger 1:
 *     - ADC Trigger 1 Configuration: Enable
 *     - Update Trigger Source: Master Timer
 *     - Trigger Sources Selection: 1
 *       - 1st Trigger Source: master compare 3
 */
#ifndef __CONFIG_ADC_H__
#define __CONFIG_ADC_H__

#include "mbase.h"
#include "stm32g4xx_hal.h"
#include "adc.h"

/********************* USER CONFIG BEGIN *************************/

/**
 * @brief ADC通道数量
 *
 * @note 电压参考通道包含在ADC1的通道中
 */
#define ADC1_CHANNEL_NUM 3
#define ADC2_CHANNEL_NUM 0
#define ADC3_CHANNEL_NUM 0
#define ADC4_CHANNEL_NUM 0
#define ADC5_CHANNEL_NUM 0

/**
 * @brief ADC DMA缓冲区类型
 * @arg uint8_t -> byte
 * @arg uint16_t -> half word
 * @arg uint32_t -> word
 */
typedef uint16_t ADC_DMA_Buffer_t;

/********************* USER CONFIG END ***************************/

/**
 * @brief ADC DMA缓冲区
 */

#define ADC_DMA1_BUFFER_OFFSET (0)
#define ADC_DMA2_BUFFER_OFFSET (ADC_DMA1_BUFFER_OFFSET + ADC1_CHANNEL_NUM)
#define ADC_DMA3_BUFFER_OFFSET (ADC_DMA2_BUFFER_OFFSET + ADC2_CHANNEL_NUM)
#define ADC_DMA4_BUFFER_OFFSET (ADC_DMA3_BUFFER_OFFSET + ADC3_CHANNEL_NUM)
#define ADC_DMA5_BUFFER_OFFSET (ADC_DMA4_BUFFER_OFFSET + ADC4_CHANNEL_NUM)

#define ADC_DMA_BUFFER_SIZE (ADC_DMA5_BUFFER_OFFSET + ADC5_CHANNEL_NUM)

extern ADC_DMA_Buffer_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];


/**
 * @brief ADC句柄
 */
#define ADC1_HANDLER (&hadc1)
#define ADC2_HANDLER (&hadc2)
#define ADC3_HANDLER (&hadc3)
#define ADC4_HANDLER (&hadc4)
#define ADC5_HANDLER (&hadc5)

/**
 * @note 电压参考通道校准地址: VREFINT_CAL_ADDR (const uint16_t*)
 */

/**
 * @brief 启动ADC校准
 */
void CADC_Calibration_Start(void);

/**
 * @brief 启动/停止ADC DMA搬运
 */
void CADC_Start_DMA(void);
void CADC_Stop_DMA(void);

/**
 * @brief ADC中断回调函数类型
 */
typedef void (*ADC_IT_cbk)(ADC_HandleTypeDef* hadc);

typedef struct {
    ADC_IT_cbk conv_cplt_cbk;
    ADC_IT_cbk conv_half_cplt_cbk;
} ADC_IT_Callbacks_t;

/**
 * @brief 注册ADC回调
 * @param adc_it_callbacks ADC回调结构体指针
 */
void ADC_IT_Callbacks_Register(ADC_IT_Callbacks_t* adc_it_callbacks);

#endif

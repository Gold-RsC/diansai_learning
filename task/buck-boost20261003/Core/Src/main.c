/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "hrtim.h"
#include "gpio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "config_hrtim.h"
#include "config_adc.h"
#include "cv_cc.h"
#include "sliding_filter.h"
#include "kalman.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

typedef struct {
    float voltage;
    float current;
    float power;
} Power_t;
Power_t in, out;

Kalman_t kalman[5];
Sliding_Filter_t sliding_filter[5];

CV_CC_t cv_cc;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void set_pwm(float pwm) {
    CHRTIM_Compare_Set(HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, 5000 * (1 - pwm));
    CHRTIM_Compare_Set(HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, 5000 * (1 + pwm));
}
void ADC_IT_cbk(ADC_HandleTypeDef* hadc) {
    if (hadc == &hadc1) {

        // 校准电压和电流，得到引脚上的电压值(mv)
        CADC_Calibrate_VDDA();
        in.voltage  = CADC_Calibrate_mv(adc_dma_buffer[1]);
        out.voltage = CADC_Calibrate_mv(adc_dma_buffer[2]);
        in.current  = CADC_Calibrate_mv(adc_dma_buffer[3]);
        out.current = CADC_Calibrate_mv(adc_dma_buffer[4]);

        // 滤波
        in.voltage  = Sliding_Filter_Update(&sliding_filter[1], Kalman_Update(&kalman[1], in.voltage));
        out.voltage = Sliding_Filter_Update(&sliding_filter[2], Kalman_Update(&kalman[2], out.voltage));
        in.current  = Sliding_Filter_Update(&sliding_filter[3], Kalman_Update(&kalman[3], in.current));
        out.current = Sliding_Filter_Update(&sliding_filter[4], Kalman_Update(&kalman[4], out.current));

        // 拟合
        in.voltage  = in.voltage * 1.0f + 0.0f;
        out.voltage = out.voltage * 1.0f + 0.0f;
        in.current  = (in.current - 1650) * 1.0f + 0.0f;
        out.current = (out.current - 1650) * 1.0f + 0.0f;

        // 计算功率
        in.power  = in.voltage * in.current;
        out.power = out.voltage * out.current;

        // 控制输出
        CV_CC_Update(&cv_cc, out.voltage, out.current, target_voltage, target_current);
        set_pwm(cv_cc.out);
    }
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_HRTIM1_Init();
    /* USER CODE BEGIN 2 */

    CADC_Calibration_Start();
    CADC_Start_DMA();

    ADC_IT_Callbacks_t cbks = {.conv_cplt_cbk = ADC_IT_cbk, .conv_half_cplt_cbk = NULL};
    ADC_IT_Callbacks_Register(&cbks);

    CHRTIM_Timer_Start(HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);
    CHRTIM_Output_Start(HRTIM_OUTPUTID_TIMER_A | HRTIM_OUTPUTID_TIMER_B);


    CV_CC_Init(&cv_cc, 0.002, 0.0055, 0.002, 0.0055, 0.5f, 0.1f, 0.9f, 0.5f);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV2;
    RCC_OscInitStruct.PLL.PLLN            = 25;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
 * @note CubeMX 配置：
 * 1. Clock Configuration -> HCLK(MKz)设置为100 -> 回车
 * 2. Pinout & Configuration -> HRTIM1
 *   - Master Timer Enable [*]
 *   - Timer A : TA1 and TA2 outputs active
 *   - Timer B : TB1 and TB2 outputs active
 *   ...
 * 3. Configuration -> Master Timer
 *   - Time Base Setting
 *     - Prescaler Ratio: HRTIM Clock (即1x倍率)
 *     - Period: 10000 (此时周期为10000，频率为10000Hz)
 *   - Timer Unit
 *     - Preload Enable: enabled
 *     - Repetition Update: enabled
 *   - Compare Unit 1
 *   - Compare Unit 2
 *   ...
 * 4. Configuration -> Timer A
 *   - Time Base Setting
 *     - Prescaler Ratio: HRTIM Clock (即1x倍率)
 *     - Period: 10000 (此时周期为10000，频率为10000Hz)
 *   - Timer Unit
 *     - Preload Enable: enabled
 *     - Repetition Update: enabled
 *     - Dead Time Insertion: Deadtime is inserted between output 1 and output 2 (此时TA1 TA2自动互补)
 *     - Reset Trigger Sources Selection: upon master timer period event (这个配置是让Timer A和Master
 *                                        Timer同步，注意一定要让Period值相同，否则会出一些小问题)
 *   - Compare Unit 1
 *   - Compare Unit 2
 *   ...
 *   - Dead Time
 *     - Dead Time Conifguration: enbale
 *     - Prescaler: fDTG = fHRTIM (即1x倍率)
 *     - Rising Value: (单位0.01us)
 *     - Falling Value: (单位0.01us)
 *   - Output 1 Configuration
 *     - Set Source Selection: Compare Unit 1 (最好沿中心对称!!!)
 *     - Reset Source Selection: Compare Unit 2
 * 5. Configuration -> NVIC Settings (按需配置)
 *   - HRTIM master timer global interrupt: enabled
 *   - HRTIM timer A timer global interrupt: enabled
 */
#ifndef __CONFIG_HRTIM_H__
#define __CONFIG_HRTIM_H__

#include "mbase.h"
#include "stm32g4xx_hal.h"
#include "hrtim.h"

#define HRTIM_HANDLER (&hhrtim1)
/**
 * @brief 启动HRTIM定时器
 * @param hrtim_timerid 定时器ID, 可以用"|"拼接
 *     @arg HRTIM_TIMERID_MASTER
 *     @arg HRTIM_TIMERID_TIMER_A
 *     @arg HRTIM_TIMERID_TIMER_B
 *     @arg HRTIM_TIMERID_TIMER_C
 *     @arg HRTIM_TIMERID_TIMER_D
 *     @arg HRTIM_TIMERID_TIMER_E
 *     @arg HRTIM_TIMERID_TIMER_F
 */
#define CHRTIM_Timer_Start(hrtim_timerid) HAL_HRTIM_WaveformCountStart(HRTIM_HANDLER, hrtim_timerid)

/**
 * @brief 启动HRTIM输出
 * @param hrtim_output_identifier 输出标识符, 可以用"|"拼接
 *     @arg HRTIM_OUTPUT_TA1
 *     @arg HRTIM_OUTPUT_TA2
 *     @arg HRTIM_OUTPUT_TB1
 *     @arg HRTIM_OUTPUT_TB2
 *     @arg HRTIM_OUTPUT_TC1
 *     @arg HRTIM_OUTPUT_TC2
 *     @arg HRTIM_OUTPUT_TD1
 *     @arg HRTIM_OUTPUT_TD2
 *     @arg HRTIM_OUTPUT_TE1
 *     @arg HRTIM_OUTPUT_TE2
 *     @arg HRTIM_OUTPUT_TF1
 *     @arg HRTIM_OUTPUT_TF2
 */
#define CHRTIM_Output_Start(hrtim_output_identifier)                                                                   \
    HAL_HRTIM_WaveformOutputStart(HRTIM_HANDLER, hrtim_output_identifier)

/**
 * @brief 启用/禁用HRTIM Master Timer中断
 * @param hrtim_master_interrupt_register 主定时器中断使能寄存器值, 可以用"|"拼接
 *     @arg HRTIM_MDIER_MCMP1IE  CMP1中断使能位
 *     @arg HRTIM_MDIER_MCMP2IE  CMP2中断使能位
 *     @arg HRTIM_MDIER_MCMP3IE  CMP3中断使能位
 *     @arg HRTIM_MDIER_MCMP4IE  CMP4中断使能位
 *     @arg HRTIM_MDIER_MREPIE   重装载中断使能位
 *     @arg HRTIM_MDIER_SYNCIE   同步中断使能位
 *     @arg HRTIM_MDIER_MUPDIE   更新中断使能位
 *
 * @note 也可使用CubeMX配置开启，但个人认为这样配置符合习惯
 */
#define CHRTIM_Master_Enable_IT(hrtim_master_interrupt_register)                                                       \
    __HAL_HRTIM_MASTER_ENABLE_IT(HRTIM_HANDLER, hrtim_master_interrupt_register)
#define CHRTIM_Master_Disable_IT(hrtim_master_interrupt_register)                                                      \
    __HAL_HRTIM_MASTER_DISABLE_IT(HRTIM_HANDLER, hrtim_master_interrupt_register)

/**
 * @brief 启用/禁用HRTIM Timer中断
 * @param hrtim_timer_index 定时器索引, 指定要操作的定时器单元
 *     @arg HRTIM_TIMERINDEX_TIMER_A  Timer A
 *     @arg HRTIM_TIMERINDEX_TIMER_B  Timer B
 *     @arg HRTIM_TIMERINDEX_TIMER_C  Timer C
 *     @arg HRTIM_TIMERINDEX_TIMER_D  Timer D
 *     @arg HRTIM_TIMERINDEX_TIMER_E  Timer E
 *     @arg HRTIM_TIMERINDEX_TIMER_F  Timer F
 * @param hrtim_timer_interrupt_register 定时器中断使能寄存器值, 可以用"|"拼接
 *     @arg HRTIM_TIM_IT_NONE    无中断使能位
 *     @arg HRTIM_TIM_IT_CMP1    CMP1中断使能位
 *     @arg HRTIM_TIM_IT_CMP2    CMP2中断使能位
 *     @arg HRTIM_TIM_IT_CMP3    CMP3中断使能位
 *     @arg HRTIM_TIM_IT_CMP4    CMP4中断使能位
 *     @arg HRTIM_TIM_IT_REP     重装载中断使能位
 *     @arg HRTIM_TIM_IT_UPD     更新中断使能位
 *     @arg HRTIM_TIM_IT_CPT1    捕获1中断使能位
 *     @arg HRTIM_TIM_IT_CPT2    捕获2中断使能位
 *     @arg HRTIM_TIM_IT_SET1    输出置位1中断使能位
 *     @arg HRTIM_TIM_IT_RST1    输出复位1中断使能位
 *     @arg HRTIM_TIM_IT_SET2    输出置位2中断使能位
 *     @arg HRTIM_TIM_IT_RST2    输出复位2中断使能位
 *     @arg HRTIM_TIM_IT_RST     复位中断使能位
 *     @arg HRTIM_TIM_IT_DLYPRT  延迟保护中断使能位
 * @note Timer Master不能通过此函数配置中断使能位，只能通过HRTIM_Master_Enable_IT()函数配置
 */
#define CHRTIM_Timer_Enable_IT(hrtim_timer_index, hrtim_timer_interrupt_register)                                      \
    __HAL_HRTIM_TIMER_ENABLE_IT(HRTIM_HANDLER, hrtim_timer_index, hrtim_timer_interrupt_register)
#define CHRTIM_Timer_Disable_IT(hrtim_timer_index, hrtim_timer_interrupt_register)                                     \
    __HAL_HRTIM_TIMER_DISABLE_IT(HRTIM_HANDLER, hrtim_timer_index, hrtim_timer_interrupt_register)


/**
 * @brief 定时器索引
 * @arg HRTIM_TIMERINDEX_TIMER_A = 0x0U
 * @arg HRTIM_TIMERINDEX_TIMER_B = 0x1U
 * @arg HRTIM_TIMERINDEX_TIMER_C = 0x2U
 * @arg HRTIM_TIMERINDEX_TIMER_D = 0x3U
 * @arg HRTIM_TIMERINDEX_TIMER_E = 0x4U
 * @arg HRTIM_TIMERINDEX_TIMER_F = 0x5U
 * @arg HRTIM_TIMERINDEX_MASTER  = 0x6U
 */
typedef uint32_t HRTIM_Timer_Index_t;

typedef void (*HRTIM_IT_cbk)(HRTIM_Timer_Index_t TimerIdx);

#define HRTIM_TIMERINDEX_NUM 7


typedef struct {
    HRTIM_IT_cbk registers_update_cbk;    // 预装载寄存器更新到实际工作寄存器时触发（对应Period）
    HRTIM_IT_cbk repetition_event_cbk;    // 重装载时触发（对应Repetition Counter）
    HRTIM_IT_cbk compare1_cbk;            // 计数器到达CMP1时触发
    HRTIM_IT_cbk compare2_cbk;            // 计数器到达CMP2时触发
    HRTIM_IT_cbk compare3_cbk;            // 计数器到达CMP3时触发
    HRTIM_IT_cbk compare4_cbk;            // 计数器到达CMP4时触发
    HRTIM_IT_cbk capture1_cbk;            // CPT1中断时触发
    HRTIM_IT_cbk capture2_cbk;            // CPT2中断时触发
    HRTIM_IT_cbk delayed_protection_cbk;  // 进入延迟空闲或平衡空闲时触发
    HRTIM_IT_cbk counter_reset_cbk;       // 计数器复位时触发
    HRTIM_IT_cbk output1_set_cbk;         // 通道1为高电平时触发
    HRTIM_IT_cbk output1_reset_cbk;       // 通道1为低电平时触发
    HRTIM_IT_cbk output2_set_cbk;         // 通道2为高电平时触发
    HRTIM_IT_cbk output2_reset_cbk;       // 通道2为低电平时触发
    HRTIM_IT_cbk burst_dma_transfer_cbk;  // 基冲DMA传输完成时触发
    HRTIM_IT_cbk error_cbk;               // 错误中断时触发，其参数并不重要，一定为HRTIM_TIMERINDEX_NUM，无意义
} HRTIM_IT_Callbacks_t;

/**
 * @brief 注册HRTIM中断回调函数
 * @param hrtim_it_callbacks HRTIM中断回调函数指针, 指向HRTIM_IT_Callbacks_t结构体的指针
 *
 * @note 不用时最好设置为NULL
 */
void CHRTIM_IT_Callbacks_Register(HRTIM_IT_Callbacks_t* hrtim_it_callbacks);


/**
 * @brief 设置HRTIM Timer的比较值
 * @param hrtim_timer_index 定时器索引, 指定要操作的定时器单元
 *     @arg HRTIM_TIMERINDEX_TIMER_A  Timer A
 *     @arg HRTIM_TIMERINDEX_TIMER_B  Timer B
 *     @arg HRTIM_TIMERINDEX_TIMER_C  Timer C
 *     @arg HRTIM_TIMERINDEX_TIMER_D  Timer D
 *     @arg HRTIM_TIMERINDEX_TIMER_E  Timer E
 *     @arg HRTIM_TIMERINDEX_TIMER_F  Timer F
 *     @arg HRTIM_TIMERINDEX_MASTER   Timer Master
 * @param cmp_unit 比较单元
 *     @arg HRTIM_COMPAREUNIT_1    CMP1
 *     @arg HRTIM_COMPAREUNIT_2    CMP2
 *     @arg HRTIM_COMPAREUNIT_3    CMP3
 *     @arg HRTIM_COMPAREUNIT_4    CMP4
 * @param cmp_value 比较值, 要设置的比较值
 */
#define CHRTIM_Compare_Set(hrtim_timer_index, cmp_unit, cmp_value)                                                     \
    __HAL_HRTIM_SETCOMPARE(HRTIM_HANDLER, hrtim_timer_index, cmp_unit, cmp_value)

#endif

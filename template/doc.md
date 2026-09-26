# template API 速查

浮点类型统一用 `Float_t`（`mbase.h` 中 `typedef float Float_t;`）。
函数命名统一为 `Xxx_Yyy_Init` / `Xxx_Yyy_Update`。

目录按功能分组：

```
mbase/    基础类型与宏
dsp/      通用数字信号处理模块
filter/   滤波器
control/  控制
measure/  测量与估计
debug/    调试输出与上位机
```

## mbase

类型与宏，无函数。所有模块都包含它。

```c
Float_t             等价于 float
MATH_PI             3.1415926f
GRID_FREQUENCY      50.0f

min(x, y)           (x) < (y) ? (x) : (y)
max(x, y)           (x) > (y) ? (x) : (y)
clamp(x, min, max)  限幅
diff(x, y)          fabsf((x) - (y))
```

## dsp/notch

二阶 IIR 陷波器。中心频率处增益**严格为 0**，直流与奈奎斯特频率处严格为 1。
`q` 为品质因数（陷波中心频率 / 陷波带宽），建议 20 ~ 50，不要超过 100。

```c
void    Notch_Filter_Init(Notch_Filter_t* filter,
                          Float_t freq,          // 陷波中心频率，单位：Hz
                          Float_t sample_freq,   // 采样（ISR）频率，单位：Hz
                          Float_t q,             // 品质因数
                          Float_t initial_value);
Float_t Notch_Filter_Update(Notch_Filter_t* filter, Float_t input);
```

## filter/harmonic

2、3、5 次谐波抑制。三个陷波器**级联**，`gain_h` 为该次谐波的抑制强度
（0 直通，1 完全滤除），实测谐波处增益恰为 `1 - gain_h`。

```c
void    Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                             Float_t gain_2, Float_t gain_3, Float_t gain_5,
                             Float_t sample_freq,   // 采样（ISR）频率，单位：Hz
                             Float_t grid_freq,     // 基波频率，单位：Hz
                             Float_t out_min, Float_t out_max,
                             Float_t initial_value);
Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement);
```

## filter/kalman

一维 Kalman 滤波器。`Q` 为系统过程噪声，`R` 为测量噪声。

```c
void    Kalman_Filter_Init(Kalman_Filter_t* filter,
                           Float_t Q, Float_t R,
                           Float_t out_min, Float_t out_max,
                           Float_t initial_value);
Float_t Kalman_Filter_Update(Kalman_Filter_t* filter, Float_t measurement);
```

## filter/sliding

滑动平均滤波器，窗口 5 点（`SLIDING_FILTER_WINDOW_SIZE`）。

```c
void    Sliding_Filter_Init(Sliding_Filter_t* filter,
                            Float_t out_min, Float_t out_max,
                            Float_t initial_value);
Float_t Sliding_Filter_Update(Sliding_Filter_t* filter, Float_t measurement);
```

## control/pid

增量式 PI 控制器。

```c
void    PI_Init(PI_t* analyzer, Float_t kp, Float_t ki,
                Float_t outmin, Float_t outmax);
Float_t PI_Update(PI_t* analyzer, Float_t now, Float_t target);
```

## control/spwm

SPWM 占空比生成。`SPWM_Update_Open` 为开环，`SPWM_Update` 为闭环。
`SPWM_PLL_t` 版本从外部 PLL 取相位（`theta`）。

```c
void    SPWM_Init(SPWM_t* spwm,
                  Float_t pwm_freq,            // PWM 载波频率，单位：Hz
                  Float_t target_freq,         // 目标频率，单位：Hz
                  Float_t target_rms_voltage); // 目标电压有效值，单位：V
Float_t SPWM_Update_Open(SPWM_t* spwm);
Float_t SPWM_Update(SPWM_t* spwm, Float_t voltage_sample, Float_t current_sample);

void    SPWM_PLL_Init(SPWM_PLL_t* spwm_pll,
                      Float_t pwm_freq, Float_t target_freq, Float_t target_rms_voltage,
                      SPLL_1ph_Sogi_t* spll);
Float_t SPWM_PLL_Update(SPWM_PLL_t* spwm_pll,
                        Float_t grid_voltage, Float_t voltage_sample, Float_t current_sample);
```

## measure/sin_analyzer

正弦量测量：真有效值、有功/视在功率、功率因数、频率、数据就绪标志。
在 ADC 中断里调用 `Update`，结果放在 `analyzer->out`。

```c
void Sin_Analyzer_Init(Sin_Analyzer_t* analyzer,
                       Float_t measure_freq,   // 采样（ISR）频率，单位：Hz
                       Float_t min_freq, Float_t max_freq);
void Sin_Analyzer_Update(Sin_Analyzer_t* analyzer,
                         Float_t voltage_sample,
                         Float_t current_sample);
```

## measure/spll

单相 SOGI 锁相环，取自 TI C2000 SolarLib（原版为 IQ23 定点，此处是浮点改写）。
结果放在 `spll->out`（`fo` / `theta` / `sine` / `cosine`）。

**注意：输入必须归一化为标幺值**，否则环路会振荡。

```c
void SPLL_1Ph_Sogi_Init(SPLL_1ph_Sogi_t* spll,
                        Float_t grid_freq,   // 电网额定频率，单位：Hz
                        Float_t isr_freq,    // ISR 调用频率，单位：Hz
                        Float_t lpf_b0, Float_t lpf_b1);
void SPLL_1Ph_Sogi_Reset(SPLL_1ph_Sogi_t* spll);
void SPLL_1Ph_Sogi_Update(SPLL_1ph_Sogi_t* spll, Float_t ac_voltage);
void SPLL_1Ph_Sogi_Coeff_Calc(SPLL_1ph_Sogi_t* spll);
```

## debug/print_adapt

串口格式化输出。`UART_ADDR` 宏指向上位机串口，输出缓冲 256 字节。

```c
size_t Printf_Normal(const char* format, ...);   // 阻塞式
size_t Printf_DMA(const char* format, ...);      // DMA
size_t Printf_IT(const char* format, ...);       // 中断式
```

## debug/vofa

VOFA+ 上位机协议。

```c
#define Vofa_Printf(format, ...)      // 等价于 Printf_Normal，走 FireWater 文本协议
#define Vofa_FireWater(format, ...)   // 同上

size_t Vofa_JustFloat(Float_t* _data, size_t _num);   // JustFloat 波形协议
```

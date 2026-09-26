# template API 速查

浮点类型统一用 `Float_t`（`mbase.h` 中 `typedef float Float_t;`）。
函数命名统一为 `Xxx_Yyy_Init` / `Xxx_Yyy_Update`，类型名 `Xxx_Yyy_t`。

目录按功能与信号域分组：

```
mbase/          基础类型与宏
dsp/            通用 DSP 原语（与具体电路无关）
filter/         通用滤波器
control/
  common/       与拓扑无关的控制原语
  dc/           直流量场合（DC-DC 及其衍生）
  ac/           交流量场合（逆变、并网、并联）
measure/
  dc/           直流量测量
  ac/           交流量测量
debug/          调试输出与上位机
```

## 目录

**mbase** — 基础类型与宏

- [mbase](#mbase)

**dsp/** — 通用 DSP 原语

- [dsp/notch](#dspnotch)
- [dsp/goertzel](#dspgoertzel)
- [dsp/lpf](#dsplpf)

**filter/** — 通用滤波器

- [filter/harmonic](#filterharmonic)
- [filter/kalman](#filterkalman)
- [filter/sliding](#filtersliding)

**control/common/** — 与拓扑无关的控制原语

- [control/common/pid](#controlcommonpid)
- [control/common/soft_start](#controlcommonsoft_start)
- [control/common/protect](#controlcommonprotect)

**control/dc/** — 直流量场合

- [control/dc/dcdc](#controldcdcdc)
- [control/dc/cv_cc](#controldccv_cc)
- [control/dc/mppt](#controldcmppt)

**control/ac/** — 交流量场合

- [control/ac/pr](#controlacpr)
- [control/ac/droop](#controlacdroop)
- [control/ac/spwm](#controlacspwm)

**measure/dc/** — 直流量测量

- [measure/dc/dc_meter](#measuredcdc_meter)

**measure/ac/** — 交流量测量

- [measure/ac/sin_analyzer](#measureacsin_analyzer)
- [measure/ac/spll](#measureacspll)
- [measure/ac/thd](#measureacthd)

**debug/** — 调试输出与上位机

- [debug/print_adapt](#debugprint_adapt)
- [debug/vofa](#debugvofa)

## mbase

无函数，只有类型与宏，所有模块的根依赖。

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

```c
void    Notch_Filter_Init(Notch_Filter_t* filter,
                          Float_t freq,          // 陷波中心频率，Hz
                          Float_t sample_freq,   // 采样频率，Hz
                          Float_t q,             // 品质因数，建议 20 ~ 50
                          Float_t initial_value);
Float_t Notch_Filter_Update(Notch_Filter_t* filter, Float_t input);
```

## dsp/goertzel

单频点 DFT。O(N) 单次遍历，无 twiddle 表，无需存储采样数组。
每满 `window_size` 点输出一次幅值与相位。

```c
void    Goertzel_Init(Goertzel_t* goertzel,
                      Float_t sample_freq, Float_t target_freq, uint32_t window_size);
void    Goertzel_Reset(Goertzel_t* goertzel);
Float_t Goertzel_Update(Goertzel_t* goertzel, Float_t sample);
```

结果在 `out.magnitude` / `out.phase`，`out.ready` 为单拍脉冲。

**窗口必须取基波的整数个周期**，否则频谱泄漏。

## dsp/lpf

一阶 IIR 低通。直流增益严格为 1，-3dB 点位于 `cutoff_freq`。

```c
void    Lpf_Filter_Init(Lpf_Filter_t* lpf,
                        Float_t sample_freq, Float_t cutoff_freq,
                        Float_t out_min, Float_t out_max, Float_t initial_value);
Float_t Lpf_Filter_Update(Lpf_Filter_t* lpf, Float_t input);
```

## filter/harmonic

2、3、5 次谐波抑制，三个陷波器**级联**。`gain_h` 为抑制强度（0 直通，1 完全滤除）。

```c
void    Harmonic_Filter_Init(Harmonic_Filter_t* filter,
                             Float_t gain_2, Float_t gain_3, Float_t gain_5,
                             Float_t sample_freq, Float_t grid_freq,
                             Float_t out_min, Float_t out_max, Float_t initial_value);
Float_t Harmonic_Filter_Update(Harmonic_Filter_t* filter, Float_t measurement);
```

## filter/kalman

一维 Kalman 滤波器。

```c
void    Kalman_Filter_Init(Kalman_Filter_t* filter, Float_t Q, Float_t R,
                           Float_t out_min, Float_t out_max, Float_t initial_value);
Float_t Kalman_Filter_Update(Kalman_Filter_t* filter, Float_t measurement);
```

## filter/sliding

滑动平均，窗口 5 点（`SLIDING_FILTER_WINDOW_SIZE`）。

```c
void    Sliding_Filter_Init(Sliding_Filter_t* filter,
                            Float_t out_min, Float_t out_max, Float_t initial_value);
Float_t Sliding_Filter_Update(Sliding_Filter_t* filter, Float_t measurement);
```

## control/common/pid

增量式 PI 控制器。增量式的状态就是被限幅的输出，**本身不会积分饱和**。
`PI_Reset` 用于启动与故障恢复，`PI_Set_Output` 供多环无扰切换。

```c
void    PI_Init(PI_t* controller, Float_t kp, Float_t ki, Float_t outmin, Float_t outmax);
Float_t PI_Update(PI_t* controller, Float_t now, Float_t target);
void    PI_Reset(PI_t* controller);
void    PI_Set_Output(PI_t* controller, Float_t output);
```

## control/common/soft_start

软启动斜坡限制器。上下行都受限，兼作变化率限制器。

```c
void    Soft_Start_Init(Soft_Start_t* controller, Float_t step, Float_t start);
void    Soft_Start_Reset(Soft_Start_t* controller);
Float_t Soft_Start_Update(Soft_Start_t* controller, Float_t target);
```

## control/common/protect

过压/过流与欠压保护。带回差、计数去抖、跳闸锁存。

```c
void Protect_Init(Protect_t* controller, Float_t over_threshold, Float_t under_threshold,
                  Float_t hysteresis, uint16_t trip_count);
void Protect_Reset(Protect_t* controller);
void Protect_Update(Protect_t* controller, Float_t value);
```

结果在 `out.over` / `out.under` / `out.tripped`。
`tripped` 为锁存位，须 `Protect_Reset` 才能清除。

## control/dc/dcdc

DC-DC 拓扑模型。**不含控制环路**，只提供稳态关系、前馈占空比、增益与安全区。

```c
typedef enum { DCDC_BUCK, DCDC_BOOST, DCDC_BUCK_BOOST } Dcdc_Topology_t;

void    Dcdc_Init(Dcdc_t* controller, Dcdc_Topology_t topology);
Float_t Dcdc_Feedforward(Dcdc_t* controller, Float_t v_in, Float_t v_target);
Float_t Dcdc_Clamp_Duty(Dcdc_t* controller, Float_t duty);
Float_t Dcdc_Plant_Gain(Dcdc_t* controller, Float_t v_in, Float_t duty);
```

Boost 与 Buck-Boost 有右半平面零点，闭环带宽不要超过开关频率的 1/10。

## control/dc/cv_cc

CV/CC 双环控制器。两环同时运行，带切换回差；闲置环跟踪生效环，
切换瞬间占空比不跳变。

```c
void    Cv_Cc_Init(Cv_Cc_t* controller, Float_t v_target, Float_t i_target,
                   Float_t kp_v, Float_t ki_v, Float_t kp_i, Float_t ki_i,
                   Float_t duty_min, Float_t duty_max, Float_t switch_margin);
void    Cv_Cc_Reset(Cv_Cc_t* controller);
Float_t Cv_Cc_Update(Cv_Cc_t* controller, Float_t v_out, Float_t i_out);
Cv_Cc_Mode_t Cv_Cc_Get_Mode(Cv_Cc_t* controller);
```

## control/dc/mppt

扰动观察法 MPPT。功率上升则保持扰动方向，下降则反向。

```c
void    Mppt_Init(Mppt_t* controller, Float_t step, Float_t min_duty, Float_t max_duty,
                  uint32_t period);
void    Mppt_Reset(Mppt_t* controller);
Float_t Mppt_Update(Mppt_t* controller, Float_t v_in, Float_t i_in);
```

输入电压/电流应先经低通滤波，否则功率比较会被噪声翻转。

## control/ac/pr

准比例谐振控制器。谐振频率处增益为 `kp + kr`，直流增益为 `kp`。
用于交流量闭环——PI 对交流量有稳态误差，PR 没有。

```c
void    Pr_Init(Pr_t* controller, Float_t kp, Float_t kr,
                Float_t resonant_freq, Float_t bandwidth, Float_t sample_freq,
                Float_t out_min, Float_t out_max);
void    Pr_Reset(Pr_t* controller);
Float_t Pr_Update(Pr_t* controller, Float_t error);
```

注意本模块直接接收**误差**，而不是实测值与目标值两个参数。

## control/ac/droop

下垂控制。`f = f0 - m*P`，`V = V0 - n*Q`。并联运行类赛题的核心算法。

```c
void Droop_Init(Droop_t* controller, Float_t freq_noload, Float_t amp_noload,
                Float_t m, Float_t n, Float_t filter_cutoff, Float_t sample_freq);
void Droop_Reset(Droop_t* controller);
void Droop_Update(Droop_t* controller, Float_t p, Float_t q);
```

结果在 `out.freq` 与 `out.amplitude`。P/Q 内置低通，**不可省略**。

## control/ac/spwm

SPWM 占空比生成。`SPWM_Update_Open` 开环，`SPWM_Update` 闭环。
`SPWM_PLL_t` 版本从外部 PLL 取相位。

```c
void    SPWM_Init(SPWM_t* controller, Float_t pwm_freq, Float_t target_freq,
                  Float_t target_rms_voltage);
Float_t SPWM_Update_Open(SPWM_t* controller);
Float_t SPWM_Update(SPWM_t* controller, Float_t voltage_sample, Float_t current_sample);

void    SPWM_PLL_Init(SPWM_PLL_t* controller, Float_t pwm_freq, Float_t target_freq,
                      Float_t target_rms_voltage, SPLL_1ph_Sogi_t* analyzer);
Float_t SPWM_PLL_Update(SPWM_PLL_t* controller, Float_t grid_voltage,
                        Float_t voltage_sample, Float_t current_sample);
```

## measure/dc/dc_meter

直流量测量：窗口内平均值与纹波峰峰值。窗口应覆盖整数个开关周期。

```c
void    Dc_Meter_Init(Dc_Meter_t* analyzer, uint32_t window_size);
void    Dc_Meter_Reset(Dc_Meter_t* analyzer);
Float_t Dc_Meter_Update(Dc_Meter_t* analyzer, Float_t sample);
```

结果在 `out.average` / `out.ripple`，`out.ready` 为单拍脉冲。

## measure/ac/sin_analyzer

正弦量测量：真有效值、有功/视在功率、功率因数、频率。

```c
void Sin_Analyzer_Init(Sin_Analyzer_t* analyzer, Float_t measure_freq,
                       Float_t min_freq, Float_t max_freq);
void Sin_Analyzer_Update(Sin_Analyzer_t* analyzer, Float_t voltage_sample,
                         Float_t current_sample);
```

`out.data_ready` 是**电平式**标志，由消费者清零，且只能有一个消费者。

## measure/ac/spll

单相 SOGI 锁相环，取自 TI C2000 SolarLib（原版 IQ23 定点，此处为浮点改写）。

```c
void SPLL_1Ph_Sogi_Init(SPLL_1ph_Sogi_t* analyzer, Float_t grid_freq, Float_t isr_freq,
                        Float_t lpf_b0, Float_t lpf_b1);
void SPLL_1Ph_Sogi_Reset(SPLL_1ph_Sogi_t* analyzer);
void SPLL_1Ph_Sogi_Update(SPLL_1ph_Sogi_t* analyzer, Float_t ac_voltage);
void SPLL_1Ph_Sogi_Coeff_Calc(SPLL_1ph_Sogi_t* analyzer);
```

**输入必须归一化为标幺值**，否则环路会振荡。

## measure/ac/thd

总谐波畸变率。窗口固定取一个基波周期 `N = fs/f0`，各次谐波自动落在整数 bin。

```c
void    Thd_Init(Thd_t* analyzer, Float_t sample_freq, Float_t fundamental_freq,
                 uint8_t max_order);
void    Thd_Reset(Thd_t* analyzer);
Float_t Thd_Update(Thd_t* analyzer, Float_t sample);
```

结果在 `out.fundamental` / `out.thd`（小数，0.05 表示 5%）。
最高分析 `THD_MAX_ORDER` 次。

## debug/print_adapt

串口格式化输出。`UART_ADDR` 指向串口，缓冲 256 字节。

```c
size_t Printf_Normal(const char* format, ...);   // 阻塞
size_t Printf_DMA(const char* format, ...);      // DMA
size_t Printf_IT(const char* format, ...);       // 中断
```

## debug/vofa

VOFA+ 上位机协议。

```c
#define Vofa_Printf(format, ...)
#define Vofa_FireWater(format, ...)
size_t Vofa_JustFloat(Float_t* _data, size_t _num);
```

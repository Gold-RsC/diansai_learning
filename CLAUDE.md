# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目用途

**电赛电源方向的软件学习项目。** 用 2023 年电赛 A 题（单相逆变器并联运行系统）作为载体，把逆变器软件涉及的算法逐个写成独立的、可复用的 C 模块，供备赛时按需取用。

`template/` 是**模块化的算法模板，不是可直接烧录的固件**：没有构建系统、没有 HAL 配置、没有 `main.c`。`driver/` 与 `print_adapt` 依赖外部 STM32 工程的 HAL（`stm32g4xx_hal.h`、CubeMX 生成的 `hrtim.h` / `adc.h`、`huart4`），这些都不在此仓库内。学习重点在算法实现与模块接口设计，不在工程集成。

`docs/` 是原理笔记（SPWM、单极性/双极性对比、题目资料）。`template/filter/harmonic/谐波抑制滤波器.md` 是该模块的完整数学推导，文档与代码同放。

## 模块结构与数据流

```
mbase/          基础类型与宏，所有模块的根依赖
driver/         MCU 外设驱动封装 —— hrtim / adc（依赖具体芯片的 HAL）
dsp/            通用 DSP 原语 —— notch / goertzel / lpf
filter/         通用滤波器 —— harmonic / kalman / sliding
control/
  common/       与拓扑无关的控制原语 —— pid / soft_start / protect
  dc/           直流量场合 —— dcdc / cv_cc / mppt
  ac/           交流量场合 —— pr / droop / spwm
measure/
  dc/           直流量测量 —— dc_meter
  ac/           交流量测量 —— sin_analyzer / spll / thd
debug/          调试输出与上位机 —— print_adapt / vofa
```

模块按**信号域**分组：`dc/` 放直流量场合的东西（DC-DC、恒流充电、光伏），
`ac/` 放交流量场合（逆变、并网、并联）。找模块时先想属于哪个域。

`UART_ADDR` 等实现细节见 `template/doc.md`（全模块 API 速查）。

**实际连起来的只有 `control/` 和 `measure/`**，其余模块目前是孤立的：

- `SPWM_Update_Open(spwm)` —— 开环，仅靠相位累加器推进，每次调用推进一个 PWM 周期，所以**必须按 `pwm_freq` 的节奏调用**（PWM 中断里）。
- `SPWM_Update(spwm, v, i)` —— 闭环。`Sin_Analyzer_Update` 逐点累加，`data_ready` 置位时用 PI 修正调制比 `max_amplitude`。
- `SPWM_PLL_Update(pll, grid_v, v, i)` —— 用 `SPLL_1Ph_Sogi_Update(grid_v)` 锁出相位，取 `spll->out.theta` 作为调制相位，与电网同相。

两个值得注意的设计：

- `max_amplitude` 就是调制比本身：`SPWM_Update` / `SPWM_PLL_Update` 把 `PI_Update` 的返回值**直接赋给**它，限幅在 PI 内部按 `[0.02, 0.98]` 完成，`PI_Init` 的 `init_value` 取 0.8（与开环起始调制比一致）。**电压环 PI 的增益量纲是「调制比 / V」**，数值必须远小于把它当增量用时的取值——实测 `kp = 0.08` 会让调制比在 0.02 与 0.98 之间满幅打摆，现在取 `kp = 0.01`、`ki = 0.02`。`kp` 项作用于 `(e − e_prev)` 本质是微分，而 `Sin_Analyzer` 的窗口是整整一个工频周期，测量滞后大，所以 `kp` 一超过 `1/P`（P = 被控对象「Vrms / 调制比」增益）闭环必发散。
- `filter/`（harmonic、kalman、sliding）**当前零调用点**。它们曾经由一层 `Filter_t`（kalman→harmonic 级联）聚合，那层已在 `efb036b` 删除。

## 代码规范

对既有模块做修改或新增模块时必须遵守：

- **浮点一律用 `Float_t`**（`mbase.h` 中 `typedef float Float_t;`），不写裸 `float`。
- **函数命名 `Xxx_Yyy_Init` / `Xxx_Yyy_Update`**，类型名 `Xxx_Yyy_t`。
- **结构体固定三段**：`struct { ... } param;`、`struct { ... } _state;`、最后 `Float_t out;`。
- 输出统一用 `mbase.h` 的 `clamp(x, out_min, out_max)` 限幅。
- 连续赋值用空格对齐（clang-format 的 AlignConsecutiveAssignments 风格），4 空格缩进，K&R 大括号。
- Doxygen 中文注释：`@brief` / `@param` / `@return`，必要时加 `@note`。
- 头文件保护宏 `__XXX_H__`，与文件基本名一致（如 `notch.h` → `__NOTCH_H__`）。

## 编码（最容易踩的坑）

**大部分源文件是 GBK 编码 + CRLF 换行**（`file` 报 `ISO-8859 text, with CRLF line terminators` 即是）。中文注释以 GBK 字节直接存储。

**两个例外：`measure/spll/spll_1ph_sogi.{c,h}` 是 UTF-8 + LF，`debug/vofa/vofa.h` 是 UTF-8 + CRLF。改动这两个文件时不要"顺手统一"换行符。**

改 GBK 文件时：

- `Edit` 工具匹配的是 UTF-8 视图，**碰不了其中的中文串**；任何 decode/re-encode 路径（含 VS Code 直接保存、`iconv | sed | iconv`）都会把注释变成乱码。
- 要改的标识符都是纯 ASCII，用 Python 按字节操作即可，全程不解码：`open(p,'rb')` → 断言锚点唯一 → `replace` → `open(p,'wb')`。行尾保持 `\r\n`。
- 改完务必校验非 ASCII 字节与改动前**逐字节相同**，这是中文注释存活的唯一可靠证据。
- 仓库设了 `core.autocrlf`：`git show HEAD:path > path` 得到的是 **LF** 版本，写回工作区后 git 会报 modified，需补回 CRLF（`sed -i 's/\r$//; s/$/\r/'`）。

## 构建与验证

**没有构建系统**（无 Makefile / CMake / Keil 工程），验证靠手工 gcc（MinGW gcc 8.1.0）。所有 include 都是裸 `#include "base_name.h"`，且全树无重名头文件，所以 `-I` 顺序无关，但必须齐全：

```bash
INC="-Itemplate/mbase \
     -Itemplate/driver/hrtim -Itemplate/driver/adc \
     -Itemplate/dsp/notch -Itemplate/dsp/goertzel -Itemplate/dsp/lpf \
     -Itemplate/filter/harmonic -Itemplate/filter/kalman -Itemplate/filter/sliding \
     -Itemplate/control/common/pid -Itemplate/control/common/soft_start -Itemplate/control/common/protect \
     -Itemplate/control/dc/dcdc -Itemplate/control/dc/cv_cc -Itemplate/control/dc/mppt \
     -Itemplate/control/ac/pr -Itemplate/control/ac/droop -Itemplate/control/ac/spwm \
     -Itemplate/measure/dc/dc_meter \
     -Itemplate/measure/ac/sin_analyzer -Itemplate/measure/ac/spll -Itemplate/measure/ac/thd \
     -Itemplate/debug/print_adapt -Itemplate/debug/vofa \
     -Itmp/stub"                     # HAL 桩目录，见下
for f in $(find template -name '*.c'); do
  gcc -fsyntax-only -std=gnu11 -Wall -Wextra -Werror=implicit-function-declaration $INC "$f"
done
```

- `-std=gnu11` 必需：`debug/vofa/vofa.h` 用了 GNU 的 `##__VA_ARGS__` 扩展。
- `-Werror=implicit-function-declaration` 是价值最高的一个标志：漏改调用点在 gcc 里默认只是 warning，会"编译通过"但在 ARM 上静默返回垃圾值。
- 这套 `-I` 齐全时 **24 个 `.c` 全部干净通过**（零 warning）。

**`-Itmp/stub` 是必需的**（`tmp/` 在 `.gitignore` 里，桩不在仓库内）。3 个头文件直接 `#include "stm32g4xx_hal.h"`：`driver/hrtim/config_hrtim.h`、`driver/adc/config_adc.h`、`debug/print_adapt/print_adapt.h`；牵连 4 个 `.c`（`config_hrtim.c`、`config_adc.c`、`print_adapt.c`，以及经 `vofa.h → print_adapt.h` 的 `vofa.c`）。此外 `config_hrtim.h` 还要 CubeMX 生成的 `"hrtim.h"`、`config_adc.h` 还要 `"adc.h"`。缺了这三个头，整棵树会停在 `fatal error: stm32g4xx_hal.h: No such file or directory`，24 个 `.c` 里 4 个编不过。

桩只需**符号名与真实 HAL 对齐**，字段布局不必真实：

- 句柄：`HRTIM_HandleTypeDef`、`ADC_HandleTypeDef`、`UART_HandleTypeDef`，以及 `hhrtim1` / `hadc1..5` / `huart4`。
- 函数：`HAL_HRTIM_WaveformCountStart`（**不是** `CounterStart`）、`HAL_HRTIM_WaveformOutputStart`、`HAL_ADCEx_Calibration_Start(hadc, SingleDiff)`（G4 上是**两参数**）、`HAL_ADC_Start_DMA` / `HAL_ADC_Stop_DMA`、`HAL_UART_Transmit{,_DMA,_IT}`。
- 宏：`__HAL_HRTIM_SETCOMPARE`（**没有** `HAL_HRTIM_WaveformSetCompare` 这个函数）、`__HAL_HRTIM_MASTER_{ENABLE,DISABLE}_IT`、`__HAL_HRTIM_TIMER_{ENABLE,DISABLE}_IT`，以及 `HRTIM_TIMERID_*` / `HRTIM_OUTPUT_T*` / `HRTIM_MDIER_*` / `HRTIM_TIM_IT_*` / `HRTIM_COMPAREUNIT_*` / `HRTIM_TIMERINDEX_*` 枚举与 `ADC_SINGLE_ENDED`、`VREFINT_CAL_ADDR`、`VREFINT_CAL_VREF`。
- 桩里的 `__HAL_*` 宏写成 `((void)(...), ...)` 丢弃参数即可——它们只用来让仓库自身的代码通过语法检查，不校验 HAL 内部实现。

**`harmonic.h` 依赖 `template/dsp/notch` 在 include 路径上。** 上面这套 `-I` 已经带上；集成到 Keil 工程时必须同样补上。

## 问题记录：写进 `Issues.md`

发现的问题**不要自己动手改**，写进仓库根的 `Issues.md`，由用户自己修。

- 一条一个问题，**三行内说清**：在哪（`文件` 或 `文件:行`）、什么现象、怎么改。
- 只写仍然存在的。改掉的直接删，不留 changelog、不留"已修复"记录。
- `Issues.md` 是待修问题的唯一事实来源，本节不再另列清单。
- **改代码前先读 `Issues.md`**，免得把已知问题当成正常行为、或在不知情下重新引入。

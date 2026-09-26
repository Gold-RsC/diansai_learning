# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目用途

**电赛电源方向的软件学习项目。** 用 2023 年电赛 A 题（单相逆变器并联运行系统）作为载体，把逆变器软件涉及的算法逐个写成独立的、可复用的 C 模块，供备赛时按需取用。

`template/` 是**模块化的算法模板，不是可直接烧录的固件**：没有构建系统、没有 HAL 配置、没有 `main.c`。`print_adapt` 依赖的 `huart4` 与 `HAL_UART_*` 来自外部 STM32 工程，不在此仓库内。学习重点在算法实现与模块接口设计，不在工程集成。

`docs/` 是原理笔记（SPWM、单极性/双极性对比、题目资料）。`template/filter/harmonic/谐波抑制滤波器.md` 是该模块的完整数学推导，文档与代码同放。

## 模块结构与数据流

```
mbase/          基础类型与宏，所有模块的根依赖
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

- `max_amplitude` 既是调制比又是 PI 的积分输出——`PI_Update` 的输出直接加在它上面再限幅回 `[0.02, 0.98]`。PI 的 `out_min/out_max` 因此设成对称的 `±0.3`，是"增量"而非绝对量。
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
     -Itemplate/dsp/notch -Itemplate/dsp/goertzel -Itemplate/dsp/lpf \
     -Itemplate/filter/harmonic -Itemplate/filter/kalman -Itemplate/filter/sliding \
     -Itemplate/control/common/pid -Itemplate/control/common/soft_start -Itemplate/control/common/protect \
     -Itemplate/control/dc/dcdc -Itemplate/control/dc/cv_cc -Itemplate/control/dc/mppt \
     -Itemplate/control/ac/pr -Itemplate/control/ac/droop -Itemplate/control/ac/spwm \
     -Itemplate/measure/dc/dc_meter \
     -Itemplate/measure/ac/sin_analyzer -Itemplate/measure/ac/spll -Itemplate/measure/ac/thd \
     -Itemplate/debug/print_adapt -Itemplate/debug/vofa"
for f in $(find template -name '*.c'); do
  gcc -fsyntax-only -std=gnu11 -Wall -Wextra -Werror=implicit-function-declaration $INC "$f"
done
```

- `-std=gnu11` 必需：`debug/vofa/vofa.h` 用了 GNU 的 `##__VA_ARGS__` 扩展。
- `-Werror=implicit-function-declaration` 是价值最高的一个标志：漏改调用点在 gcc 里默认只是 warning，会"编译通过"但在 ARM 上静默返回垃圾值。
- `debug/print_adapt/print_adapt.c` 在仓库外无法单独编译（缺 `huart4` 与 `HAL_UART_*`），需自建最小桩。

**`harmonic.h` 依赖 `template/dsp/notch` 在 include 路径上。** 仓库内没有消费者所以编译测试抓不到这条，集成到 Keil 工程时必须补上。

## 已知问题

- `control/spwm/spwm.c` 的 `SPWM_PLL_Update` 中，失锁保护写作 `if (diff(spll->out.fo, GRID_FREQUENCY) < 0.01f) return 0.5f;`——条件是 `fo` **接近** 50Hz 时返回直通占空比，方向疑似写反（应为 `>`，即失锁时才旁路）。
- `filter/sliding/sliding.c` 的 `out` 同时充当递推累加器与对外输出，且被 `clamp` 截断。递推要求累加器未截断，因此输入一旦越出 `[out_min, out_max]` 范围，滤波器会卡在限幅值无法恢复。信号始终在范围内时不会触发。

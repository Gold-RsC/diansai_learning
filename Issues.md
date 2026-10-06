# Issues

待修问题清单。一条一个问题，**三行内说清：在哪 / 什么现象 / 怎么改**。
改掉的直接删，不留 changelog。

---

## 1. SPWM_PLL_Update 的失锁保护方向反了

`template/control/ac/spwm/spwm.c` —— `diff(spll->out.fo, GRID_FREQUENCY) < 0.01f` 时 `return 0.5f`。
这是 **fo ≈ 50Hz（已锁定）** 时旁路、失锁时反而正常输出，反了。`<` 改 `>`。

## 2. Sin_Analyzer 在零电流时永不输出

`template/measure/ac/sin_analyzer/sin_analyzer.c` —— 过零检测判的是 `current_sample`，
`data_ready` 只靠它翻转触发。空载（电流恒 0）时符号永不翻转 → `out.rms_voltage` 一直不更新
→ 挂在后面的 SPWM 电压环完全不动作。改成判 `voltage_sample`，或电流为 0 时回退用电压。

## 3. sliding 的 out 兼作累加器，会被 clamp 卡死

`template/filter/sliding/sliding.c` —— `out` 既是递推累加器又是对外输出，每步还被 `clamp` 截断。
累加器一旦被截断递推就错了：输入越出 `[out_min, out_max]` 后滤波器卡在限幅值不恢复。
把累加量挪进 `_state` 单独存，`out` 只做对外输出。

## 4. driver 缺 CubeMX 生成的两个头文件

`template/driver/hrtim/config_hrtim.h` 要 `hrtim.h`、`template/driver/adc/config_adc.h` 要 `adc.h`，
两个文件都不在仓库里，单独编译直接停在 fatal error。
补一份最小声明（句柄 `hhrtim1` / `hadc1..5` + 用到的枚举）即可自洽。

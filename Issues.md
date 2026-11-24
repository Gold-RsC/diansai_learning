# Issues

待修问题清单。一条一个问题，**三行内说清：在哪 / 什么现象 / 怎么改**。
改掉的直接删，不留 changelog。

---

## 1. sliding 的 out 兼作累加器，会被 clamp 卡死

`template/filter/sliding/sliding.c` —— `out` 既是递推累加器又是对外输出，每步还被 `clamp` 截断。
累加器一旦被截断递推就错了：输入越出 `[out_min, out_max]` 后滤波器卡在限幅值不恢复。
把累加量挪进 `_state` 单独存，`out` 只做对外输出。

## 2. driver 缺 CubeMX 生成的两个头文件

`template/driver/hrtim/config_hrtim.h` 要 `hrtim.h`、`template/driver/adc/config_adc.h` 要 `adc.h`，
两个文件都不在仓库里，单独编译直接停在 fatal error。
补一份最小声明（句柄 `hhrtim1` / `hadc1..5` + 用到的枚举）即可自洽。

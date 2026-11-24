#include "spll_1ph_sogi.h"

void SPLL_1Ph_Sogi_Init(SPLL_1ph_Sogi_t* analyzer,
                        Float_t grid_freq,
                        Float_t isr_freq,
                        Float_t omega_n,
                        Float_t zeta)
{
    // 清零整个结构体，避免未初始化数据干扰
    memset(analyzer, 0, sizeof(SPLL_1ph_Sogi_t));

    // 保存用户配置参数
    analyzer->param.grid_freq = grid_freq;
    analyzer->param.isr_freq  = isr_freq;
    analyzer->param.omega_n   = omega_n;
    analyzer->param.zeta      = zeta;

    // 计算采样时间间隔
    analyzer->_state.delta_t = 1.0f / isr_freq;

    // 计算 SOGI 与环路滤波器内部系数
    SPLL_1Ph_Sogi_Coeff_Calc(analyzer);
}

void SPLL_1Ph_Sogi_Reset(SPLL_1ph_Sogi_t* analyzer)
{
    // 清零输入数据缓冲区
    analyzer->_state.u[0] = 0.0f;
    analyzer->_state.u[1] = 0.0f;
    analyzer->_state.u[2] = 0.0f;

    // 清零 SOGI 直接通路输出缓冲区
    analyzer->_state.osg_u[0] = 0.0f;
    analyzer->_state.osg_u[1] = 0.0f;
    analyzer->_state.osg_u[2] = 0.0f;

    // 清零 SOGI 正交通路输出缓冲区
    analyzer->_state.osg_qu[0] = 0.0f;
    analyzer->_state.osg_qu[1] = 0.0f;
    analyzer->_state.osg_qu[2] = 0.0f;

    // 清零环路滤波器数据
    analyzer->_state.ylf[0]    = 0.0f;
    analyzer->_state.ylf[1]    = 0.0f;
    analyzer->_state.err_prev  = 0.0f;

    // 清零输出量
    analyzer->out.fo     = 0.0f;
    analyzer->out.theta  = 0.0f;
    analyzer->out.sine   = 0.0f;
    analyzer->out.cosine = 0.0f;
    analyzer->out.u_Q    = 0.0f;
    analyzer->out.u_D    = 0.0f;
}

void SPLL_1Ph_Sogi_Coeff_Calc(SPLL_1ph_Sogi_t* analyzer)
{
    // 计算角频率 ω = 2 * π * fn
    Float_t wn = analyzer->param.grid_freq * 2.0f * MATH_PI;

    // SOGI 增益系数 k，典型值取 0.5
    // k 影响 SOGI 的带宽：k 越大，带宽越宽，动态响应越快，但滤波效果变差
    analyzer->_state.osg_k = 0.5f;

    // 计算中间变量 x = ω * Ts
    analyzer->_state.osg_x = wn * analyzer->_state.delta_t;

    // 计算中间变量 y = (ω * Ts)^2
    analyzer->_state.osg_y = analyzer->_state.osg_x * analyzer->_state.osg_x;

    // 计算归一化因子 temp = 1 / (x + y + 4)
    // 该因子来源于双线性变换（Tustin 变换）离散化 SOGI 传递函数
    Float_t temp = 1.0f / (analyzer->_state.osg_x + analyzer->_state.osg_y + 4.0f);

    // OSG 直接通路分子系数
    // 直接通路传递函数实现同相信号提取
    analyzer->_state.osg_b0 = analyzer->_state.osg_x * temp;
    analyzer->_state.osg_b2 = -analyzer->_state.osg_b0;

    // OSG 分母系数（直接通路与正交通路共用分母）
    analyzer->_state.osg_a1 = (2.0f * (4.0f - analyzer->_state.osg_y)) * temp;
    analyzer->_state.osg_a2 = (analyzer->_state.osg_x - analyzer->_state.osg_y - 4.0f) * temp;

    // OSG 正交通路分子系数
    // 正交通路传递函数实现 90° 相移信号提取
    analyzer->_state.osg_qb0 = (0.5f * analyzer->_state.osg_y) * temp;
    analyzer->_state.osg_qb1 = analyzer->_state.osg_qb0 * 2.0f;
    analyzer->_state.osg_qb2 = analyzer->_state.osg_qb0;

    // 环路滤波器整定：二阶系统，误差已按幅值归一化
    // kp = 2*zeta*omega_n/(2*pi)，ki = omega_n^2/(2*pi)
    // 离散形式 Ylf(z) = (b0 + b1*z^-1)/(1 - z^-1)，其中 b0 = kp + ki*Ts，b1 = -kp
    Float_t kp = 2.0f * analyzer->param.zeta * analyzer->param.omega_n / (2.0f * MATH_PI);
    Float_t ki = analyzer->param.omega_n * analyzer->param.omega_n / (2.0f * MATH_PI);

    analyzer->_state.lpf_b0 = kp + ki * analyzer->_state.delta_t;
    analyzer->_state.lpf_b1 = -kp;
}

void SPLL_1Ph_Sogi_Update(SPLL_1ph_Sogi_t* analyzer, Float_t ac_voltage)
{
    // -------------------------------------------------------------------------
    // 步骤 1：更新当前输入电压采样值到缓冲区
    // -------------------------------------------------------------------------
    analyzer->_state.u[0] = ac_voltage;

    // -------------------------------------------------------------------------
    // 步骤 2：SOGI 正交信号发生器（OSG）
    // -------------------------------------------------------------------------
    // SOGI 通过二阶广义积分器生成一对正交信号 (u, qu)
    // 其中 osg_u 为同相信号（与输入同相），osg_qu 为正交信号（滞后 90°）

    // 直接通路：生成同相信号 osg_u[n]
    analyzer->_state.osg_u[0] =
          analyzer->_state.osg_b0 * (analyzer->_state.u[0] - analyzer->_state.u[2])
        + analyzer->_state.osg_a1 * analyzer->_state.osg_u[1]
        + analyzer->_state.osg_a2 * analyzer->_state.osg_u[2];

    // 正交通路：生成正交信号 osg_qu[n]（滞后 osg_u 90°）
    analyzer->_state.osg_qu[0] =
          analyzer->_state.osg_qb0 * analyzer->_state.u[0]
        + analyzer->_state.osg_qb1 * analyzer->_state.u[1]
        + analyzer->_state.osg_qb2 * analyzer->_state.u[2]
        + analyzer->_state.osg_a1 * analyzer->_state.osg_qu[1]
        + analyzer->_state.osg_a2 * analyzer->_state.osg_qu[2];

    // 数据移位：为下一次采样做准备，将当前值移入历史缓冲区
    analyzer->_state.osg_u[2] = analyzer->_state.osg_u[1];
    analyzer->_state.osg_u[1] = analyzer->_state.osg_u[0];

    analyzer->_state.osg_qu[2] = analyzer->_state.osg_qu[1];
    analyzer->_state.osg_qu[1] = analyzer->_state.osg_qu[0];

    analyzer->_state.u[2] = analyzer->_state.u[1];
    analyzer->_state.u[1] = analyzer->_state.u[0];

    // -------------------------------------------------------------------------
    // 步骤 3：Park 变换（αβ → dq）
    // -------------------------------------------------------------------------
    // 将 SOGI 生成的正交信号 (osg_u, osg_qu) 视为 αβ 坐标系下的分量
    // 通过 Park 变换投影到旋转的 dq 坐标系上
    // 当锁相环锁定时，Q 轴分量 u_Q 应趋近于 0

    analyzer->out.u_Q = analyzer->out.cosine * analyzer->_state.osg_u[0]
                  + analyzer->out.sine   * analyzer->_state.osg_qu[0];

    analyzer->out.u_D = analyzer->out.cosine * analyzer->_state.osg_qu[0]
                  - analyzer->out.sine   * analyzer->_state.osg_u[0];

    // -------------------------------------------------------------------------
    // 步骤 4：归一化相位误差
    // -------------------------------------------------------------------------
    // u_Q = A*sin(θ-θref)，u_D = -A*cos(θ-θref)
    // err = u_Q / sqrt(u_Q^2 + u_D^2) = sin(θ-θref)，与输入幅值无关

    Float_t mag = sqrtf(analyzer->out.u_Q * analyzer->out.u_Q
                      + analyzer->out.u_D * analyzer->out.u_D);
    Float_t err = (mag > 1e-6f) ? (analyzer->out.u_Q / mag) : 0.0f;

    // -------------------------------------------------------------------------
    // 步骤 5：环路滤波器（PI 型）
    // -------------------------------------------------------------------------
    // 传递函数：Ylf(z) = (b0 + b1*z^-1) / (1 - z^-1)
    // b0 = kp + ki*Ts，b1 = -kp，err 已归一化

    analyzer->_state.ylf[0] = analyzer->_state.ylf[1]
                        + analyzer->_state.lpf_b0 * err
                        + analyzer->_state.lpf_b1 * analyzer->_state.err_prev;

    // 更新环路滤波器历史值
    analyzer->_state.ylf[1]   = analyzer->_state.ylf[0];
    analyzer->_state.err_prev = err;

    // -------------------------------------------------------------------------
    // 步骤 6：VCO 压控振荡器
    // -------------------------------------------------------------------------
    // 将环路滤波器输出叠加到额定频率上，得到实际输出频率
    // 对频率积分得到相位角，并计算对应的正弦和余弦值

    // 输出频率 = 额定频率 + 频率修正量
    analyzer->out.fo = analyzer->param.grid_freq + analyzer->_state.ylf[0];

    // 相位角积分：θ[n] = θ[n-1] + 2π * fo * Ts
    analyzer->out.theta += analyzer->out.fo * analyzer->_state.delta_t * 2.0f * MATH_PI;

    // 相位角限幅：保持在 [0, 2π) 范围内
    if (analyzer->out.theta >= 2.0f * MATH_PI) {
        analyzer->out.theta -= 2.0f * MATH_PI;
    }

    // 计算相位角的正弦和余弦值，供下一次 Park 变换使用
    analyzer->out.sine   = sinf(analyzer->out.theta);
    analyzer->out.cosine = cosf(analyzer->out.theta);
}

#include <stdio.h>

// PID控制器结构体定义
typedef struct {
    double Kp;         // 比例增益
    double Ki;         // 积分增益
    double Kd;         // 微分增益
    double setpoint;   // 设定温度

    double error;      // 当前误差
    double last_error; // 上一个时间步的误差
    double integral;   // 积分误差累积
} PIDController;

// 初始化PID控制器
void PID_Init(PIDController *pid, double Kp, double Ki, double Kd, double setpoint) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->setpoint = setpoint;

    pid->error = 0.0;
    pid->last_error = 0.0;
    pid->integral = 0.0;
}

// 更新PID控制器并计算控制信号
double PID_Update(PIDController *pid, double measured_value) {
    // 计算当前误差
    pid->error = pid->setpoint - measured_value;

    // 计算比例控制项
    double P = pid->Kp * pid->error;

    // 计算积分控制项
    pid->integral += pid->error;
    double I = pid->Ki * pid->integral;

    // 计算微分控制项
    double D = pid->Kd * (pid->error - pid->last_error);

    // 计算PID输出
    double output = P + I + D;

    // 更新上一个时间步的误差
    pid->last_error = pid->error;

    return output;
}

// 模拟温度控制系统
int main() {
    double setpoint_temp = 25.0; // 设定温度

    // PID参数
    double Kp = 0.6;
    double Ki = 0.1;
    double Kd = 0.2;

    // 创建PID控制器
    PIDController pid;
    PID_Init(&pid, Kp, Ki, Kd, setpoint_temp);

    // 模拟温度测量和控制
    double current_temp = 20.0; // 初始温度

    // 模拟时间步
    int time_steps = 50;
    for (int i = 0; i < time_steps; ++i) {
        // 更新PID控制器并获取控制信号
        double control_signal = PID_Update(&pid, current_temp);

        // 模拟加热系统的响应（调整当前温度）
        current_temp += control_signal;

        // 输出当前温度和控制信号
        printf("Current Temperature: %.2f °C, Control Signal: %.2f\n", current_temp, control_signal);
    }

    return 0;
}

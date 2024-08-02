#include <Arduino.h>
#include <stdbool.h>

/* Pin */ 
const int Right_SensorB = 3;  // Sensor I/O
const int Right_SensorA = 2;
const int Middle_Sensor = 10;
const int Left_SensorA = 11;
const int Left_SensorB = 13;

const int Right_Back_Wheel = 6;
const int Right_Front_Wheel = 7;
const int Left_Back_Wheel = 4;
const int Left_Front_Wheel = 5;

/* Speed Const */
const int LOWER = 220;  //完全停下
const int HIGHER = 130;
const int TRANS = 140;
const int DASH = 100;
const int DEL = 30;

const int cursor_speed = 120;

#define LOW_SPEED 210

/**
    variable
*/ 

// 判断I/O口传感器信号状态,白色为0，黑色为1
int rr = 0;
int rm = 0;
int md = 0;
int lm = 0;
int ll = 0;

bool last_dir = true;  //右转为真，左转为假

int current_status = 0;




/* Set up */
void setup() {
/**
* Wheel Motor Setup
*/
  pinMode(Right_Back_Wheel, OUTPUT);  //right front wheel
  pinMode(Right_Front_Wheel, OUTPUT);  //right back wheel
  pinMode(Left_Back_Wheel, OUTPUT);  //left front wheel
  pinMode(Left_Front_Wheel, OUTPUT);  //left back wheel

/**
* Sensor I/O setup
*/
  pinMode(Right_SensorB, INPUT);  
  pinMode(Right_SensorA, INPUT);
  pinMode(Middle_Sensor, INPUT);
  pinMode(Left_SensorA, INPUT);
  pinMode(Left_SensorB, INPUT);
}

// double PID_Update(PIDController *pid, double sensor_value[]) {
//     // 计算当前误差（假设传感器按顺序从左到右编号）
//     pid->error = sensor_value[2] - pid->setpoint; // 使用中间传感器的值作为当前误差

//     // 计算比例控制项
//     double P = pid->Kp * pid->error;

//     // 计算积分控制项
//     pid->integral += pid->error;
//     double I = pid->Ki * pid->integral;

//     // 计算微分控制项
//     double D = pid->Kd * (pid->error - pid->last_error);

//     // 计算PID输出
//     double output = P + I + D;

//     // 更新上一个时间步的误差
//     pid->last_error = pid->error;

//     return output;
// }

// int Velocity_A = 0;
// int Count_A = 0;
// int Velocity_B = Count_B; //电机B的速度计算
// int Count_B = 0;



// /**********定时器中断触发函数*********/
// void control()
// {       
//         //Velocity=Count;    //把采用周期(内部定时中断周期)所累计的脉冲下降沿的个数,赋值给速度
//         //Count=0;           //将脉冲计数器清零
//         //value=Incremental_PI_A(Velocity,Target);  //通过目标值和当前值在这个函数下算出我们需要调整用的PWM值
//         //Set_PWM(value);    //将算好的值输出给电机
//         Velocity_A = Count_A; //电机A的速度计算
//         Count_A = 0; //清零计数器A
//         Velocity_B = Count_B; //电机B的速度计算
//         Count_B = 0; //清零计数器B
//         int value_A = Incremental_PI_A(Velocity_A, Target_A); //电机A的PWM计算
//         int value_B = Incremental_PI_B(Velocity_B, Target_B); //电机B的PWM计算
//         Set_PWM_A(value_A); //设置电机A的PWM
//         Set_PWM_B(value_B); //设置电机B的PWM
// }



// int Incremental_PI_B (int Encoder,float Target2)
// {  
//   // static float Bias,PWM=0,Last_bias=0;                    //定义全局静态浮点型变量 PWM,Bias(本次偏差),Last_bias(上次偏差)
//    Bias_B=Target2-Encoder;                                  //计算偏差,目标值减去当前值
//    PWM_B += Velocity_KP*(Bias_B-Last_bias_B)+Velocity_KI*Bias_B;   //增量式PI控制计算
   
//    if(PWM_B>PWM_Restrict)
//    PWM_B=PWM_Restrict;                                     //限幅
   
//    if(PWM_B<-PWM_Restrict)
//    PWM_B=-PWM_Restrict;                                    //限幅  
   
//    Last_bias_B=Bias_B;                                       //保存上一次偏差 
//   /*
//    Serial.print(PWM_B);
//    Serial.print(" ");
//    Serial.println(Encoder);
//    */
//    return PWM_B;                                           //增量输出
// }

float P = 0;
float I = 0;
float D = 0;
float PID = 0;

const float Kp = 3;
const float Ki = 0.05;
const float Kd = 0.025;

float previous_status = 0;

float PID_value = 0;

/**
  判断I/O口传感器信号状态,白色为0，黑色为1
*/
inline void Get_sensor_status(){
    rr = digitalRead(Right_SensorB);  // Left outside
    rm = digitalRead(Right_SensorA);  // Left inside
    md = digitalRead(Middle_Sensor);  // Middle
    lm = digitalRead(Left_SensorA);  // Right inside
    ll = digitalRead(Left_SensorB);  // Right outside

    // 0 0 1 0 0 
    if(rr == 0 && rm == 0 && md == 1 && lm == 0 && ll == 0 ){
      current_status = 0;
      I = 0;
    }
    else if(rr == 0 && rm == 1 && md == 1 && lm == 0 && ll == 0){
      current_status = -1;
    }
    else if(rr == 0 && rm == 1 && md == 0 && lm == 0 && ll == 0){
      current_status = -2;
    }
    else if(rr == 1 && rm == 1 && md == 0 && lm == 0 && ll == 0){
      current_status = -3;
    }
    else if(rr == 1 && rm == 0 && md == 0 && lm == 0 && ll == 0){
      current_status = -5;
    }

    else if(rr == 0 && rm == 0 && md == 1 && lm == 1 && ll == 0){
      current_status = 1;
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 1 && ll == 0){
      current_status = 2;
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 1 && ll == 1){
      current_status = 5;
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 0 && ll == 1){
      current_status = 5;
    }
    else{
      current_status = 20;
    }

    // current_status = 0;

    // if(rr == 1){
    //     current_status += 5;
    // }
    // if(rm == 1){
    //     current_status += 3;
    // }
    // if(lm == 1){
    //     current_status -= 3;
    // }
    // if(ll == 1){
    //     current_status -= 5;
    // }
    // if(md == 1){
    //     current_status = 0;
    // }

}



inline void calculate_pid(){
  P = current_status;
  I = I + current_status;
  D = current_status - previous_status;
 
  PID_value = (Kp * P) + (Ki * I) + (Kd * D);
 
  previous_status = current_status;
}

int Left_wheel_speed = 0;
int Right_wheel_speed = 0;

inline void Update_Speed(){
    
    Left_wheel_speed = cursor_speed + PID_value;
    Right_wheel_speed = cursor_speed - PID_value;

    if(Left_wheel_speed > LOW_SPEED){
        Left_wheel_speed = LOW_SPEED;
    }
    if(Left_wheel_speed < 20){
        Left_wheel_speed = 20;
    }

    if(Right_wheel_speed > LOW_SPEED){
        Right_wheel_speed = LOW_SPEED;
    }
    if(Right_wheel_speed < 20){
        Right_wheel_speed = 20;
    }

    digitalWrite(4,HIGH);
    analogWrite(5, Right_wheel_speed);
    digitalWrite(7,HIGH);
    analogWrite(6, Left_wheel_speed);
}


void loop() {

  Get_sensor_status();
  
  calculate_pid();

  Update_Speed();
/*
  电机控制笔记
以爪子为车头
右倒车：4-HIGH 5PWM，0最快
左倒车：7-HIGH 6PWM，0最快

右前进：4-LOW 5PWM，255最快
左前进：7-LOW 6PWM，255最快

  */
    // if(!ll && lm && rm && !rr)  //中间两个黑色，左右两个白色，前进11
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, DASH);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, DASH);
    //    last_dir=false;
    // }

    // else if(!ll && !lm && rm && !rr)  //中间两个右边黑色左边白，右转4
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, LOWER);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, HIGHER);
    //    delay(DEL);
    //    last_dir=true;

    // }

    // else if(!ll && lm && !rm && !rr)  //中间两个右边白左边黑，小左转3
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, HIGHER);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, LOWER);
    //    delay(DEL);
    //    last_dir=false;
    // }

    // else if(!ll && !lm && rm && rr)  //右边两个黑，右转7
    // {
    //    digitalWrite(4,LOW);
    //    analogWrite(5, TRANS);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, HIGHER);
    //    delay(DEL);
    //    last_dir=true;
    // }

    // else if(ll && lm && !rm && !rr)  //左边两个黑，左转6
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, HIGHER);
    //    digitalWrite(7,LOW);
    //    analogWrite(6, TRANS);
    //    delay(DEL);
    //    last_dir=false;
    // }

    // else if(ll && !lm && !rm && !rr)  //左边一个黑,大偏右，左转2？
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, HIGHER);
    //    digitalWrite(7,LOW);
    //    analogWrite(6, TRANS);
    //    delay(DEL);
    //    last_dir=false;
    // }

    // else if(!ll && !lm && !rm && rr)  //右边一个黑，大偏左，右转5?
    // {
    //    digitalWrite(4,LOW);
    //    analogWrite(5, TRANS);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, HIGHER);
    //    delay(DEL);
    //    last_dir=true;
    // }

    // else if(ll && lm && rm && !rr)  //左边三个黑，直角左转12
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, HIGHER);
    //    digitalWrite(7,LOW);
    //    analogWrite(6, TRANS);
    //    delay(DEL);
    //    last_dir=false;
    // }

    // else if(!ll && lm && rm && rr)  //右边三个黑，直角右转13
    // {
    //    digitalWrite(4,LOW);
    //    analogWrite(5, TRANS);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, HIGHER);
    //    delay(DEL);
    //    last_dir=true;
    // }

    // else if(ll && lm && rm && rr)  //全黑，优先右转
    // {
    //    digitalWrite(4,LOW);
    //    analogWrite(5, TRANS);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, HIGHER);
    //    delay(DEL);
    //    last_dir=true;
    // }
    
    // else if(!ll && !lm && !rm && !rr)  //全白，按照上一次转向来转
    // {
    //    if(last_dir)//左边
    //    {
    //     digitalWrite(4,LOW);
    //     analogWrite(5, TRANS);
    //     digitalWrite(7,HIGH);
    //     analogWrite(6, HIGHER);
    //    }
    //    else
    //    {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, HIGHER);
    //    digitalWrite(7,LOW);
    //    analogWrite(6, TRANS);
    //    }

    // }
    // else//其他情况，先直走
    // {
    //    digitalWrite(4,HIGH);
    //    analogWrite(5, DASH);
    //    digitalWrite(7,HIGH);
    //    analogWrite(6, DASH);
    // }
}

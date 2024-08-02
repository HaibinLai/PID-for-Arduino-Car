/*! \file Arduino_code.c */
#include <Arduino.h>
#include <stdbool.h>
#include <avr/wdt.h>
#include <MsTimer2.h>
#include <avr/power.h>

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

int cursor_speed = 100;

#define LOW_SPEED 210

#define HIGH_SPEED_LOW_DRAG 80

/**
    variable
*/ 

/*判断I/O口传感器信号状态,白色为0，黑色为1*/ 
int rr = 0;
int rm = 0;
int md = 0;
int lm = 0;
int ll = 0;

int last_dir = 1; 

int current_status = 0;


int max_speed = 30;

/*!
  \li Low drag
 
  \li This function takes turn the clock down;
 
  \return MsTimer2::stop()
 */
void Low_Speed(){
    max_speed = 80;
    cursor_speed += 40;
    clock_prescale_set(clock_div_8);
    MsTimer2::stop();   //关闭定时中断的函数
}



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

  wdt_enable(WDTO_8S); //开启看门狗，并设置溢出时间为8秒
  MsTimer2::set(400000,Low_Speed);   //400秒进入一次中断，中断函数是fals（）
  MsTimer2::start();     //开启定时中断函数

  clock_prescale_set(clock_div_2);

  digitalWrite(4,HIGH);
  analogWrite(5, 200);
  digitalWrite(7,HIGH);
  analogWrite(6, 200);
   
}



float P = 0;
float I = 0;
float I_angular = 0;
int counter = 0;

float D = 0;
float PID = 0;

const float Kp = 6;
const float Ki = 0.08;
const float Kd = 0.05;

float previous_status = 0;
float PID_value = 0;

int go_stright_value = 100;
int turn_right_value = 0;
int turn_left_value  = 0; 

/**
  判断I/O口传感器信号状态,白色为0，黑色为1
*/
inline void Get_sensor_status(){
    rr = digitalRead(Right_SensorB);  // Left outside
    rm = digitalRead(Right_SensorA);  // Left inside
    md = digitalRead(Middle_Sensor);  // Middle
    lm = digitalRead(Left_SensorA);  // Right inside
    ll = digitalRead(Left_SensorB);  // Right outside

    counter += 1;

    // 0 0 1 0 0 
    if(rr == 0 && rm == 0 && md == 1 && lm == 0 && ll == 0 ){
      current_status = 0;
      I = 0;
      go_stright_value += 1;
      // I_angular = 0;
      wdt_reset();
    }
    else if(rr == 0 && rm == 1 && md == 1 && lm == 0 && ll == 0){
      current_status = -1;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 0 && rm == 1 && md == 0 && lm == 0 && ll == 0){
      current_status = -3;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 1 && rm == 1 && md == 0 && lm == 0 && ll == 0){
      current_status = -5;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 1 && rm == 0 && md == 0 && lm == 0 && ll == 0){
      current_status = -9;
      last_dir = -20;
      go_stright_value = 0;
      wdt_reset();
    }

    else if(rr == 0 && rm == 0 && md == 1 && lm == 1 && ll == 0){
      current_status = 1;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 1 && ll == 0){
      current_status = 3;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 1 && ll == 1){
      current_status = 5;
      go_stright_value = 0;
      wdt_reset();
    }
    else if(rr == 0 && rm == 0 && md == 0 && lm == 0 && ll == 1){
      current_status = 9;
      go_stright_value = 0;
      last_dir = 20;
      wdt_reset();
    }
    else{
        current_status = 12;
    }

    if(counter > 4000){
      I = 0;
      counter = 0;
    }

}


inline void calculate_pid(){
  P = current_status;
  I = I + current_status;
  // I_angular = I_angular + current_status * 0.001;
  D = current_status - previous_status;
 
  PID_value = (Kp * P) + (Ki * I) + (Kd * D);
 
  previous_status = current_status;
}

int Left_wheel_speed = 0;
int Right_wheel_speed = 0;

inline void Update_Speed(){
    
    Left_wheel_speed = cursor_speed + PID_value;
    Right_wheel_speed = cursor_speed - PID_value;

    if(go_stright_value >= 500){
      Left_wheel_speed = HIGH_SPEED_LOW_DRAG;
      Right_wheel_speed = HIGH_SPEED_LOW_DRAG;
    }

    if(Left_wheel_speed > LOW_SPEED){
        Left_wheel_speed = LOW_SPEED;
    }
    if(Left_wheel_speed < max_speed){
        Left_wheel_speed = max_speed;
    }

    if(Right_wheel_speed > LOW_SPEED){
        Right_wheel_speed = LOW_SPEED;
    }
    if(Right_wheel_speed < max_speed){
        Right_wheel_speed = max_speed;
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

}

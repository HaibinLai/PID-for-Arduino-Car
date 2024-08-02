#include <Arduino.h>
#include <stdbool.h>

bool last_dir=true;//右转为真，左转为假


//加入
int trac1 = 14; //s1//定义四个传感器I/O口，从车头方向的最右边开始排序 (传感器)
int trac2 = 15; 
int trac3 = 16; 
int trac4 = 17; 
//

void setup(){
 

   pinMode(4, OUTPUT);  //右轮
  pinMode(5, OUTPUT);  //右轮
  pinMode(7, OUTPUT);  //左轮
  pinMode(6, OUTPUT);  //左轮
//加入
  pinMode(trac1, INPUT); //配置四个巡线传感器I/O口
  pinMode(trac2, INPUT);
  pinMode(trac3, INPUT);
  pinMode(trac4, INPUT);
  //
}

void loop(){
 

 
//模式为巡线模式，执行巡线代码
    const int LOWER=220;//完全停下
    const int HIGHER=130;
    const int TRANS=140;
    const int DASH=100;
    const int DEL=30;
    
      //判断四个I/O口传感器信号状态,白色为0，黑色为155
    int rr = digitalRead(trac1);//最右边
    int rm = digitalRead(trac2);//中右
    int lm = digitalRead(trac3);//中左
    int ll = digitalRead(trac4);//最左边
    //45右 67左
    
/*
  电机控制笔记
以爪子为车头
右倒车：4-HIGH 5PWM，0最快
左倒车：7-HIGH 6PWM，0最快

右前进：4-LOW 5PWM，255最快
左前进：7-LOW 6PWM，255最快

  */
    if(!ll && lm && rm && !rr)  //中间两个黑色，左右两个白色，前进11
    {
       digitalWrite(4,HIGH);
       analogWrite(5, DASH);
       digitalWrite(7,HIGH);
       analogWrite(6, DASH);
       last_dir=false;
    }

    else if(!ll && !lm && rm && !rr)  //中间两个右边黑色左边白，右转4
    {
       digitalWrite(4,HIGH);
       analogWrite(5, LOWER);
       digitalWrite(7,HIGH);
       analogWrite(6, HIGHER);
       delay(DEL);
       last_dir=true;

    }

    else if(!ll && lm && !rm && !rr)  //中间两个右边白左边黑，小左转3
    {
       digitalWrite(4,HIGH);
       analogWrite(5, HIGHER);
       digitalWrite(7,HIGH);
       analogWrite(6, LOWER);
       delay(DEL);
       last_dir=false;
    }

    else if(!ll && !lm && rm && rr)  //右边两个黑，右转7
    {
       digitalWrite(4,LOW);
       analogWrite(5, TRANS);
       digitalWrite(7,HIGH);
       analogWrite(6, HIGHER);
       delay(DEL);
       last_dir=true;
    }

    else if(ll && lm && !rm && !rr)  //左边两个黑，左转6
    {
       digitalWrite(4,HIGH);
       analogWrite(5, HIGHER);
       digitalWrite(7,LOW);
       analogWrite(6, TRANS);
       delay(DEL);
       last_dir=false;
    }

    else if(ll && !lm && !rm && !rr)  //左边一个黑,大偏右，左转2？
    {
       digitalWrite(4,HIGH);
       analogWrite(5, HIGHER);
       digitalWrite(7,LOW);
       analogWrite(6, TRANS);
       delay(DEL);
       last_dir=false;
    }

    else if(!ll && !lm && !rm && rr)  //右边一个黑，大偏左，右转5?
    {
       digitalWrite(4,LOW);
       analogWrite(5, TRANS);
       digitalWrite(7,HIGH);
       analogWrite(6, HIGHER);
       delay(DEL);
       last_dir=true;
    }

    else if(ll && lm && rm && !rr)  //左边三个黑，直角左转12
    {
       digitalWrite(4,HIGH);
       analogWrite(5, HIGHER);
       digitalWrite(7,LOW);
       analogWrite(6, TRANS);
       delay(DEL);
       last_dir=false;
    }

    else if(!ll && lm && rm && rr)  //右边三个黑，直角右转13
    {
       digitalWrite(4,LOW);
       analogWrite(5, TRANS);
       digitalWrite(7,HIGH);
       analogWrite(6, HIGHER);
       delay(DEL);
       last_dir=true;
    }

    else if(ll && lm && rm && rr)  //全黑，优先右转
    {
       digitalWrite(4,LOW);
       analogWrite(5, TRANS);
       digitalWrite(7,HIGH);
       analogWrite(6, HIGHER);
       delay(DEL);
       last_dir=true;
    }
    
    else if(!ll && !lm && !rm && !rr)  //全白，按照上一次转向来转
    {
       if(last_dir)//左边
       {
        digitalWrite(4,LOW);
        analogWrite(5, TRANS);
        digitalWrite(7,HIGH);
        analogWrite(6, HIGHER);
       }
       else
       {
       digitalWrite(4,HIGH);
       analogWrite(5, HIGHER);
       digitalWrite(7,LOW);
       analogWrite(6, TRANS);
       }

    }
    else//其他情况，先直走
    {
       digitalWrite(4,HIGH);
       analogWrite(5, DASH);
       digitalWrite(7,HIGH);
       analogWrite(6, DASH);
    }

}

 

#include <MsTimer2.h>         //定时中断头文件库
#include <PinChangeInterrupt.h>
#include <DFRobot_VL53L0X.h>
#include <Servo.h>     
#include <DFRobot_LiquidCrystal_I2C.h> //加载文件库

// 创建对象
/***********电机控制板引脚定义************/
unsigned int Motor_AIN1=6;        //控制电机的方向引脚  一定改成自己用的
unsigned int Motor_AIN2=3;        //控制电机的方向引脚  一定改成自己用的
unsigned int Motor_BIN1=11;        //控制电机的方向引脚  一定改成自己用的
unsigned int Motor_BIN2=5;        //控制电机的方向引脚  一定改成自己用的
unsigned int left1 = 51;
unsigned int left2 = 53;
unsigned int right1 = 47;
unsigned int right2 = 45;
unsigned int middle = 49;
unsigned int Buz = 43;
int pos = 90;
int xunxian[5] = {1,1,1,1,1};

//void control();
//void READ_ENCODER_A(); 
//void READ_ENCODER_B();

// String Target_Value;             //串口获取的速度字符串变量
// Servo myservo;
// DFRobot_VL53L0X vl53l0x;
// DFRobot_LiquidCrystal_I2C lcd1602;
int value;                       //用于存储通过PI控制器计算得到的用于调整电机转速的PWM值的整形变量 
int count;
float dis = 0;
float shownum = 0;
const int arrnum = 91;
float disarr[arrnum];
float min;
bool measure = false;
float percent = 0.2;
bool show;
/***********编码器引脚************/
#define ENCODER_A 2              //编码器A相引脚——————需要为中断引脚
#define ENCODER_B 7              //编码器B相引脚——————如果不4分频，可以不为中断引脚
#define ENCODER_C 4             //编码器A相引脚——————需要为中断引脚
#define ENCODER_D 8             //编码器B相引脚——————如果不4分频，可以不为中断引脚
#define KEY A4

int Velocity_A,Count_A=0;            //Count计数变量 Velocity存储设定时间内A相上升沿和下降沿的个数
int Velocity_B,Count_B=0;
int Default_speed = 7.5;
/***********PID控制器相关参数************/
float Velocity_KP =10, Velocity_KI= 10;
//Velocity_KP,Velocity_KI.PI参数 
volatile float Target_A=0;//目标值
volatile float Target_B=0;
static float Bias_A,PWM_A=0,Last_bias_A=0; 
static float Bias_B,PWM_B=0,Last_bias_B=0;  
/*********** 限幅************
*以下两个参数让输出的PWM在一个合理区间
*当输出的PWM小于50时电机不转 所以要设置一个启始PWM
*arduino mega 2560 单片机的PWM不能超过255 所以 PWM_Restrict 起到限制上限的作用
*****************************/
int startPWM=10;                 //初始PWM，暂时不用
int PWM_Restrict=255;            //startPW+PWM_Restric=255<256
bool runjudge;

// unsigned char My_click (void) {
//     static byte flag_key = 1; //按键按松开标志
//     if (flag_key && (digitalRead(KEY) == 0))   { //如果发生单击事件
//     flag_key = 0;  
//     Serial.println("Click~");
//     if (digitalRead(KEY) == 0)  return 1;    //M键
//   }
//   else if (digitalRead(KEY) == 1)     flag_key = 1;
//   return 0;//无按键按下
// }


// int getX(int num){
//   if((num / 1000) != 0){
//     return 0;
//   }else if(num / 100 != 0){
//     return 1;
//   }else if(num / 10 != 0){
//     return 2;
//   }else{
//     return 0;
//   }
// }

float Mini(float arr[]){
  float a;
  for (int count = 0; count < arrnum; count++){
    if (arr[count] > 30.0 && arr[count] < 2000.0){
      a = arr[count];
      break;
    }
    return 0.0;
  }
  for(int count = 0; count < arrnum; count++){
    float b = arr[count];
    if(b < a && b > 0.0){
     if(static_cast<float>(abs(abs(a)-abs(b)))/ static_cast<float>(a) <= percent)//防止尖峰突变值
     {
      a = b;
     }
    }
  }
  return a;
}

bool Decreasing(float arr[]){
  float a = arr[0];
  for(int count = 0; count < min(sizeof(arr), 5); count++){
    if(arr[count] > a){
      return false;
    }
    a = arr[count];
  }
  return true;
}
/***********初始化************/
void setup() 
{

  pinMode(ENCODER_A,INPUT);     //设置两个相线为输入模式
  pinMode(ENCODER_B,INPUT);
  pinMode(ENCODER_C,INPUT);     //设置两个相线为输入模式
  pinMode(ENCODER_D,INPUT);
  pinMode(Motor_AIN1,OUTPUT);   //设置两个驱动引脚为输出模式
  pinMode(Motor_AIN2,OUTPUT); 
  pinMode(Motor_BIN1,OUTPUT);   //设置两个驱动引脚为输出模式
  pinMode(Motor_BIN2,OUTPUT); 


  // myservo.attach(A9, 500, 2500);          //修正脉冲宽度(PWM引脚)
  // vl53l0x.begin();
	vl53l0x.setMode(vl53l0x.High, vl53l0x.Continuous);
  // lcd1602.begin(0x27);
  lcd1602.clear()
  
MsTimer2::set(10, control); //10毫秒定时中断函数
 MsTimer2::start ();        //中断使能 
  attachInterrupt(0, READ_ENCODER_A,CHANGE);      //开启对应2号引脚的0号外部中断,触发方式为FALLING 即下降沿触发,触发的中断函数为 READ_ENCODER_A 
  //attachPCINT(digitalPinToPCINT(0), READ_ENCODER_A,CHANGE);
  //attachInterrupt(4, READ_ENCODER_B,CHANGE);
  attachPCINT(16, READ_ENCODER_B,CHANGE);  //开启外部中断 编码器接口2
  runjudge = true;
  count = 0;
}
/***********主程序************/
void loop() 
{
  /*
  while(Serial.available()>0)          //检测串口是否接收到了数据
  {
    String Target_Value=Serial.readString();  //读取串口字符串
    int commaIndex = Target_Value.indexOf(','); // 查找逗号分隔符
    if (commaIndex != -1) {
      // 分割字符串获取两个目标值
      Target_A = Target_Value.substring(0, commaIndex).toFloat();
      Target_B = Target_Value.substring(commaIndex + 1).toFloat();
    //Target=Target_Value.toFloat();     //将字符串转换为浮点型,并将其赋给目标值
    Serial.print("Target A: ");
    Serial.println(Target_A);
    Serial.print("Target B: ");
    Serial.println(Target_B);
  }
}
*/
show = false;
lcd1602.printLine(uint32_t(1), "Distance:");
int arr[] = {digitalRead(left2), digitalRead(left1), digitalRead(middle), digitalRead(right1), digitalRead(right2)};
int i = 0;
for (int val : arr) {
  xunxian[i++] = val;
}



if(runjudge){
  run(xunxian);
}

if(show){

	for (pos = 90; pos <= 180; pos += 1) {       //pos+=1等价于pos=pos+1

    disarr[pos - 90] = dis;
    delay(15);
  }
  if(Decreasing){
    min = Mini(disarr);
  }else{
    measure = true;
  }
  for (pos = 90; pos >= 0; pos -= 1) {

    disarr[90 - pos] = dis;
    delay(15);
  }
  if(measure){
    min = Mini(disarr);
  }
  pos = 90;
  // myservo.write(pos);
  count = 1;

  float distance = min;
  //以下为LED显示正确距离的代码
  // if(distance <= 2000 && distance >= 30){//判断距离是否在量程内
    // shownum = (floor(distance) + round(distance * 100) % 100 * 0.01);
	  // lcd1602.print(uint32_t(getX(distance)), uint32_t(10), String(shownum));
	  // lcd1602.print(7, uint32_t(10), "mm");
    // if((shownum * 10) == (int)(shownum * 10)){
	  //   if((shownum) == (int)(shownum)){
	  //     lcd1602.print(4 , uint32_t(10), ".00mm");
	  //   }else{
	  //     lcd1602.print(6 , uint32_t(10), "0mm");
	  //   }
	  // }
  // }else{
  //   lcd1602.print(0, uint32_t(10), "Error!");
  // }

}
}
/**********外部中断触发计数器函数************
*根据转速的方向不同我们将计数器累计为正值或者负值(计数器累计为正值为负值为计数器方向)
*只有方向累计正确了才可以实现正确的调整,否则会出现逆方向满速旋转
*
*※※※※※※超级重点※※※※※※
*
*所谓累计在正确的方向即
*(1)计数器方向
*(2)电机输出方向(控制电机转速方向的接线是正着接还是反着接)
*(3)PI 控制器 里面的误差(Basi)运算是目标值减当前值(Target-Encoder),还是当前值减目标值(Encoder-Target)
*三个方向只有对应上才会有效果否则你接上就是使劲的朝着一个方向(一般来说是反方向)满速旋转

例子里是已经对应好的,如果其他驱动单片机在自己尝试的时候出现满速旋转就是三个方向没对应上

下列函数中由于在A相上升沿触发时,B相是低电平,和A相下降沿触发时B是高电平是一个方向,在这种触发方式下,我们将count累计为正,另一种情况将count累计为负
********************************************/
void READ_ENCODER_A() 
{
    if (digitalRead(ENCODER_A) ==0) 
    {     
     if (digitalRead(ENCODER_B) == LOW)      
       Count_A--;  //根据另外一相电平判定方向
     else      
       Count_A++;
    }
    else 
    {    
     if (digitalRead(ENCODER_B) == LOW)      
     Count_A++; //根据另外一相电平判定方向
     else      
     Count_A--;
    }
}

void READ_ENCODER_B() 
{
    if (digitalRead(ENCODER_C) ==0) 
    {     
     if (digitalRead(ENCODER_D) == LOW)      
       Count_B++;  //根据另外一相电平判定方向
     else      
       Count_B--;
    }
    else 
    {    
     if (digitalRead(ENCODER_D) == LOW)      
     Count_B--; //根据另外一相电平判定方向
     else      
     Count_B++;
    }
}
/**********定时器中断触发函数*********/
void control()
{       
        //Velocity=Count;    //把采用周期(内部定时中断周期)所累计的脉冲下降沿的个数,赋值给速度
        //Count=0;           //将脉冲计数器清零
        //value=Incremental_PI_A(Velocity,Target);  //通过目标值和当前值在这个函数下算出我们需要调整用的PWM值
        //Set_PWM(value);    //将算好的值输出给电机
        Velocity_A = Count_A; //电机A的速度计算
        Count_A = 0; //清零计数器A
        Velocity_B = Count_B; //电机B的速度计算
        Count_B = 0; //清零计数器B
        int value_A = Incremental_PI_A(Velocity_A, Target_A); //电机A的PWM计算
        int value_B = Incremental_PI_B(Velocity_B, Target_B); //电机B的PWM计算
        Set_PWM_A(value_A); //设置电机A的PWM
        Set_PWM_B(value_B); //设置电机B的PWM
}
/***********PI控制器****************/
int Incremental_PI_A (int Encoder,float Target1)
{  
  // static float Bias,PWM=0,Last_bias=0;                    //定义全局静态浮点型变量 PWM,Bias(本次偏差),Last_bias(上次偏差)
   Bias_A=Target1-Encoder;                                  //计算偏差,目标值减去当前值
   PWM_A += Velocity_KP*(Bias_A-Last_bias_A)+Velocity_KI*Bias_A;   //增量式PI控制计算
   
   if(PWM_A>PWM_Restrict)
   PWM_A=PWM_Restrict;                                     //限幅
   
   if(PWM_A<-PWM_Restrict)
   PWM_A=-PWM_Restrict;                                    //限幅  
   
   Last_bias_A=Bias_A;                                       //保存上一次偏差 
  /* 
   Serial.print(PWM_A);
   Serial.print(" ");
   Serial.println(Encoder);
   */
   return PWM_A;                                           //增量输出
}

int Incremental_PI_B (int Encoder,float Target2)
{  
  // static float Bias,PWM=0,Last_bias=0;                    //定义全局静态浮点型变量 PWM,Bias(本次偏差),Last_bias(上次偏差)
   Bias_B=Target2-Encoder;                                  //计算偏差,目标值减去当前值
   PWM_B += Velocity_KP*(Bias_B-Last_bias_B)+Velocity_KI*Bias_B;   //增量式PI控制计算
   
   if(PWM_B>PWM_Restrict)
   PWM_B=PWM_Restrict;                                     //限幅
   
   if(PWM_B<-PWM_Restrict)
   PWM_B=-PWM_Restrict;                                    //限幅  
   
   Last_bias_B=Bias_B;                                       //保存上一次偏差 
  /*
   Serial.print(PWM_B);
   Serial.print(" ");
   Serial.println(Encoder);
   */
   return PWM_B;                                           //增量输出
}
/**********PWM控制函数*********/
void Set_PWM_A(int motora)                        
{ 
  if (motora > 0)  //如果算出的PWM为正
  {
    analogWrite(Motor_AIN2,motora);  //让PWM在设定正转方向(我们认为的正转方向)正向输出调整，10是死区补偿
    digitalWrite(Motor_AIN1, 0);  //让PWM在设定正转方向(我们认为的正转方向)正向输出调整
  } else if (motora == 0)  //如果PWM为0停车
  {
    digitalWrite(Motor_AIN1, 0);
    digitalWrite(Motor_AIN2, 0);
  } else if (motora < 0)  //如果算出的PWM为负
  {
     analogWrite(Motor_AIN2, motora+255); //让PWM在设定反转方向反向输出调整
     digitalWrite(Motor_AIN1,1);
  }
}

void Set_PWM_B(int motorb)                        
{ 
  if (motorb > 0)  //如果算出的PWM为正
  {
    analogWrite(Motor_BIN1,motorb);  //让PWM在设定正转方向(我们认为的正转方向)正向输出调整，10是死区补偿
    digitalWrite(Motor_BIN2, 0);  //让PWM在设定正转方向(我们认为的正转方向)正向输出调整
  } else if (motorb == 0)  //如果PWM为0停车
  {
    digitalWrite(Motor_BIN1, 0);
    digitalWrite(Motor_BIN2, 0);
  }else if (motorb < 0)  //如果算出的PWM为负
  {
     analogWrite(Motor_BIN1, motorb+255); //让PWM在设定反转方向反向输出调整
     digitalWrite(Motor_BIN2,1);
  }
}

void run(int arr[]){
 bool left = false;
 bool right = false;
 bool middle = false;
 if(arr[2] == 0){
  middle = true;
 }
 if(arr[0] == 0 || arr[1] == 0){
  left = true;
 }
 if(arr[3] == 0 || arr[4] == 0){
  right = true;
 }
 if((left && right) || (left && middle) || (right && middle)){
  runjudge = false;
  Target_A = 0;
  Target_B = 0;
  show = true;
 }else if(arr[0] == 0){
   Target_A = Default_speed - 7.5;
   Target_B = Default_speed + 7.5;
 }else if(arr[4] == 0){
   Target_A = Default_speed + 7.5;
   Target_B = Default_speed - 7.5;
 }else if(arr[1] == 0){
   Target_A = Default_speed - 5;
   Target_B = Default_speed + 5;
 }else if(arr[3] == 0){
   Target_A = Default_speed + 5;
   Target_B = Default_speed - 5;
 }else{
  Target_A = Default_speed;
  Target_B = Default_speed;
 }
}

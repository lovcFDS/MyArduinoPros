/*
  非常简单的串口控制小车，使用了两个点击驱动小车，使用蓝牙串口对小车进行控制。
  2019/05/16
*/
//左右控制端口
int left_pin1 = 10;
int left_pin2 = 11;
int right_pin1 = 7;
int right_pin2 = 6;
char cmd = 0;
//系统状态字 0无操作 1前进 2后退 3左转 4右转
int sta = 0;
//转向使用延时函数，接收到转向命令后，延时一段时间，然后回复到原来的前进或者后退，这个变量用来保存转向前的方向。
int staBak;
void setup() {
  // put your setup code here, to run once:
  //设置引脚为输出引脚
  Serial.begin(9600);
  pinMode(left_pin1,OUTPUT);
  pinMode(left_pin2,OUTPUT);
  pinMode(right_pin1,OUTPUT);
  pinMode(right_pin2,OUTPUT);
  //开机后为停止状态
  sta = 0;
}

void loop() {
  //串口控制
  if(Serial.available()>0)
  {
    //接受串口命令
    cmd = Serial.read();
    switch(cmd)
    {
      case 'w': Serial.println(cmd); sta = 1; staBak = sta; break;
      case 's': Serial.println(cmd); sta = 2; staBak = sta; break;
      case 'a': Serial.println(cmd); sta = 3; break;
      case 'd': Serial.println(cmd); sta = 4; break;
      case 'e': Serial.println("Stop"); sta = 0; break;
    }
  }
  //检测当前状态，并执行相应的操作 
  switch(sta)
  {
    case 0:stopGo();break;
    case 1:forward();break;
    case 2:backward();break;
    case 3:turnLeft();break;
    case 4:turnRight();break;
  }
}
//全部拉低，停止前进
void stopGo()
{
  digitalWrite(left_pin1,LOW);
  digitalWrite(left_pin2,LOW);
  digitalWrite(right_pin1,LOW);
  digitalWrite(right_pin2,LOW);
}
//向右转弯
void turnRight()
{
  digitalWrite(left_pin1,LOW);
  digitalWrite(left_pin2,HIGH);
  digitalWrite(right_pin1,LOW);
  digitalWrite(right_pin2,LOW);
  _delay_ms(300);
  sta = staBak;
}
//向左转弯
void turnLeft()
{
  digitalWrite(left_pin1,LOW);
  digitalWrite(left_pin2,LOW);
  digitalWrite(right_pin1,HIGH);
  digitalWrite(right_pin2,LOW);
  _delay_ms(300);
  sta = staBak;
}
//前进
void forward()
{
  digitalWrite(left_pin1,LOW);
  digitalWrite(left_pin2,HIGH);
  digitalWrite(right_pin1,HIGH);
  digitalWrite(right_pin2,LOW);
}
//后退
void backward()
{
  digitalWrite(left_pin1,HIGH);
  digitalWrite(left_pin2,LOW);
  digitalWrite(right_pin1,LOW);
  digitalWrite(right_pin2,HIGH);
}

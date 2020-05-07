/*
* 该项目是通过ESP8266上传温室大棚内的温度，光照强度，二氧化碳浓度到自己搭建的私有Blynk服务器
* 由于ESP8266只有一个A0端口，无法读取过多的模拟引脚值，所以外接了PCF8591模块，读取其他的模拟信号值。
* pcf8591  器件地址  0x90
* nodeMcu SCL D1  SDA D2
* 
* 2019/10/15
* 
* 百度云服务已到期未续费，所以该地址已废弃。
* 2020/4/31
 */
#define BLYNK_PRINT Serial
#include<ESP8266WiFi.h>
#include<BlynkSimpleEsp8266.h>
#include<Wire.h>
#include<dht11.h>

#define MYSERVER IPAddress(106,54,130,123) //将数据发送到自己的服务器上，我的服务器没有域名，所以直接使用IP连接，值得注意的是，IP地址之间的分隔符是‘，’
#define DHTPIN 2    //对应 nodemcu D4引脚

/*  在blynk的安卓端，可以通过读取虚拟引脚的值获取对应的开发板上的值   */
#define PIN_HUM V1  //湿度 虚拟引脚
#define PIN_TEM V2  //温度 虚拟引脚
#define PIN_LIG V3 //光照强度 虚拟引脚

#define SCL_PIN D2  //I2C引脚定义
#define SDA_PIN D1

#define Address (0x90>>1) //地址右移一位，

char ssid[] = "*******";   //连接的WiFi热点的ssid
char pass[] = "***********";   //WiFi热点的密码
char auth[] = "OokJfudBF9n7gCRU-BG1Z1iCtQmuq6bh";   //blynk的应用auth码

dht11 DHT11;    //建立dht11对象
BlynkTimer timer;   //建立定时器对象

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //串口通信波特率115200，
  delay(10);  //保证串口通信正确开启。

  Wire.begin(SDA_PIN,SCL_PIN);  //初始化I2C总线
  delay(10);
  
  //首先尝试连接到WiFi，然后连接到指定的服务器。
  Blynk.begin(auth,ssid,pass,MYSERVER,8080);
  timer.setInterval(1*1000L,sendSensor);  //定时器初始化，第一个参数为延迟的时间，第二个参数为要执行的方法名称。
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();  //启动blynk服务。
  timer.run();  //启动定时器，定时器每隔一段时间执行方法
}
//I2C写入命令方法
void writeCmd(byte cmd)
{
  //1.启动总线，发送器件地址
  Wire.beginTransmission(Address);
  //2.发送命令
  Wire.write(cmd);
  //3.发送停止信号
  Wire.endTransmission();
}
//读取I2C总线返回的数据(在本项目中，每次返回的只是一个字节)
byte readData(int len) {
  byte c;
  Wire.requestFrom(Address,len);
  while (0 <Wire.available()) {
    c = Wire.read();      /* receive byte as a character */
  }
  return c;             /* to newline */
}
//定时器方法，将传感器数据发送到服务器。
void sendSensor()
{
  /*空气温湿度数据发送*/
  DHT11.read(DHTPIN);
  float tem=DHT11.temperature;                //将温度值赋值给tem
  float hum=DHT11.humidity;                   //将湿度值赋给hum
  Serial.println(tem);                        //串口打印
  Serial.println(hum);
  Blynk.virtualWrite(PIN_HUM,hum);            //上传服务器指定虚拟引脚
  Blynk.virtualWrite(PIN_TEM,tem);


  byte lightData,heatData,data4;
  /*光照强度数据发送*/
  
  //读取光敏模拟信号值
  writeCmd(0x00); 
  lightData = readData(1);
  Serial.println(256 - lightData);
  Blynk.virtualWrite(PIN_LIG,256 - lightData);

  /*
  //读取第二个模拟信号值
  writeCmd(0x42); 
  heatData = readData(1);
  Serial.println(heatData);
  //读取第三个模拟信号值
  //writeCmd(0x43);
  //readData(1); 
  //读取第四个模拟信号值
  writeCmd(0x40);
  data4 = readData(1);
  Serial.println(data4);
  Serial.println();
  */
}
//将blynk上对应的虚拟引脚的值映射到当前开发板(用来控制引脚）
//BLYNK_WRITE(V6)
//{
//  pin = param.asInt();
//}

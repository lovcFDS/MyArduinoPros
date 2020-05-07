#include <Arduino.h>
#include <U8g2lib.h>
#include <Ticker.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define SCL 5
#define SDA 4

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID     //请在这里修改接入点和密码 
#define STASSID "TP-LINK_BEA0"  //WiFi接入点
#define STAPSK  "14066666"      //接入点密码
#endif

#include <dht11.h>
#define DHTPIN 2    //对应 nodemcu D4引脚

dht11 DHT11;        
Ticker flipper;   //定时器
IPAddress ip;   //用于保存ESP的IP地址

const char* ssid = STASSID;
const char* password = STAPSK;

char hum_str[5];
char tem_str[5];

//使用静态IP 方便记忆与使用
IPAddress staticIP(10,3,13,176);
IPAddress subnet(255,255,224,0);
IPAddress gateway(10,3,0,1);

ESP8266WebServer server(80);    //创建80端口的服务

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);    //初始化显示器，创建u8g2对象

void handleRoot() {     //根处理事件
  server.send(200, "text/plain", "Hello from ESP8266\nThis is the temperature and humidity monitoring program for 140 dormitories.\nPower By Flame.");
}

void handleNotFound() {   //NOT FOUND处理事件
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
//定时器中断事件
void flip()
{
  static int sta = 0;
  float tem=DHT11.temperature;               //将温度值赋值给tem
  float hum=DHT11.humidity;                  //将湿度值赋给hum
  int val=map(analogRead(A0),0,1024,1024,0);                    //光敏强度
  gcvt(tem, 3,tem_str);     //float类型转换为字符数组（使用该转换方法,dtostrf方法会出现tem读取不到数值,具体为第一个使用该方法的字符数组为空，第二个可以正常转换内容.如果在两次调用之间隔开一行代码, 则会出现第一个调用方法的数组读取到两次的值）
  gcvt(hum, 3,hum_str);     
  
  if(sta == 0)    //显示不同的页面,第一页为温湿度和IP地址
  {
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(5,16,"Connected");
      u8g2.setCursor(5,32);
      u8g2.print(ip);
      u8g2.setCursor(5,48);
      u8g2.print("Tem:");
      u8g2.print(tem);
      u8g2.setCursor(5,64);
      u8g2.print("Hum:");
      u8g2.print(hum);
    } while ( u8g2.nextPage() );
    sta = 1;
  }
  else if( sta == 1)    //第二页显示欢迎语,和光照强度
  {
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(5,16,"Welcome !");
      u8g2.setCursor(5,32);
      u8g2.print("Room 140");
      u8g2.setCursor(5,48);
      u8g2.print("Light:");
      u8g2.print(val);
    } while ( u8g2.nextPage() );
    sta = 0;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);     //开启串口
    
  WiFi.mode(WIFI_STA);      //设置WiFi为sta模式,并尝试连接到接入点
  WiFi.begin(ssid, password); 
  //使用静态IP 方便记忆与使用
IPAddress staticIP(10,3,13,176);
IPAddress subnet(255,255,224,0);
IPAddress gateway(10,3,0,1);
  Serial.println("");

  //等待WiFi连接
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  ip = WiFi.localIP();
  Serial.println(ip);
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }  

  //事件注册
  server.on("/", handleRoot);
  //获取湿度的处理事件,使用匿名方法
  server.on("/getHum", [](){
    server.send(200, "text/plain", hum_str);
  });
  //获取温度的处理事件,使用匿名方法
  server.on("/getTem", [](){
    server.send(200, "text/plain", tem_str);
  });
  //NOT Found事件注册
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  //设置温湿度传感器的GPIO引脚为输出模式
  pinMode(DHTPIN,OUTPUT);
  u8g2.begin();               //开启显示功能
  flipper.attach(5, flip);    //设置定时器中断事件,当前为5秒发生一次定时器中断
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();    //开启WebServer服务
  MDNS.update();
  DHT11.read(DHTPIN);   //获取传感器的温湿度数值（不要放入定时器中断事件内,会出现无法读取数据的情况）
}

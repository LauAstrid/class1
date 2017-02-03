#include <Rtc_Pcf8563.h>
Rtc_Pcf8563 rtc;
#define music_num_MAX 9
#include <SoftwareSerial.h>
//用户自定义部分-----------------
#include <Wire.h>
//EEPROM---------------------
#include <EEPROM.h>
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

struct config_type
{
  int EEPROM_music_num;       //歌曲的数目
  int EEPROM_music_vol;       //歌曲的音量
};

//用户自定义部分------------------------
#include "audio.h"   //"arduino.h"是控制音频文件

#include "U8glib.h"
//-------字体设置，大、中、小
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_fixed_v0r)
/*
  font:
  u8g_font_7x13
  u8g_font_fixed_v0r
  u8g_font_chikitar
  u8g_font_osb21
  u8g_font_courB14r
  u8g_font_courB24n
  u8g_font_9x18Br
*/

//屏幕类型--------
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

#define init_draw 500  //主界面刷新时间
unsigned long timer_draw;

int MENU_FONT = 1;  //初始化字体大小 0：小，1：中，2：大

boolean music_status = false; //歌曲播放状态
int music_num = 1;    //歌曲序号
int music_vol = 20;             //音量0~30

String dateStr, ret;


unsigned long music_vol_time_cache = 0;
unsigned long music_vol_time = 0;
boolean key[NUM_DIGITAL_PINS];
boolean key_cache[NUM_DIGITAL_PINS];    //检测按键松开缓存
boolean music_vol_sta;
#define mic_pin A6
#define key_pin 4
int voice_data;
long time, timer;
boolean add;
int i = 200;
#include <Microduino_Key.h>

Key KeyA(A0, INPUT);     //定义摇杆输入
Key KeyB(A0, INPUT);
Key KeyC(A0, INPUT);
Key KeyD(A0, INPUT);
Key KeyE(A0, INPUT);
 
void setup() {
  Serial.begin(9600);    
  Serial.println("Microduino!");
  eeprom_READ();
  audio_init(DEVICE_TF, MODE_loopOne, music_vol);   //初始化mp3模块
  pinMode(mic_pin, INPUT);//pinMode()函数用来设置引脚是输入模式还是输出模式
  pinMode(key_pin, INPUT);//定义引脚6为输入模式，用来接收信号
  key_init();
  delay(1000);
}

void play()
{
      music_status = !music_status; //播放或暂停
      if (music_status == true) //播放
      {
        Serial.println("play");   //串口输出 “play”（工作
        audio_play();              //音频工作
      }
      else  //暂停
      {
        Serial.println("pause");   //串口输出 “pause”（暂停）
        audio_pause();              //音频暂停工作
      }
      Serial.println(analogRead(A0));
       
}

const int sensorPin=A3;    //传感器连接到A3
 const int THRESHOLD=100;
 void setup2()
{
 pinMode(sensorPin,INPUT);
}   //设置振动传感器为输入设备


int uiStep() 
{
    int val=analogRead(sensorPin);
    if (val>THRESHOLD)    //
   {
    delay(50);       //延迟50毫秒输出
    return 2;      //回到动作2
    }
}

void eeprom_WRITE()
{
  config_type config;     // 定义结构变量config，并定义config的内容
  config.EEPROM_music_num = music_num;
  config.EEPROM_music_vol = music_vol;

  EEPROM_write(0, config);  // 变量config存储到EEPROM，地址0写入
}

void eeprom_READ()
{
  config_type config_readback;
  EEPROM_read(0, config_readback);
  music_num = config_readback.EEPROM_music_num;
  music_vol = config_readback.EEPROM_music_vol;
}

void draw()
{
  setFont_L;

  u8g.setPrintPos(4, 16);
  u8g.print("Music_sta:");
  u8g.print(music_status ? "play" : "pause");

  u8g.setPrintPos(4, 16 * 2);
  u8g.print("Music_vol:"); 
  u8g.print(music_vol);
  u8g.print("/30");
  u8g.setPrintPos(4, 16 * 3);
  u8g.print("Music_num:");
  u8g.print(music_num);
  u8g.print("/");
  u8g.print(music_num_MAX);
  u8g.setPrintPos(4, 16 * 4);
  u8g.print("....Microduino....");
  //u8g.print(rtc.formatTime(RTCC_TIME_HMS));
}
    
void loop() {
int vol = uiStep(); //检测输入动作
if (vol == 2)              //向右拨动开关为动作2
  {
      music_vol_time = millis() - music_vol_time_cache;
    //    if(music_vol_time>200)  //拨动开关时间大于0.2秒
    delay(500);                   //延迟0.5秒
      Serial.println("next");    //歌曲输出下一个（向右波动1次开关）

      music_num++;  //歌曲序号加
      if (music_num > music_num_MAX)  //限制歌曲序号范围，如果歌曲序号大于9
      {
        music_num = 1; //歌曲序号返回1
      }
      audio_choose(music_num);
      audio_play();
      //delay(500);      //延迟0。5秒
      music_status = true;       //音频状态为工作
      eeprom_WRITE();

    }
  
  //摇杆向里按
  switch (KeyE.read(0, 20)) {
    case SHORT_PRESS:
      Serial.println("KEY ENTER(analog) SHORT_PRESS");  //短按
      play();
      break;
    case LONG_PRESS:
      Serial.println("KEY ENTER(analog) LONG_PRESS");//长按
      play();
      break;
  }
  if (millis() - timer_draw > init_draw)
  {
    u8g.firstPage();
    do {
      draw();
    }
    while ( u8g.nextPage() );
    timer_draw = millis();
  }
  
  delay(5);
}

void key_init()
{
  for (int a = 0; a < NUM_DIGITAL_PINS; a++)
  {
    key[a] = LOW;
    key_cache[a] = HIGH;
  }
}

boolean key_get(int _key_pin, boolean _key_type)
{
  key_cache[_key_pin] = key[_key_pin];    //缓存作判断用

  key[_key_pin] = !digitalRead(_key_pin); //触发时

  switch (_key_type)
   {
    case 0:
      if (!key[_key_pin] && key_cache[_key_pin])
       return true;
      else
        return false;
       break;
   case 1:
      if (key[_key_pin] && !key_cache[_key_pin])
        return true;
      else
        return false;
        break;
   }
  

    voice_data = analogRead(mic_pin);//analogRead()函数在引脚设置为输入的情况下，可以获取引脚的电压情况HIGH（高电平）或者LOW第电平
  Serial.println(voice_data);//将数据从Arduino传递到PC，
    if (millis() - time > 500 )//millis()是Arduino的自有函数，它返回以毫秒为单位的从Arduino开始执行到运行到当前的时间
    {
      voice_data = analogRead(mic_pin);
      switch(voice_data)
      {
       case 0:
         if(0<=voice_data<5)
      {music_vol_sta = true;        //音量工作
       music_vol=10;
       audio_vol(music_vol);
       Serial.println("++");       //Serial.println函数输出“+1”
       delay(100);                  //延迟0.1秒
       eeprom_WRITE();
       }break;
       case 1:
         if(5<=voice_data<10)
         {music_vol_sta = true;        //音量工作
          music_vol=15;
          audio_vol(music_vol);
          Serial.println("++");       //Serial.println函数输出“+1”
          delay(100);                  //延迟0.1秒
          eeprom_WRITE();
         }break;
       case 2:
         if(10<=voice_data<15)
         {music_vol_sta = true;        //音量工作
          music_vol=20;
          audio_vol(music_vol);
          Serial.println("++");       //Serial.println函数输出“+1”
          delay(100);                  //延迟0.1秒
          eeprom_WRITE();     
         }break;
       case 3:
         if(15<=voice_data<20)
         {music_vol_sta = true;        //音量工作7
          music_vol=25;
          audio_vol(music_vol);
          Serial.println("++");       //Serial.println函数输出“+1”
          delay(100);                  //延迟0.1秒
          eeprom_WRITE();         
         }break;
       case 4:
         if(20<=voice_data)
         {music_vol_sta = true;        //音量工作
          music_vol=30;
          audio_vol(music_vol);
          Serial.println("++");       //Serial.println函数输出“+1”
          delay(100);                  //延迟0.1秒
          eeprom_WRITE();          
         }break;
     }
    }
}

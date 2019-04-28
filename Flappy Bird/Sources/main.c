#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "LCD.h"      
#include "18B20.h"     

#define BUS_CLOCK		   32000000	   //总线频率
#define OSC_CLOCK		   16000000	   //晶振频率


#define KEY1 PTIH_PTIH3
#define KEY2 PTIH_PTIH2
#define KEY3 PTIH_PTIH1
#define KEY4 PTIH_PTIH0
#define KEY1_dir DDRH_DDRH3
#define KEY2_dir DDRH_DDRH2
#define KEY3_dir DDRH_DDRH1
#define KEY4_dir DDRH_DDRH0

#define LED PORTB
#define LED_dir DDRB
#define KEY1 PTIH_PTIH3
#define KEY2 PTIH_PTIH2
#define KEY3 PTIH_PTIH1
#define KEY4 PTIH_PTIH0
#define KEY1_dir DDRH_DDRH3
#define KEY2_dir DDRH_DDRH2
#define KEY3_dir DDRH_DDRH1
#define KEY4_dir DDRH_DDRH0
#define LEDCPU PORTK_PK4
#define LEDCPU_dir DDRK_DDRK4
#define BUZZ PORTK_PK5
#define BUZZ_dir DDRK_DDRK5

#define CONT1 PORTK_PK3
#define CONT2 PORTK_PK2
#define CONT3 PORTK_PK1
#define CONT4 PORTK_PK0
#define CONT1_dir DDRK_DDRK3
#define CONT2_dir DDRK_DDRK2
#define CONT3_dir DDRK_DDRK1
#define CONT4_dir DDRK_DDRK0
#define SHUMA PTP
#define SHUMA_dir DDRP


unsigned char data=0x01;
unsigned char mode=1;
unsigned char F1_last=1;
unsigned char F2_last=1;

unsigned char direction=1;   //设置灯亮的方向，0向左，1向右。
unsigned char time=20;        //设置灯闪的速度。
unsigned char cycle=1;
unsigned int wendu;
unsigned int zhengshu,xiaoshu;
unsigned char temperature[6]={'0','0','0','.','0','0'};
unsigned char dianya[4];
unsigned int AD_in;
unsigned int dianyazhi;
unsigned int deathsign = 0;
unsigned int state1 = 0;

byte data1 = 0;
byte data2 = 0;
byte data3 = 0;
byte data4 = 0;
byte single = 1;

byte shuma[20]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,                   0x6f,       //0~9对应的段码
                0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xef};      //0~9后加小数点对应的段码




/*************************************************************/
/*                      初始化锁相环                         */
/*************************************************************/
void INIT_PLL(void) 
{
    CLKSEL &= 0x7f;       //set OSCCLK as sysclk
    PLLCTL &= 0x8F;       //Disable PLL circuit
    CRGINT &= 0xDF;
    
    #if(BUS_CLOCK == 40000000) 
      SYNR = 0x44;
    #elif(BUS_CLOCK == 32000000)
     SYNR = 0x43;     
    #elif(BUS_CLOCK == 24000000)
     SYNR = 0x42;
    #endif 

    REFDV = 0x81;         //PLLCLK=2×OSCCLK×(SYNDIV+1)/(REFDIV+1)＝64MHz ,fbus=32M
    PLLCTL =PLLCTL|0x70;  //Enable PLL circuit
    asm NOP;
    asm NOP;
    while(!(CRGFLG&0x08)); //PLLCLK is Locked already
    CLKSEL |= 0x80;        //set PLLCLK as sysclk
}



/************************************************************/
/*                    初始化ECT模块                         */
/************************************************************/
void initialize_ect(void){
  TSCR1_TFFCA = 1;  // 定时器标志位快速清除
  TSCR1_TEN = 1;    // 定时器使能位. 1=允许定时器正常工作; 0=使主定时器不起作用(包括计数器)
  TIOS  = 0xff;      //指定所有通道为输出比较方式
  TCTL1 = 0x00;	    // 后四个通道设置为定时器与输出引脚断开
  TCTL2 = 0x00;     // 前四个通道设置为定时器与输出引脚断开
  TIE   = 0x00;     // 允许通道4定时中断
  TSCR2 = 0x07;	    // 预分频系数pr2-pr0:111,,时钟周期为4us,
  TFLG1 = 0xff;	    // 清除各IC/OC中断标志位
  TFLG2 = 0xff;     // 清除自由定时器中断标志位
}

/************************************************************/
/*                         延时函数                         */
/************************************************************/
void Delay(int x){
  unsigned long volatile time;
  time = 727240*200*x/91;
  while(time){
    time--;
  }
}

/*************************************************************/
/*                       初始化按键                          */
/*************************************************************/
void init_key(void) 
{
     KEY1_dir =0;       //设置为输入
     KEY2_dir=0;
     KEY3_dir=0;
     KEY4_dir=0;
     PPSH = 0x00;		      //极性选择寄存器,选择下降沿;
     PIFH = 0x0f;					//对PIFH的每一位写1来清除标志位;
     PIEH = 0x0f;		      //中断使能寄存器;
}

/*************************************************************/
/*                      初始化数码                           */
/*************************************************************/
void INIT_shuma(void) 
{
  CONT1_dir = 1;
  CONT2_dir = 1;
  CONT3_dir = 1;
  CONT4_dir = 1;
  CONT1 = 0;
  CONT2 = 0;
  CONT3 = 0;
  CONT4 = 0;
  SHUMA_dir = 0xff;
  SHUMA = 0x00;
}




/*************************************************************/
/*                    按键中断函数                           */
/*************************************************************/
#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt void PTH_inter(void) 
{
   if(PIFH != 0)     //判断中断标志
   {
      PIFH = 0xff;     //清除中断标志
      if(KEY1 == 0)         //按键1按下
      {
          birdtiaodong();
          BUZZ=1;
          delay1ms(20);
          BUZZ = 0;
      }
      if(KEY2 == 0) 
      {
          birdtiaodong();
          BUZZ=1;
          delay1ms(20);
          BUZZ = 0;      
      }
      if(KEY3 == 0)
      {
          birdtiaodong();
          BUZZ=1;
          delay1ms(20);
          BUZZ = 0;
      }
      if(KEY4 == 0)
      {
          birdtiaodong();
          BUZZ=1;
          delay1ms(20);
          BUZZ = 0;
      }
   }
}

/*************************************************************/
/*                      中断扫描函数                         */
/*************************************************************/
interrupt void scan(void)
{
  if(TFLG1_C4F == 1)
  {
    TFLG1_C4F = 1;
    TC4 = TCNT + 1250;         //设置输出比较时间为5ms
  }
  switch(single)
  {
    case 1:
      CONT1 = 1;
      CONT2 = 0;
      CONT3 = 0;
      CONT4 = 0;
      SHUMA=shuma[data1];
      break;

    case 2:
      CONT1 = 0;
      CONT2 = 1;
      CONT3 = 0;
      CONT4 = 0;
      SHUMA=shuma[data2];
      break;

    case 3:
      CONT1 = 0;
      CONT2 = 0;
      CONT3 = 1;
      CONT4 = 0;
      SHUMA=shuma[data3];
      break;

    case 4:
      CONT1 = 0;
      CONT2 = 0;
      CONT3 = 0;
      CONT4 = 1;
      SHUMA=shuma[data4];
      break;
    
    default:
      break;
  }
  
  single +=1;
  if(single == 5) single = 1;
}
#pragma CODE_SEG DEFAULT

void fs2shuma(void)
{
    int fs;
    fs = fschuanshu(); //将当前的分数取出
    data4 = fs % 10;
    data3 = (fs % 100 - fs % 10)/10;
    data2 = (fs % 1000 - fs % 100)/100;
    data1 = (fs % 10000 - fs % 1000)/1000;
}


/************************************************************/
/*                         主函数                           */
/************************************************************/
void main(void) {
	DisableInterrupts;
  INIT_PLL();
  initialize_ect();
  INIT_PORT();
  init_key();
  INIT_shuma();
  LEDCPU_dir=1;
  LEDCPU=0;
  BUZZ_dir=1;
  BUZZ=0;
	EnableInterrupts;
  TFLG1_C4F = 1;
  TC4 = TCNT + 1250;         //设置输出比较时间为5ms
   TIE=0x10;
  for(;;) 
  {
    flappybirdmapcreate();
    drawbird();
    birdmove();
    flapptbirdlcdpicture();
    delay1ms(20);
    chengji();
    fs2shuma();
    state1 = returnstate();
    if (state1 == 1)
    {
      chongxing();
      BUZZ = 1;
      delay1ms(500);
      BUZZ = 0;
    }
  } 
}

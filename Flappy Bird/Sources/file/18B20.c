#include "derivative.h"      /* derivative-specific definitions */
#include "18B20.h"      


/*************************************************************/
/*                         延时函数1                         */
/*************************************************************/
void delay8us(unsigned int n) 
{
    unsigned int i;
    for(i=0;i<n;i++) 
    {
        TFLG1_C0F = 1;              //清除标志位
        TC0 = TCNT + 2;             //设置输出比较时间为8us
        while(TFLG1_C0F == 0);      //等待，直到发生输出比较事件
    }
}

/*************************************************************/
/*                      初始化18B20                          */
/*************************************************************/
void init18b20(void)
{
 DSDDR=1;
 DSO = 1; 
 delay8us(1);
 DSO = 0;          //拉低数据线，复位总线；
 delay8us(63);     //延时504us 
 DSO = 1;         //提升数据线；
 delay8us(4);     //延时32us；
 DSDDR=0;
 while(DSI)       //等待从器件器件应答信号；
 {asm("nop");}
 DSDDR=1;
 delay8us(16);     //延时128us； 
 DSO = 1;          //提升数据线，准备数据传输；
}

/*************************************************************/
/*                      向18B20写入数据                      */
/*************************************************************/
void WR18b20(byte cmd)
{
    unsigned char k;
    for(k=0;k<8;k++)
    {
        if(cmd & 0x01)  	    //低位在前；
        {
            DSO = 0;    
            delay8us(1); 
            DSO = 1;          //发送数据;
        }                 	
        else 
        {
            DSO = 0;    	       
            delay8us(1);  
        }
        delay8us(8);  		 //延时64us等待从器件采样；
        DSO = 1;      	   //拉高总线
        delay8us(1);      	 
        cmd >>= 1;
    }
}

/*************************************************************/
/*                      由18B20读取数据                      */
/*************************************************************/
unsigned char RD18b20(void)
{
    unsigned char k;
    unsigned char tmp=0;
    DSO = 1;  
    delay8us(1);          //准备读；
    for(k=0;k<8;k++)
    {
      tmp >>= 1;    		 //先读取低位
      DSO = 0;     	     //Read init；
      delay8us(1);     		
      DSO = 1;      		//必须写1，否则读出来的将是不预期的数据；
      asm("nop");asm("nop");asm("nop");			//延时9us�  
      DSDDR=0;
      asm("nop");
      if(DSI)      	//在12us处读取数据；
      tmp |= 0x80;
      delay8us(8);
      DSDDR=1;
      DSO = 1;  
      delay8us(1);
        //恢复One Wire Bus；
    }
    return tmp; 
}

/*************************************************************/
/*                      由18B20读取温度                      */
/*************************************************************/
unsigned int read_T(void)
{  
	 unsigned int t;
	 unsigned char temp[2];
   
   init18b20();
   WR18b20(0xcc);  
   WR18b20(0x44); 
   init18b20();
   WR18b20(0xcc);
   WR18b20(0xbe);
   temp[0]=RD18b20();
   temp[1]=RD18b20();
   init18b20();
   t=(temp[1]<<8)|temp[0];
   return(t); 
 } 



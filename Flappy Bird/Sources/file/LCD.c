#include "derivative.h" /* derivative-specific definitions */
#include "LCD.h"

#define BUZZ PORTK_PK5
#define BUZZ_dir DDRK_DDRK5


int roll[16] = {80, 75, 55, 60, 70, 65, 75, 55,
                40, 30, 55, 60, 80, 55, 68, 70};
int rollsign = 0;
int row_now = 1;
int flappybirdmap[64][16];
int flappybirdmap2[64][16];
int secpipey, thipipey;
int firpipey = 39;
int firpipex;
int secpipex;
int thipipex;
int pipewidth = 16;
int kongbai = 40;
int state = 0;
int DEATH = 0;
int birdl = 68;
int birdstate = 1; //为1时为下降状态，为0时为死亡下降状态。
int pipe2pipe = 40;

int badapplesign = 0;
int badapplemap[64][16];
int badapplemap2[64][16];
int badappleallmap[6400][16];

int julivuzi; 
int juli = 0;   //鸟儿前进的距离，用以计算分数;
int fs = 0;             //得分
/*************************************************************/
/*                      初始化液晶接口                       */
/*************************************************************/
void INIT_PORT(void)
{
    PSB_dir = 1;
    RS_dir = 1;
    RW_dir = 1;
    EN_dir = 1;
    DATA_dir = 0xff;
    DATA = 0;
    PSB = 1; //并行模式
    EN = 0;
    RW = 0;
    RS = 0;
}

/*************************************************************/
/*                     向液晶发送数据                        */
/*************************************************************/
void write_Data(unsigned char b)
{
    RS = 1; //写数据操作
    RW = 0; //指明为写操作
    EN = 1;
    somenop();
    DATA = b;
    somenop();
    EN = 0;
}

/*************************************************************/
/*                      向液晶发送命令                       */
/*************************************************************/
void write_command(unsigned char b)
{
    RS = 0; //写命令操作
    RW = 0;
    delay20us(1);
    EN = 1;
    somenop();
    DATA = b;
    somenop();
    delay20us(1);
    EN = 0;
}


/*************************************************************/
/*                         延时函数1                         */
/*************************************************************/
void delay20us(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        TFLG1_C0F = 1;  //清除标志位
        TC0 = TCNT + 5; //设置输出比较时间为20us
        while (TFLG1_C0F == 0)
            ; //等待，直到发生输出比较事件
    }
}

/*************************************************************/
/*                         延时函数2                         */
/*************************************************************/
void delay1ms(unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++)
    {
        TFLG1_C0F = 1;    //清除标志位
        TC0 = TCNT + 250; //设置输出比较时间为1ms
        while (TFLG1_C0F == 0)
            ; //等待，直到发生输出比较事件
    }
}


/***************************************************************************/
/*                            清屏函数                                     */
/***************************************************************************/
void fill_GDRAM(uchar dat) //dat 为填充的数据
{
    uchar j, k;
    uchar GDRAM_X = 0x80; //GDRAM 水平地址
    uchar GDRAM_Y = 0x80; //GDRAM 垂直地址
    write_command(0x34);  //使用扩展指令集，绘图显示OFF

    for (j = 0; j < 32; j++) //上半屏
    {
        write_command(GDRAM_X + j);
        delay20us(4); //延时80us
        write_command(0x80);
        delay20us(4); //延时80us
        for (k = 0; k < 16; k++)
        {
            write_Data(dat);
            delay20us(4);
        }
    }
    for (j = 0; j < 32; j++) //下半屏
    {
        write_command(GDRAM_X + j);
        delay20us(4); //延时80us
        write_command(0x88);
        delay20us(4); //延时80us
        for (k = 0; k < 16; k++)
        {
            write_Data(dat);
            delay20us(4);
        }
    }

    write_command(0x36); // 打开绘图模式
                         //write_command(0x30);// 恢复基本指令集，关闭绘图模式
}


/***************************************************************************/
/*                              柱子生成函数                                */
/***************************************************************************/
void flappybirdmapcreate(void)
{
    int i, j;

    if (state == 0) //第一次执行时
    {
        julivuzi = firpipey + pipewidth;    //鸟距离第一个柱子的距离，用以计算分数
        firpipex = roll[(rollsign)&0x07];
        secpipex = roll[(rollsign + 1) & 0x07];
        thipipex = roll[(rollsign + 2) & 0x07];
        fill_GDRAM(0x00);   //第一次刷新时，先将屏幕进行空白刷新
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 16; j++)
            {
                flappybirdmap[i][j] = 0x00;
            }
        }
        drawnewpipe(firpipex, firpipey, pipewidth, kongbai);
        secpipey = firpipey + pipe2pipe;
        drawnewpipe(secpipex, secpipey, pipewidth, kongbai);
        thipipey = secpipey + pipe2pipe;
        if (thipipey < 64) //当第三个柱子该出现时
        {
            drawnewpipe(thipipex, thipipey, pipewidth, kongbai);
        }
        firpipey--;
        state++;
    }
    else
    {
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 16; j++)
            {
                flappybirdmap2[i][j] = flappybirdmap[i][j]; //datamap为暂存函数
                flappybirdmap[i][j] = 0x00;
            }
        }
        drawnewpipe(firpipex, firpipey, pipewidth, kongbai);
        secpipey = firpipey + pipe2pipe;
        if (secpipey < 64)
        {
            drawnewpipe(secpipex, secpipey, pipewidth, kongbai);
            thipipey = secpipey + pipe2pipe;
            if (thipipey < 64) //当第三个柱子该出现时
            {
                drawnewpipe(thipipex, thipipey, pipewidth, kongbai);
            }
        }

        firpipey--;
        if (firpipey < -pipewidth - 1)
        {
            firpipey = secpipey;
            firpipex = secpipex;
            secpipey = thipipey;
            secpipex = thipipex;
            thipipex = roll[(rollsign + 2) & 0x07];
            rollsign++;
        }
    }
}

void flapptbirdlcdpicture(void)
{
    unsigned int isign, rowsign;
    for (rowsign = 0; rowsign < 32; rowsign++) //按照行数写入
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (flappybirdmap2[rowsign][2 * isign] != flappybirdmap[rowsign][2 * isign] || flappybirdmap2[rowsign][2 * isign + 1] != flappybirdmap[rowsign][2 * isign + 1])
            {
                write_command(0x36); //使用扩展指令集，绘图显示OFF
                delay20us(4);
                write_command(0x80 + rowsign); //行位置
                delay20us(4);                  //延时80us
                write_command(0x80 + isign);   //上半屏列位置
                delay20us(4);                  //延时80us
                write_Data(flappybirdmap[rowsign][2 * isign]);
                delay20us(4); //延时80us
                write_Data(flappybirdmap[rowsign][2 * isign + 1]);
                delay20us(4);        //延时80us
                write_command(0x30); //使用扩展指令集，绘图显示ON
                delay20us(4);
                //delay1ms(3);
            }
        }
    }
    for (rowsign = 0; rowsign < 32; rowsign++)
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (flappybirdmap2[rowsign + 32][2 * isign] != flappybirdmap[rowsign + 32][2 * isign] || flappybirdmap2[rowsign + 32][2 * isign + 1] != flappybirdmap[rowsign + 32][2 * isign + 1])
            {
                write_command(0x36);
                delay20us(4);
                write_command(0x80 + rowsign);
                delay20us(4); //延时80us
                write_command(0x88 + isign);
                delay20us(4); //延时80us
                write_Data(flappybirdmap[rowsign + 32][2 * isign]);
                delay20us(4); //延时80us
                write_Data(flappybirdmap[rowsign + 32][2 * isign + 1]);
                delay20us(4);        //延时80us
                write_command(0x30); //使用扩展指令集，绘图显示ON
                delay20us(4);
                //delay1ms(3);
            }
        }
    }
}

/***************************************************************************/
/*                            更改显示的图像                                */
/***************************************************************************/
void flappybirdmapchange(void)
{
    int i, ii;

    for (i = 0; i < 64; i++)
    {
        for (ii = 0; ii < 16; ii++)
        {
            flappybirdmap2[i][ii] = flappybirdmap[i][ii]; //datamap为暂存函数
        }
    }
    for (i = 0; i < 64; i++)
    {
        for (ii = 0; ii < 16; ii++)
        {
            if (row_now + i < 64) //小于64则直接更改
            {
                flappybirdmap[row_now + i][ii] = flappybirdmap2[i][ii];
            }
            else
            {
                flappybirdmap[row_now + i - 64][ii] = flappybirdmap2[i][ii];
            }
        }
    }
}

/***************************************************************************/
/*                             画出新柱子                                   */
/*          a:空白部分坐标  b；柱子起始坐标 c 柱子宽度  d 空白宽度                         */
/***************************************************************************/
void drawnewpipe(int a, int b, int c, int d)
{
    int i, j, k;
    int ysign;
    for (i = b; i < b + c; i++)
    {
        if (i >= 0 && i <= 63)
        {
            ysign = 0;
            for (j = 0; j < 16; j++)
            {
                for (k = 7; k >= 0; k--)
                {
                    if (ysign < a - d / 2)
                    {
                        flappybirdmap[i][j] = flappybirdmap[i][j] + pow1(2, k);
                        ysign++;
                    }
                }
            }
            ysign = 0;
            for (j = 15; j >= 0; j--)
            {
                for (k = 0; k < 8; k++)
                {
                    if (ysign < 128 - a - d / 2)
                    {
                        flappybirdmap[i][j] = flappybirdmap[i][j] + pow1(2, k);
                        ysign++;
                    }
                }
            }
        }
    }
}

/***************************************************************************/
/*                             幂函数                                    */
/***************************************************************************/
int pow1(int a, int b)
{
    int i;
    int j = 1;
    for (i = 0; i < b; i++)
    {
        j = j * a;
    }
    return j;
}

/***************************************************************************/
/*                             鸟点函数                                    */
/***************************************************************************/
void birdonepoint(int row, int i)
{
    uchar x_Dyte, x_byte;  // 定义列地址的字节位及在字节中的哪一位
    x_Dyte = i / 8;    //算出它在哪一个字节（地址）,一个地址是16位的
    x_byte = i % 8 ; // 计算在该字节中的 哪一位
    row = row & 0x1f;
    flappybirdmap[row][x_Dyte] |= 0X01 << (7 - x_byte);
    panduan(row, i, firpipex, firpipey, pipewidth, kongbai);
    panduan(row, i, secpipex, secpipey, pipewidth, kongbai);
}

/***************************************************************************/
/*                             全鸟函数                                    */
/***************************************************************************/
void drawbird(void)
{
    birdonepoint(0, birdl - 7);
    birdonepoint(0, birdl - 8);
    birdonepoint(0, birdl - 9);

    birdonepoint(1, birdl - 4);
    birdonepoint(1, birdl - 5);
    birdonepoint(1, birdl - 6);
    birdonepoint(1, birdl - 10);

    birdonepoint(2, birdl - 3);
    birdonepoint(2, birdl - 6);
    birdonepoint(2, birdl - 10);

    birdonepoint(3, birdl - 2);
    birdonepoint(3, birdl - 6);
    birdonepoint(3, birdl - 10);

    birdonepoint(4, birdl - 2);
    birdonepoint(4, birdl - 6);
    birdonepoint(4, birdl - 9);
    birdonepoint(4, birdl - 10);

    birdonepoint(5, birdl - 1);
    birdonepoint(5, birdl - 6);
    birdonepoint(5, birdl - 8);
    birdonepoint(5, birdl - 11);

    birdonepoint(6, birdl - 0);
    birdonepoint(6, birdl - 7);
    birdonepoint(6, birdl - 11);

    birdonepoint(7, birdl - 0);
    birdonepoint(7, birdl - 11);

    birdonepoint(8, birdl - 0);
    birdonepoint(8, birdl - 2);
    birdonepoint(8, birdl - 3);
    birdonepoint(8, birdl - 4);
    birdonepoint(8, birdl - 8);
    birdonepoint(8, birdl - 11);

    birdonepoint(9, birdl - 0);
    birdonepoint(9, birdl - 1);
    birdonepoint(9, birdl - 5);
    birdonepoint(9, birdl - 7);
    birdonepoint(9, birdl - 9);

    birdonepoint(10, birdl - 0);
    birdonepoint(10, birdl - 6);
    birdonepoint(10, birdl - 8);
    birdonepoint(10, birdl - 9);
    birdonepoint(10, birdl - 10);
    birdonepoint(11, birdl - 0);

    birdonepoint(11, birdl - 6);
    birdonepoint(11, birdl - 8);
    birdonepoint(11, birdl - 10);

    birdonepoint(12, birdl - 1);
    birdonepoint(12, birdl - 3);
    birdonepoint(12, birdl - 4);
    birdonepoint(12, birdl - 6);
    birdonepoint(12, birdl - 8);
    birdonepoint(12, birdl - 10);

    birdonepoint(13, birdl - 2);
    birdonepoint(13, birdl - 6);
    birdonepoint(13, birdl - 8);
    birdonepoint(13, birdl - 10);

    birdonepoint(14, birdl - 3);
    birdonepoint(14, birdl - 4);
    birdonepoint(14, birdl - 5);
    birdonepoint(14, birdl - 6);
    birdonepoint(14, birdl - 8);
    birdonepoint(14, birdl - 10);

    birdonepoint(15, birdl - 6);
    birdonepoint(15, birdl - 8);
    birdonepoint(15, birdl - 9);

    birdonepoint(16, birdl - 7);
}



/***************************************************************************/
/*                             碰撞判断函数                                 */
/***************************************************************************/

void panduan(int row, int i, int a, int b, int c, int d)
{
    if (row >= b && row <= b + c)
    {
        if (i <= (a - d / 2) || i >= (a + d / 2))
        {
            DEATH = 1;
            // state = 0; //二状态表示发生碰撞
            // birdl = 68;
            // rollsign = 0;
            // firpipey = 39;
            // juli = 0;
            // fs = 0;
            // delay1ms(200); //死亡时延时
            // return;
        }
    }
    if (i<=0 || i>=128)
    {
            DEATH = 1;
            // state = 0; //二状态表示发生碰撞
            // birdl = 68;
            // rollsign = 0;
            // firpipey = 39;
            // juli = 0;
            // fs = 0;
            // delay1ms(200); //死亡时延时
            // return;
    }
    
}

/***************************************************************************/
/*                             碰撞判断函数                                 */
/***************************************************************************/
int returnstate(void)
{
    return DEATH;
}

int live(void)
{
    DEATH = 0;
}
void chongxing(void)
{
    state = 0; //二状态表示发生碰撞
    birdl = 68;
    rollsign = 0;
    firpipey = 39;
    juli = 0;
    fs = 0;
    DEATH = 0;
}



/*************************************************************/
/*                      案件按下使得鸟跳动                     */
/*************************************************************/
int birdtiaodong(void)
{
  birdstate = 4;
}


/***************************************************************************/
/*                             鸟移动函数                                    */
/***************************************************************************/
void birdmove(void)
{
    //按键中断下birdstate设置为4
    if (birdstate > -2 && birdstate < 1)
    {
        birdl = birdl -2;
        birdstate--;
    }
    else if (birdstate > 2)
    {
        birdl = birdl + 4;
        birdstate--;
    }
    else if (birdstate > 0 && birdstate < 3)
    {
        birdl = birdl + 2 ;
        birdstate--;
    }
    else
    {
        birdl = birdl - 4;
        //birdstate = 4;
    }
}


/***************************************************************************/
/*                            badapple图纸构造函数                         */
/***************************************************************************/
void badapplecreate(void)
{
    int i,j;
    if ( badapplesign == 0 )
    {
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 16; j++)
            {
                badapplemap[i][j] = badappleallmap[i + badapplesign][j];
                badapplemap2[i][j] = 0x00;
            }
        }
        badapplesign = badapplesign + 64;
    }
    else
    {
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 16; j++)
            {
                badapplemap[i][j] = badappleallmap[i + badapplesign][j];
                badapplemap2[i][j] = badapplemap[i][j];
            }
        }
        badapplesign = badapplesign + 64;
    }
}

/***************************************************************************/
/*                             badapple绘制函数                            */
/***************************************************************************/
void badapple(void)
{
    unsigned int isign, rowsign;
    for (rowsign = 0; rowsign < 32; rowsign++) //按照行数写入
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (badapplemap2[rowsign][2 * isign] !=badapplemap[rowsign][2 * isign] || badapplemap2[rowsign][2 * isign + 1] !=badapplemap[rowsign][2 * isign + 1])
            {
                write_command(0x36);//使用扩展指令集，绘图显示OFF
                delay20us(4);
                write_command(0x80 + rowsign); //行位置
                delay20us(4);                  //延时80us
                write_command(0x80 + isign);   //上半屏列位置
                delay20us(4);                  //延时80us
                write_Data(badapplemap[rowsign][2 * isign]);
                delay20us(4); //延时80us
                write_Data(badapplemap[rowsign][2 * isign + 1]);
                delay20us(4);        //延时80us
                write_command(0x30); //使用扩展指令集，绘图显示ON
                delay20us(4);
            }
        }
    }
    for (rowsign = 0; rowsign < 32; rowsign++)
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (badapplemap2[rowsign + 32][2 * isign] != badapplemap[rowsign + 32][2 * isign] || badapplemap2[rowsign + 32][2 * isign + 1] != badapplemap[rowsign + 32][2 * isign + 1])
            {
                write_command(0x36);
                delay20us(4);
                write_command(0x80 + rowsign);
                delay20us(4); //延时80us
                write_command(0x88 + isign);
                delay20us(4); //延时80us
                write_Data(badapplemap[rowsign + 32][2 * isign]);
                delay20us(4); //延时80us
                write_Data(badapplemap[rowsign + 32][2 * isign + 1]);
                delay20us(4);        //延时80us
                write_command(0x30); //使用扩展指令集，绘图显示ON
                delay20us(4);
            }
        }
    }
}

int fschuanshu(void)
{
    return fs;
}

void chengji(void)
{
    juli++;
    if ( juli > julivuzi) 
    {
        fs = 1 + (juli - julivuzi) / (pipe2pipe);
    }
}

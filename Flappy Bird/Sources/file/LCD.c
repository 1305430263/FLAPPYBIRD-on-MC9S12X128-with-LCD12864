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
int birdstate = 1; //Ϊ1ʱΪ�½�״̬��Ϊ0ʱΪ�����½�״̬��
int pipe2pipe = 40;

int badapplesign = 0;
int badapplemap[64][16];
int badapplemap2[64][16];
int badappleallmap[6400][16];

int julivuzi; 
int juli = 0;   //���ǰ���ľ��룬���Լ������;
int fs = 0;             //�÷�
/*************************************************************/
/*                      ��ʼ��Һ���ӿ�                       */
/*************************************************************/
void INIT_PORT(void)
{
    PSB_dir = 1;
    RS_dir = 1;
    RW_dir = 1;
    EN_dir = 1;
    DATA_dir = 0xff;
    DATA = 0;
    PSB = 1; //����ģʽ
    EN = 0;
    RW = 0;
    RS = 0;
}

/*************************************************************/
/*                     ��Һ����������                        */
/*************************************************************/
void write_Data(unsigned char b)
{
    RS = 1; //д���ݲ���
    RW = 0; //ָ��Ϊд����
    EN = 1;
    somenop();
    DATA = b;
    somenop();
    EN = 0;
}

/*************************************************************/
/*                      ��Һ����������                       */
/*************************************************************/
void write_command(unsigned char b)
{
    RS = 0; //д�������
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
/*                         ��ʱ����1                         */
/*************************************************************/
void delay20us(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        TFLG1_C0F = 1;  //�����־λ
        TC0 = TCNT + 5; //��������Ƚ�ʱ��Ϊ20us
        while (TFLG1_C0F == 0)
            ; //�ȴ���ֱ����������Ƚ��¼�
    }
}

/*************************************************************/
/*                         ��ʱ����2                         */
/*************************************************************/
void delay1ms(unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++)
    {
        TFLG1_C0F = 1;    //�����־λ
        TC0 = TCNT + 250; //��������Ƚ�ʱ��Ϊ1ms
        while (TFLG1_C0F == 0)
            ; //�ȴ���ֱ����������Ƚ��¼�
    }
}


/***************************************************************************/
/*                            ��������                                     */
/***************************************************************************/
void fill_GDRAM(uchar dat) //dat Ϊ��������
{
    uchar j, k;
    uchar GDRAM_X = 0x80; //GDRAM ˮƽ��ַ
    uchar GDRAM_Y = 0x80; //GDRAM ��ֱ��ַ
    write_command(0x34);  //ʹ����չָ�����ͼ��ʾOFF

    for (j = 0; j < 32; j++) //�ϰ���
    {
        write_command(GDRAM_X + j);
        delay20us(4); //��ʱ80us
        write_command(0x80);
        delay20us(4); //��ʱ80us
        for (k = 0; k < 16; k++)
        {
            write_Data(dat);
            delay20us(4);
        }
    }
    for (j = 0; j < 32; j++) //�°���
    {
        write_command(GDRAM_X + j);
        delay20us(4); //��ʱ80us
        write_command(0x88);
        delay20us(4); //��ʱ80us
        for (k = 0; k < 16; k++)
        {
            write_Data(dat);
            delay20us(4);
        }
    }

    write_command(0x36); // �򿪻�ͼģʽ
                         //write_command(0x30);// �ָ�����ָ����رջ�ͼģʽ
}


/***************************************************************************/
/*                              �������ɺ���                                */
/***************************************************************************/
void flappybirdmapcreate(void)
{
    int i, j;

    if (state == 0) //��һ��ִ��ʱ
    {
        julivuzi = firpipey + pipewidth;    //������һ�����ӵľ��룬���Լ������
        firpipex = roll[(rollsign)&0x07];
        secpipex = roll[(rollsign + 1) & 0x07];
        thipipex = roll[(rollsign + 2) & 0x07];
        fill_GDRAM(0x00);   //��һ��ˢ��ʱ���Ƚ���Ļ���пհ�ˢ��
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
        if (thipipey < 64) //�����������Ӹó���ʱ
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
                flappybirdmap2[i][j] = flappybirdmap[i][j]; //datamapΪ�ݴ溯��
                flappybirdmap[i][j] = 0x00;
            }
        }
        drawnewpipe(firpipex, firpipey, pipewidth, kongbai);
        secpipey = firpipey + pipe2pipe;
        if (secpipey < 64)
        {
            drawnewpipe(secpipex, secpipey, pipewidth, kongbai);
            thipipey = secpipey + pipe2pipe;
            if (thipipey < 64) //�����������Ӹó���ʱ
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
    for (rowsign = 0; rowsign < 32; rowsign++) //��������д��
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (flappybirdmap2[rowsign][2 * isign] != flappybirdmap[rowsign][2 * isign] || flappybirdmap2[rowsign][2 * isign + 1] != flappybirdmap[rowsign][2 * isign + 1])
            {
                write_command(0x36); //ʹ����չָ�����ͼ��ʾOFF
                delay20us(4);
                write_command(0x80 + rowsign); //��λ��
                delay20us(4);                  //��ʱ80us
                write_command(0x80 + isign);   //�ϰ�����λ��
                delay20us(4);                  //��ʱ80us
                write_Data(flappybirdmap[rowsign][2 * isign]);
                delay20us(4); //��ʱ80us
                write_Data(flappybirdmap[rowsign][2 * isign + 1]);
                delay20us(4);        //��ʱ80us
                write_command(0x30); //ʹ����չָ�����ͼ��ʾON
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
                delay20us(4); //��ʱ80us
                write_command(0x88 + isign);
                delay20us(4); //��ʱ80us
                write_Data(flappybirdmap[rowsign + 32][2 * isign]);
                delay20us(4); //��ʱ80us
                write_Data(flappybirdmap[rowsign + 32][2 * isign + 1]);
                delay20us(4);        //��ʱ80us
                write_command(0x30); //ʹ����չָ�����ͼ��ʾON
                delay20us(4);
                //delay1ms(3);
            }
        }
    }
}

/***************************************************************************/
/*                            ������ʾ��ͼ��                                */
/***************************************************************************/
void flappybirdmapchange(void)
{
    int i, ii;

    for (i = 0; i < 64; i++)
    {
        for (ii = 0; ii < 16; ii++)
        {
            flappybirdmap2[i][ii] = flappybirdmap[i][ii]; //datamapΪ�ݴ溯��
        }
    }
    for (i = 0; i < 64; i++)
    {
        for (ii = 0; ii < 16; ii++)
        {
            if (row_now + i < 64) //С��64��ֱ�Ӹ���
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
/*                             ����������                                   */
/*          a:�հײ�������  b��������ʼ���� c ���ӿ��  d �հ׿��                         */
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
/*                             �ݺ���                                    */
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
/*                             ��㺯��                                    */
/***************************************************************************/
void birdonepoint(int row, int i)
{
    uchar x_Dyte, x_byte;  // �����е�ַ���ֽ�λ�����ֽ��е���һλ
    x_Dyte = i / 8;    //���������һ���ֽڣ���ַ��,һ����ַ��16λ��
    x_byte = i % 8 ; // �����ڸ��ֽ��е� ��һλ
    row = row & 0x1f;
    flappybirdmap[row][x_Dyte] |= 0X01 << (7 - x_byte);
    panduan(row, i, firpipex, firpipey, pipewidth, kongbai);
    panduan(row, i, secpipex, secpipey, pipewidth, kongbai);
}

/***************************************************************************/
/*                             ȫ����                                    */
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
/*                             ��ײ�жϺ���                                 */
/***************************************************************************/

void panduan(int row, int i, int a, int b, int c, int d)
{
    if (row >= b && row <= b + c)
    {
        if (i <= (a - d / 2) || i >= (a + d / 2))
        {
            DEATH = 1;
            // state = 0; //��״̬��ʾ������ײ
            // birdl = 68;
            // rollsign = 0;
            // firpipey = 39;
            // juli = 0;
            // fs = 0;
            // delay1ms(200); //����ʱ��ʱ
            // return;
        }
    }
    if (i<=0 || i>=128)
    {
            DEATH = 1;
            // state = 0; //��״̬��ʾ������ײ
            // birdl = 68;
            // rollsign = 0;
            // firpipey = 39;
            // juli = 0;
            // fs = 0;
            // delay1ms(200); //����ʱ��ʱ
            // return;
    }
    
}

/***************************************************************************/
/*                             ��ײ�жϺ���                                 */
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
    state = 0; //��״̬��ʾ������ײ
    birdl = 68;
    rollsign = 0;
    firpipey = 39;
    juli = 0;
    fs = 0;
    DEATH = 0;
}



/*************************************************************/
/*                      ��������ʹ��������                     */
/*************************************************************/
int birdtiaodong(void)
{
  birdstate = 4;
}


/***************************************************************************/
/*                             ���ƶ�����                                    */
/***************************************************************************/
void birdmove(void)
{
    //�����ж���birdstate����Ϊ4
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
/*                            badappleͼֽ���캯��                         */
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
/*                             badapple���ƺ���                            */
/***************************************************************************/
void badapple(void)
{
    unsigned int isign, rowsign;
    for (rowsign = 0; rowsign < 32; rowsign++) //��������д��
    {
        for (isign = 0; isign < 8; isign++)
        {
            if (badapplemap2[rowsign][2 * isign] !=badapplemap[rowsign][2 * isign] || badapplemap2[rowsign][2 * isign + 1] !=badapplemap[rowsign][2 * isign + 1])
            {
                write_command(0x36);//ʹ����չָ�����ͼ��ʾOFF
                delay20us(4);
                write_command(0x80 + rowsign); //��λ��
                delay20us(4);                  //��ʱ80us
                write_command(0x80 + isign);   //�ϰ�����λ��
                delay20us(4);                  //��ʱ80us
                write_Data(badapplemap[rowsign][2 * isign]);
                delay20us(4); //��ʱ80us
                write_Data(badapplemap[rowsign][2 * isign + 1]);
                delay20us(4);        //��ʱ80us
                write_command(0x30); //ʹ����չָ�����ͼ��ʾON
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
                delay20us(4); //��ʱ80us
                write_command(0x88 + isign);
                delay20us(4); //��ʱ80us
                write_Data(badapplemap[rowsign + 32][2 * isign]);
                delay20us(4); //��ʱ80us
                write_Data(badapplemap[rowsign + 32][2 * isign + 1]);
                delay20us(4);        //��ʱ80us
                write_command(0x30); //ʹ����չָ�����ͼ��ʾON
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

/******************************************************************************
文 件 名   : lcd1602.c

@file lcd1602.c
@brief LCD1602驱动, 参考链接: http://www.elecfans.com/xianshi/20171020567470.html

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef LCD1602_DEV_DRIVER

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
#define DEBUG   ( 0 )

#if DEBUG
#define log(X)    print_string(X)
#else
#define log(X)
#endif

#define LCD1602_PORT  P1
sbit LCD1602_PIN_RS = P0^0;
sbit LCD1602_PIN_RW = P0^1;
sbit LCD1602_PIN_EN = P0^2;

/*----------------------------------------------*
 * 枚举定义                            *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                              *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/

/*---------------------------------------------------------------------------*/
/**
*@brief lcd1602延时1us
*
*
*@param none
*
*@return 
* 
*
*@note 根据数据手册,lcd的操作时序位ns级别,51单片机的指令周期为us级,因此延时时间远大于lcd操作时间
*
*/
static void lcd1602_delay_1us(void)
{
  _nop_();
}
/*---------------------------------------------------------------------------*/

/**
*@brief lcd1602延时1ms
*
*
*@param none
*
*@return 
* 
*
*@note 经测试,初始化时延时1ms,该延时是在 11.0592MHz 下计算所得
*
*/
void lcd1602_delay_1ms(void)
{
	unsigned char i, j;

	_nop_();
	i = 2;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
}

/*---------------------------------------------------------------------------*/
/**
*@brief 读取lcd1602状态是否忙
*
*
*@param none
*
*@return state 当前lcd状态
* 
*
*@note 
*
*/
static uint8_t lcd1602_read_state(void)
{
 	uint8_t state;

  ///下面为lcd操作时序
	LCD1602_PIN_RS = 0;
  LCD1602_PIN_RW = 1;
  LCD1602_PIN_EN = 1;
  lcd1602_delay_1us();
	state = LCD1602_PORT;
	LCD1602_PIN_EN = 0;
  lcd1602_delay_1us();
  
	return state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 等待lcd1602空闲
*
*
*@param none
*
*@return
* 
*
*@note 
*
*/
static void lcd1602_busy_wait(void)
{
  uint16_t timeout;

  timeout  = 0xffff;
 	while((lcd1602_read_state() & 0x80) == 0x80){
    timeout--;
    if(timeout == 0){
      log("lcd1602 state err\r\n");
      break;
    }
  }
	lcd1602_delay_1us();
}
/*---------------------------------------------------------------------------*/
/**
*@brief 向lcd1602写入数据
*
*
*@param dat 写入的数据值
*
*@return
* 
*
*@note 
*
*/
static void lcd1602_write_data(uint8_t dat)
{
  ///下面为lcd1602操作时序
 	lcd1602_busy_wait();
	LCD1602_PIN_RS = 1;
  LCD1602_PIN_RW = 0;
  LCD1602_PIN_EN = 0;
  LCD1602_PORT = dat;
  LCD1602_PIN_EN = 1;
  lcd1602_delay_1us();
  LCD1602_PIN_EN = 0;	
}
/*---------------------------------------------------------------------------*/
/**
*@brief 向lcd1602写入命令
*
*
*@param cmd 写入的命令值
*
*@return
* 
*
*@note 
*
*/
static void lcd1602_write_command(uint8_t cmd)
{
  ///下面为lcd1602操作时序
 	lcd1602_busy_wait();
	LCD1602_PIN_RS = 0;
  LCD1602_PIN_RW = 0;
  LCD1602_PIN_EN = 0;
  LCD1602_PORT = cmd;
  LCD1602_PIN_EN = 1;
  lcd1602_delay_1us();
  LCD1602_PIN_EN = 0;	
}
/*---------------------------------------------------------------------------*/
/**
*@brief lcd1602初始化
*
*
*@param none
*
*@return 
* 
*
*@note 
*
*/
void lcd1602_init(void)
{
 	lcd1602_write_command(0x38); ///<设置16 X 2显示, 5 X 7点阵, 8位数据接口
	lcd1602_delay_1ms();	
	lcd1602_write_command(0x01); ///<显示清0,数据指针清0
	lcd1602_delay_1ms();	
	lcd1602_write_command(0x06); ///<设置写一个字符后地址加1
	lcd1602_delay_1ms();	
	lcd1602_write_command(0x0c); ///<设置开显示,不显示光标
	lcd1602_delay_1ms();
}
/*---------------------------------------------------------------------------*/
/**
*@brief 在lcd1602上显示一个字符
*
*
*@param x x坐标值,取值范围 0 ~ 15
*@param y y坐标值,取值范围 0 ~ 1
*@param ch 要显示的字符
*
*@return 
* 
*
*@note 
*
*/
void lcd1602_display_char(    uint8_t      x, uint8_t y, uint8_t ch )
{
  if(x > 15 || y > 1){
    log("lcd1602 pos err\r\n");
    return;
  }
  if(y == 0){
    lcd1602_write_command(x | 0x80);///<设置LCD1602第一行要显示的光标位置
  }else if(y == 1){
    lcd1602_write_command(x | 0x80 | 0x40);///<设置LCD1602第二行要显示的光标位置
  }
  lcd1602_write_data( ch );
}
/*---------------------------------------------------------------------------*/
/**
*@brief lcd1602显示字符串
*
*
*@param x x坐标值,取值范围 0 ~ 15
*@param y y坐标值,取值范围 0 ~ 1
*@param str 要显示的字符串
*
*@return 
* 
*
*@note 
*
*/
void lcd1602_display_string(    uint8_t       x, uint8_t y, uint8_t * str )
{
  while(*str != '\0'){
    lcd1602_display_char(x, y, *str); ///<显示一个字符
    str++;  ///<显示下一个字符
    x++;    ///<显示下一个位置
    if(x > 15){
      log("lcd1602 display pos err\r\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 清空lcd1602显示
*
*
*@param none
*
*@return 
* 
*
*@note 
*
*/
void lcd1602_clear_display(void)
{
  lcd1602_write_command(0x01);
  lcd1602_delay_1ms();
}
/*---------------------------------------------------------------------------*/

#endif



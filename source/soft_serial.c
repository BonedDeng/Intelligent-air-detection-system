/******************************************************************************
文 件 名   : soft_serial.c

@file soft_serial.c
@brief 51 IO口模拟串口

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef SOFT_SERIAL
/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/

///模拟串口 软件发送引脚定义
sbit soft_serial_tx_pin = P3^5;

#define soft_serial_tx_set()      soft_serial_tx_pin = 1
#define soft_serial_tx_reset()    soft_serial_tx_pin = 0

#define SOFT_SERIAL_BAUD          9600 //4800/9600/38400/57600/115200

sbit ACC0 = ACC^0;
sbit ACC1 = ACC^1;
sbit ACC2 = ACC^2;
sbit ACC3 = ACC^3;
sbit ACC4 = ACC^4;
sbit ACC5 = ACC^5;
sbit ACC6 = ACC^6;
sbit ACC7 = ACC^7;
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
//static code uint8_t bit_dly = 104;///<计算电平延时: 1000000U/SOFT_SERIAL_BAUD

/*---------------------------------------------------------------------------*/
/**
*@brief  模拟串口,软件延时计算(在11.05926晶振下计算得到)
*
*
*@param none
*
*@return 
* 
*
*@note 给引脚赋值语句如: soft_serial_tx_pin = ACC0, 查看汇编代码可知,该条指令相当于2条汇编代码,因此延时时间要减2微妙
*
*/
static void soft_serial_delay(void)
{
#if (SOFT_SERIAL_BAUD == 115200)///<延时时间: 8us - 2us
	_nop_();
#elif (SOFT_SERIAL_BAUD == 9600)
  unsigned char i;///<延时时间: 104us - 2us
	_nop_();
	i = 44;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 57600)
  unsigned char i;///<延时时间: 17us - 2us
	i = 4;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 38400)
  unsigned char i;///<延时时间: 26us - 2us
	_nop_();
	i = 8;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 4800)
  unsigned char i;///<延时时间: 208us - 2us
	i = 92;
	while (--i);
#endif
}


/*---------------------------------------------------------------------------*/
/**
*@brief 软件模拟串口初始化
*
*
*@param none
*
*@return 
* 
*
*@note 9600波特率
*
*/
void soft_serial_init(void)
{
  soft_serial_tx_set();
}
/*---------------------------------------------------------------------------*/
/**
*@brief 模拟串口发送一个字符
*
*
*@param ch:要发送的字符串
*
*@return 
* 
*
*@note 9600波特率
*
*/
void soft_serial_putchar(uint8_t const ch)
{
  ACC = ch;
  
  __disable_irq();
  ///为了计算时间精确,这里不使用循环
  soft_serial_tx_reset();///<起始信号
  soft_serial_delay();

  soft_serial_tx_pin = ACC0;///<先送出低位,查看汇编代码可知,该条指令相当于2条汇编代码,因此延时时间要减2微妙
  soft_serial_delay();

  soft_serial_tx_pin = ACC1;
  soft_serial_delay();

  soft_serial_tx_pin = ACC2;
  soft_serial_delay();

  soft_serial_tx_pin = ACC3;
  soft_serial_delay();

  soft_serial_tx_pin = ACC4;
  soft_serial_delay();

  soft_serial_tx_pin = ACC5;
  soft_serial_delay();

  soft_serial_tx_pin = ACC6;
  soft_serial_delay();

  soft_serial_tx_pin = ACC7;
  soft_serial_delay();

  soft_serial_tx_set();///<停止信号
  soft_serial_delay();
  
  __enable_irq();
}
/*---------------------------------------------------------------------------*/
/**
*@brief 模拟串口发送数据
*
*
*@param buf: 数据指针
*@param size: 发送数据长度
*
*@return 
* 
*
*@note 9600波特率
*
*/
void soft_serial_transimt(const uint8_t * buf, uint16_t size)
{
  uint16_t i = 0;

  if(buf == NULL){
    return;
  }
  
  for(i = 0; i < size; i++){
    soft_serial_putchar(buf[i]);
  }

}
#endif//SOFT_SERIAL


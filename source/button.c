/******************************************************************************
文 件 名   : button.c
 					
@file button.c
@brief 独立按键驱动

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef BUTTON_DEV_DRIVER

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
#define DEBUG   ( 0 )

#if DEBUG
#define log(X)    print_string(X)
#else
#define log(X)
#endif


///独立按键按下/抬起状态值
#define BUTTON_VALUE_DOWN     ( 0 )
#define BUTTON_VALUE_UP       ( 1 )

///独立按键引脚定义
sbit KEY0_PIN = P2^0;
sbit KEY1_PIN = P2^1;
sbit KEY2_PIN = P2^2;
sbit KEY3_PIN = P2^3;
sbit KEY4_PIN = P2^4;
sbit KEY5_PIN = P1^5;
sbit KEY6_PIN = P1^6;
sbit KEY7_PIN = P1^7;
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
*@brief 按键消抖延时函数
*
*
*@param none
*
*@return 
* 
*
*@note 该延时函数是在11.0592MHz晶振下计算所得
*
*/
static void key_delay_5ms(void)
{
	unsigned char i, j;

	i = 9;
	j = 244;
	do
	{
		while (--j);
	} while (--i);
}
/*---------------------------------------------------------------------------*/
/**
*@brief 获得按键当前值
*
*
*@param key_num 独立按键编号 0 ~ 7
*
*@return IO口值0/1
* 
*
*@note 
*
*/
static uint8_t get_key_value(uint8_t key_num)
{
  uint8_t key_value;

  switch(key_num){
    case 0:
      key_value = KEY0_PIN;
      break;
    case 1:
      key_value = KEY1_PIN;
      break;
    case 2:
      key_value = KEY2_PIN;
      break;
    case 3:
      key_value = KEY3_PIN;
      break;
    case 4:
      key_value = KEY4_PIN;
      break;
    case 5:
      key_value = KEY5_PIN;
      break;
    case 6:
      key_value = KEY6_PIN;
      break;
    case 7:
      key_value = KEY7_PIN;
      break;
    default:
      return BUTTON_VALUE_UP;
  }

  return key_value;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 检测按键是否按下
*
*
*@param key_num 独立按键编号 0 ~ 7
*
*@return true 按键按下, false 按键未按下
* 
*
*@note 目前独立按键最大支持8个,即一组IO口
*
*/
bool is_key_down(uint8_t key_num)
{
  uint8_t i = 0;

  if(key_num > 7){
    log("key num over\r\n");
    return false;
  }
  
  if(get_key_value(key_num) == BUTTON_VALUE_DOWN){
    key_delay_5ms();
    if(get_key_value(key_num) == BUTTON_VALUE_DOWN){
      for(i = 0; i < 80; i++){
        key_delay_5ms();
        if(get_key_value(key_num) == BUTTON_VALUE_UP){
          break;
        }
      }
      return true;
    }
  }
  return false;
}

/*---------------------------------------------------------------------------*/
#endif//BUTTON_DEV_DRIVER


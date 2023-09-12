/******************************************************************************
文 件 名   : timer0.c
 
@file timer0.c
@brief 定时器0驱动

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef TIMER0_DEV_DRIVER

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
#define TIMER0_MODE1_DEV_DRIVER


#ifdef TIMER0_MODE1_DEV_DRIVER

#define TIMER0_MODE1_INIT_VALUE_H_50MS    (0x4C)  ///<11.05926M晶振下计算得到
#define TIMER0_MODE1_INIT_VALUE_L_50MS    (0x00)  ///<11.05926M晶振下计算得到

#define TIMER0_MODE1_INIT_VALUE_H_10MS    (0xDC)  ///<11.05926M晶振下计算得到
#define TIMER0_MODE1_INIT_VALUE_L_10MS    (0x00)  ///<11.05926M晶振下计算得到

#define TIMER0_MODE1_INIT_VALUE_H_1MS     (0xFC)  ///<11.05926M晶振下计算得到
#define TIMER0_MODE1_INIT_VALUE_L_1MS     (0x66)  ///<11.05926M晶振下计算得到

#define TIMER0_MODE1_INIT_VALUE_H         TIMER0_MODE1_INIT_VALUE_H_1MS
#define TIMER0_MODE1_INIT_VALUE_L         TIMER0_MODE1_INIT_VALUE_L_1MS
#endif

///定时器0中断定义
//#define timer0_irq()    
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

volatile clock_time_t clock_ticks = 0;
/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/

/*---------------------------------------------------------------------------*/


#ifdef TIMER0_MODE1_DEV_DRIVER
/**
*@brief 定时器0初始化
*
*
*@param none
*
*@return 
* 
*
*@note 定时器0工作在方式1,即16位计数器
*
*/
void timer0_mode1_init(void)
{
	TMOD &= 0xF0;		///<设置定时器0为方式1
	TMOD |= 0x01;		///<设置定时器0为方式1
	TL0 = TIMER0_MODE1_INIT_VALUE_L;///<设定定时器初值
	TH0 = TIMER0_MODE1_INIT_VALUE_H;			
	TF0 = 0;		///<清除TF0标志
	TR0 = 1;		///<定时器0开始计时
  ET0 = 1;    ///<允许定时/计数器0中断
}

#else
/**
*@brief 定时器0初始化
*
*
*@param none
*
*@return 
* 
*
*@note 定时器0工作在方式2,即8位自动重载
*
*/
void timer0_mode2_init(void)
{
  TMOD &= 0xF0;		///<设置定时器模式为方式2
	TMOD |= 0x02;		///<设置定时器模式
	TL0 = 0x48;		  ///<设置定时初值, 200us定时
	TH0 = 0x48;		  ///<设置定时重载值
	TF0 = 0;		    ///<清除TF0标志
	TR0 = 1;		    ///<定时器0开始计时
  ET0 = 1;        ///<允许定时/计数器0中断
}
#endif
/*---------------------------------------------------------------------------*/
/**
*@brief 延时毫秒时间
*
*
*@param Delay: 延时的毫秒
*
*@return none
* 
*
*@note 只能定时器0工作在方式1,即16位计数器,并且1毫秒中断一次
*
*/
void HAL_Delay(uint32_t Delay)
{
  clock_time_t tickstart = HAL_GetTick();
  clock_time_t wait = Delay;

  /* Add a freq to guarantee minimum wait */
  if (wait < 0xFFFFU){
    wait += 1u;
  }

  while ((HAL_GetTick() - tickstart) < wait){
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 定时器0中断函数入口
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
void timer0() interrupt 1
{
#ifdef TIMER0_MODE1_DEV_DRIVER
  TL0 = TIMER0_MODE1_INIT_VALUE_L;///<设定定时器初值
	TH0 = TIMER0_MODE1_INIT_VALUE_H;	

  timer0_irq();///<定时器0中断处理函数
#else
  timer0_irq();///<定时器0中断处理函数
#endif
}
#endif
/*---------------------------------------------------------------------------*/


/******************************************************************************
�� �� ��   : soft_serial.c

@file soft_serial.c
@brief 51 IO��ģ�⴮��

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef SOFT_SERIAL
/*----------------------------------------------*
 * �궨��                                 *
 *----------------------------------------------*/

///ģ�⴮�� ����������Ŷ���
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
 * ö�ٶ���                            *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ṹ�嶨��                              *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                       *
 *----------------------------------------------*/
//static code uint8_t bit_dly = 104;///<�����ƽ��ʱ: 1000000U/SOFT_SERIAL_BAUD

/*---------------------------------------------------------------------------*/
/**
*@brief  ģ�⴮��,�����ʱ����(��11.05926�����¼���õ�)
*
*
*@param none
*
*@return 
* 
*
*@note �����Ÿ�ֵ�����: soft_serial_tx_pin = ACC0, �鿴�������֪,����ָ���൱��2��������,�����ʱʱ��Ҫ��2΢��
*
*/
static void soft_serial_delay(void)
{
#if (SOFT_SERIAL_BAUD == 115200)///<��ʱʱ��: 8us - 2us
	_nop_();
#elif (SOFT_SERIAL_BAUD == 9600)
  unsigned char i;///<��ʱʱ��: 104us - 2us
	_nop_();
	i = 44;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 57600)
  unsigned char i;///<��ʱʱ��: 17us - 2us
	i = 4;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 38400)
  unsigned char i;///<��ʱʱ��: 26us - 2us
	_nop_();
	i = 8;
	while (--i);
#elif (SOFT_SERIAL_BAUD == 4800)
  unsigned char i;///<��ʱʱ��: 208us - 2us
	i = 92;
	while (--i);
#endif
}


/*---------------------------------------------------------------------------*/
/**
*@brief ���ģ�⴮�ڳ�ʼ��
*
*
*@param none
*
*@return 
* 
*
*@note 9600������
*
*/
void soft_serial_init(void)
{
  soft_serial_tx_set();
}
/*---------------------------------------------------------------------------*/
/**
*@brief ģ�⴮�ڷ���һ���ַ�
*
*
*@param ch:Ҫ���͵��ַ���
*
*@return 
* 
*
*@note 9600������
*
*/
void soft_serial_putchar(uint8_t const ch)
{
  ACC = ch;
  
  __disable_irq();
  ///Ϊ�˼���ʱ�侫ȷ,���ﲻʹ��ѭ��
  soft_serial_tx_reset();///<��ʼ�ź�
  soft_serial_delay();

  soft_serial_tx_pin = ACC0;///<���ͳ���λ,�鿴�������֪,����ָ���൱��2��������,�����ʱʱ��Ҫ��2΢��
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

  soft_serial_tx_set();///<ֹͣ�ź�
  soft_serial_delay();
  
  __enable_irq();
}
/*---------------------------------------------------------------------------*/
/**
*@brief ģ�⴮�ڷ�������
*
*
*@param buf: ����ָ��
*@param size: �������ݳ���
*
*@return 
* 
*
*@note 9600������
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


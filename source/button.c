/******************************************************************************
�� �� ��   : button.c
 					
@file button.c
@brief ������������

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef BUTTON_DEV_DRIVER

/*----------------------------------------------*
 * �궨��                                 *
 *----------------------------------------------*/
#define DEBUG   ( 0 )

#if DEBUG
#define log(X)    print_string(X)
#else
#define log(X)
#endif


///������������/̧��״ֵ̬
#define BUTTON_VALUE_DOWN     ( 0 )
#define BUTTON_VALUE_UP       ( 1 )

///�����������Ŷ���
sbit KEY0_PIN = P2^0;
sbit KEY1_PIN = P2^1;
sbit KEY2_PIN = P2^2;
sbit KEY3_PIN = P2^3;
sbit KEY4_PIN = P2^4;
sbit KEY5_PIN = P1^5;
sbit KEY6_PIN = P1^6;
sbit KEY7_PIN = P1^7;
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


/*---------------------------------------------------------------------------*/
/**
*@brief ����������ʱ����
*
*
*@param none
*
*@return 
* 
*
*@note ����ʱ��������11.0592MHz�����¼�������
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
*@brief ��ð�����ǰֵ
*
*
*@param key_num ����������� 0 ~ 7
*
*@return IO��ֵ0/1
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
*@brief ��ⰴ���Ƿ���
*
*
*@param key_num ����������� 0 ~ 7
*
*@return true ��������, false ����δ����
* 
*
*@note Ŀǰ�����������֧��8��,��һ��IO��
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


/******************************************************************************
�� �� ��   : ad_da_pcf8591.c

@file ad_da_pcf8591.c
@brief AD/DA pcf8591����(4·8λADC, 1·DAC)
******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef AD_DA_PFC8591_DEV_DRIVER

/*----------------------------------------------*
 * �궨��                                 *
 *----------------------------------------------*/
#define DEBUG     1
#if DEBUG
#define log(X)    print_string(X)
#else
#define log(X)
#endif

///PCF8591 I2C�������Ŷ���
sbit PCF8591_I2C_SCL = P0^4;
sbit PCF8591_I2C_SDA = P0^3;

#define  PCF8591_I2C_ADDR         ( 0x90 )///<PCF8591_I2C_ADDR I2C��ַ(��A2��A1��A0�й�)

#define i2c_scl_low()             PCF8591_I2C_SCL = 0
#define i2c_scl_high()            PCF8591_I2C_SCL = 1
#define i2c_sda_low()             PCF8591_I2C_SDA = 0
#define i2c_sda_high()            PCF8591_I2C_SDA = 1
#define i2c_sda_read()            PCF8591_I2C_SDA

///I2CӦ����
#define I2C_ACK                   ( 0 )///<Ӧ��
#define I2C_NOACK                 ( 1 )///<��Ӧ��

///I2C ��/д
#define I2C_WRITE                 0x00 ///<I2C д
#define I2C_READ                  0x01 ///<I2C ��

///PCF8591�ο���ѹ
#define PCF8591_AD_REF_VALTAGE    (5000ul)
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
*@brief ��ʼ��I2C����IO��
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
static void I2C_init(void)
{
  ///��λ״̬
  i2c_scl_high();
  i2c_sda_high();
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C��ʱ10us
*
*
*@param none
*
*@return 
* 
*
*@note ����I2C 100k��Ƶ���� 11.0592MHz ��ż���õ�
*
*/
static void I2C_delay_10us(void)
{
  unsigned char i;

	i = 2;
	while (--i);
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C��ʱ10ms
*
*
*@param none
*
*@return 
* 
*
*@note ����ʱ�� 11.0592MHz ����õ�
*
*/
static void I2C_delay_10ms(void)
{
	unsigned char i, j;

	i = 18;
	j = 235;
	do
	{
		while (--j);
	} while (--i);
}
/*---------------------------------------------------------------------------*/
/**
*@brief ����I2C��ʼ�ź�
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
static void I2C_start(void)
{
  i2c_scl_low();
  I2C_delay_10us();
  i2c_sda_high();
  I2C_delay_10us();
  i2c_scl_high();
  I2C_delay_10us();
  i2c_sda_low();
  I2C_delay_10us();
}
/*---------------------------------------------------------------------------*/
/**
*@brief ����I2Cֹͣ�ź�
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
static void I2C_stop(void)
{
    i2c_scl_low();
    I2C_delay_10us();
    i2c_sda_low();
    I2C_delay_10us();
    i2c_scl_high();
    I2C_delay_10us();
    i2c_sda_high();
    I2C_delay_10us();
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C����ȷ���ź�
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
static void I2C_ack(void)
{
  i2c_scl_low();
  I2C_delay_10us();
  i2c_sda_low();
  i2c_scl_high();
  I2C_delay_10us();
  i2c_scl_low();
  I2C_delay_10us();
  i2c_sda_high();
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C����ȷ���ź�
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
static void I2C_no_ack(void)
{
  i2c_scl_low();
  I2C_delay_10us();
  i2c_sda_high();
  I2C_delay_10us();
  i2c_scl_high();
  I2C_delay_10us();
  i2c_scl_low();
  I2C_delay_10us();
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2Cдһ���ֽ�����
*
*
*@param input Ҫд�������
*
*@return 0 �ɹ� 1ʧ��
* 
*
*@note 
*
*/
static bool I2C_write_Byte(uint8_t input)
{
  uint8_t i;
  
  for (i = 0; i < 8; i++){
    i2c_scl_low();
    input <<= 1;
    PCF8591_I2C_SDA = CY;
    i2c_scl_high();
  }
  i2c_scl_low();
  I2C_delay_10us();
  i2c_sda_high();
  i2c_scl_high();
  I2C_delay_10us();
  CY = i2c_sda_read();
  
  return (CY) ;
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C��һ���ֽ�����
*
*
*@param none
*
*@return �������ֽ�����
* 
*
*@note 
*
*/
static uint8_t I2C_read_Byte(void)
{
  uint8_t temp,rbyte = 0;
  
  for (temp = 8;temp != 0;temp--){
    i2c_scl_low();
    I2C_delay_10us();
    rbyte = (rbyte << 1) | i2c_sda_read();
    i2c_scl_high();
    I2C_delay_10us();
  }
  return rbyte;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ��pcf8591����AD����
*
*
*@param channel Ҫ��ȡ��ADֵ�Ŀ�ʼͨ��, ��Χ: 0 ~ 3
*@param pdat ָ��Ҫ��pcf8591�������ݵ�buf
*@param channel_num ����PCF8591ֵͨ������,PCF8591��4��ͨ��,����󳤶�Ϊ4
*
*@return false : ʧ��, true : �ɹ�
* 
*
*@note pcf8591 ��������ת��4��ͨ��
*
*/
bool adc_pcf8591_read_ad(uint8_t channel, uint8_t* pdat,uint16_t channel_num)
{
  uint8_t ack;
  uint8_t* p = pdat;
  uint8_t ctrl;
  
  if((p == NULL) || (channel_num == 0) || (channel_num > 4) | (channel > 3)){
    log("pcf8591 read ad param err\r\n");
    return false;
  }

  if(channel_num == 1){
    ctrl = 0x40 + channel;///<��1��ͨ��,�ʲ���������ת��
  }else{
    ctrl = 0x44 + channel;///<��ȡ���ͨ��,����ADC����ת��
  }

  __disable_irq();
  
  ///1.��ʼ
  I2C_start();

  ///2.�豸��ַ/д
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_WRITE);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///3.д��pcf8591��������
  ack = I2C_write_Byte(ctrl);   //pcf8591��������(8λ)
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }
  
  ///4.���¿�ʼ
  I2C_start();

  ///5.�豸��ַ/��
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_READ);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  I2C_read_Byte();///<��һ�����ֽ�,��Ϊ����ADCת��ʱ��,���:pcf8591 datasheet, ͼ: Fig.9 A/D conversion sequence
  I2C_ack();
  ///6.������
  while(--channel_num){
    *p = I2C_read_Byte();///<��������
    p++;                 ///<ָ���1
    I2C_ack();           ///<��ȷ���ź�
   }                                   
  *p = I2C_read_Byte();///<��ȡ���һ�ֽ�
  I2C_no_ack();        ///<û��ȷ���ź�

  ///7.ֹͣ
  I2C_stop();
  
  __enable_irq();
  
  return true;
I2C_STOP:
  I2C_stop();
  I2C_delay_10ms();
  log("I2C read err\r\n");
  return false;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ����ADֵ����pcf8591���DA��ѹ
*
*
*@param ad_value ADֵ
*
*@return false : ʧ��, true : �ɹ�
* 
*
*@note 
*
*/
bool dac_pcf8591_from_ad_val(uint8_t ad_value)
{
  uint8_t  ack;

  __disable_irq();

  ///1.��ʼ
  I2C_start();

  ///2.�豸��ַ/д
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_WRITE);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///3.д��pcf8591��������
  ack = I2C_write_Byte((uint8_t)(0x40));
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///4.д��pcf8591 dac�Ĵ���ֵ
  if(I2C_write_Byte(ad_value) == I2C_NOACK){///<���ֽڿ�д��,��Ϊ���ֽ�Ϊת��ʱ��,���:pcf8591 datasheet, ͼ: Fig.8 D/A conversion sequence.
      goto I2C_STOP;
  }

  if(I2C_write_Byte(ad_value) == I2C_NOACK){
      goto I2C_STOP;
  }
  
  ///5.ֹͣ
  I2C_stop();
  __enable_irq();
  return true;


I2C_STOP:
  I2C_stop();
  I2C_delay_10ms();
  log("I2C write err\r\n");
  return false;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ���ݵ�ѹֵ����pcf8591���DA��ѹ
*
*
*@param voltage_mv ��ѹֵ(mv)
*
*@return false : ʧ��, true : �ɹ�
* 
*
*@note 
*
*/
bool dac_pcf8591_from_voltage_mv(uint16_t voltage_mv)
{
  uint32_t temp;

  temp = voltage_mv;
  temp <<= 8;
  temp /= PCF8591_AD_REF_VALTAGE;
  
  return dac_pcf8591_from_ad_val(temp);
}
/*---------------------------------------------------------------------------*/
/**
*@brief ��pcf8591����AD��ѹֵ,��λmv
*
*
*@param channel Ҫ��ȡ��ADֵ�Ŀ�ʼͨ��, ��Χ: 0 ~ 3
*@param pdat ָ��Ҫ��pcf8591�������ݵ�buf
*@param channel_num ����PCF8591ֵͨ������,PCF8591��4��ͨ��,����󳤶�Ϊ4
*
*@return false : ʧ��, true : �ɹ�
* 
*
*@note pcf8591 ��������ת��4��ͨ��
*
*/

bool adc_pcf8591_read_voltage_mv(uint8_t channel, uint16_t* pdat,uint16_t channel_num)
{
  uint32_t temp;
  uint8_t ad_value[4];
  uint8_t i;

  if(pdat == NULL){
    log("read_voltage pdat err\r\n");
    return false;
  }
  
  if(adc_pcf8591_read_ad(channel, ad_value, channel_num)){
    for(i = 0; i < channel_num; i++){
      temp = PCF8591_AD_REF_VALTAGE * ad_value[i];
      temp >>= 8;
      pdat[i] = temp;
    }
    return true;
  }else{
    log("read_voltage err\r\n");
    return false;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief ��ʼ��pcf8591
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
void ad_da_pcf8591_init(void)
{
  I2C_init();
}
/*---------------------------------------------------------------------------*/
#endif//AD_DA_PFC8591_DEV_DRIVER


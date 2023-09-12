/******************************************************************************
文 件 名   : ad_da_pcf8591.c

@file ad_da_pcf8591.c
@brief AD/DA pcf8591驱动(4路8位ADC, 1路DAC)
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef AD_DA_PFC8591_DEV_DRIVER

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
#define DEBUG     1
#if DEBUG
#define log(X)    print_string(X)
#else
#define log(X)
#endif

///PCF8591 I2C操作引脚定义
sbit PCF8591_I2C_SCL = P0^4;
sbit PCF8591_I2C_SDA = P0^3;

#define  PCF8591_I2C_ADDR         ( 0x90 )///<PCF8591_I2C_ADDR I2C地址(与A2、A1、A0有关)

#define i2c_scl_low()             PCF8591_I2C_SCL = 0
#define i2c_scl_high()            PCF8591_I2C_SCL = 1
#define i2c_sda_low()             PCF8591_I2C_SDA = 0
#define i2c_sda_high()            PCF8591_I2C_SDA = 1
#define i2c_sda_read()            PCF8591_I2C_SDA

///I2C应答定义
#define I2C_ACK                   ( 0 )///<应答
#define I2C_NOACK                 ( 1 )///<非应答

///I2C 读/写
#define I2C_WRITE                 0x00 ///<I2C 写
#define I2C_READ                  0x01 ///<I2C 读

///PCF8591参考电压
#define PCF8591_AD_REF_VALTAGE    (5000ul)
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
*@brief 初始化I2C引脚IO口
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
  ///置位状态
  i2c_scl_high();
  i2c_sda_high();
}
/*---------------------------------------------------------------------------*/
/**
*@brief I2C延时10us
*
*
*@param none
*
*@return 
* 
*
*@note 按照I2C 100k的频率在 11.0592MHz 大概计算得到
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
*@brief I2C延时10ms
*
*
*@param none
*
*@return 
* 
*
*@note 该延时在 11.0592MHz 计算得到
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
*@brief 产生I2C开始信号
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
*@brief 产生I2C停止信号
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
*@brief I2C发送确认信号
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
*@brief I2C发无确认信号
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
*@brief I2C写一个字节数据
*
*
*@param input 要写入的数据
*
*@return 0 成功 1失败
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
*@brief I2C读一个字节数据
*
*
*@param none
*
*@return 读到的字节数据
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
*@brief 从pcf8591读出AD数据
*
*
*@param channel 要读取的AD值的开始通道, 范围: 0 ~ 3
*@param pdat 指向要从pcf8591读出数据的buf
*@param channel_num 读出PCF8591值通道个数,PCF8591共4个通道,故最大长度为4
*
*@return false : 失败, true : 成功
* 
*
*@note pcf8591 可以连续转换4个通道
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
    ctrl = 0x40 + channel;///<读1个通道,故不启用连续转换
  }else{
    ctrl = 0x44 + channel;///<读取多个通道,启用ADC连续转换
  }

  __disable_irq();
  
  ///1.开始
  I2C_start();

  ///2.设备地址/写
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_WRITE);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///3.写入pcf8591控制命令
  ack = I2C_write_Byte(ctrl);   //pcf8591控制命令(8位)
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }
  
  ///4.重新开始
  I2C_start();

  ///5.设备地址/读
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_READ);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  I2C_read_Byte();///<读一个空字节,因为这是ADC转换时间,详见:pcf8591 datasheet, 图: Fig.9 A/D conversion sequence
  I2C_ack();
  ///6.读数据
  while(--channel_num){
    *p = I2C_read_Byte();///<读出内容
    p++;                 ///<指针加1
    I2C_ack();           ///<发确认信号
   }                                   
  *p = I2C_read_Byte();///<读取最后一字节
  I2C_no_ack();        ///<没有确认信号

  ///7.停止
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
*@brief 根据AD值设置pcf8591输出DA电压
*
*
*@param ad_value AD值
*
*@return false : 失败, true : 成功
* 
*
*@note 
*
*/
bool dac_pcf8591_from_ad_val(uint8_t ad_value)
{
  uint8_t  ack;

  __disable_irq();

  ///1.开始
  I2C_start();

  ///2.设备地址/写
  ack = I2C_write_Byte(PCF8591_I2C_ADDR | I2C_WRITE);
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///3.写入pcf8591控制命令
  ack = I2C_write_Byte((uint8_t)(0x40));
  if(I2C_NOACK == ack){
    goto I2C_STOP;
  }

  ///4.写入pcf8591 dac寄存器值
  if(I2C_write_Byte(ad_value) == I2C_NOACK){///<该字节空写入,因为该字节为转换时间,详见:pcf8591 datasheet, 图: Fig.8 D/A conversion sequence.
      goto I2C_STOP;
  }

  if(I2C_write_Byte(ad_value) == I2C_NOACK){
      goto I2C_STOP;
  }
  
  ///5.停止
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
*@brief 根据电压值设置pcf8591输出DA电压
*
*
*@param voltage_mv 电压值(mv)
*
*@return false : 失败, true : 成功
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
*@brief 从pcf8591读出AD电压值,单位mv
*
*
*@param channel 要读取的AD值的开始通道, 范围: 0 ~ 3
*@param pdat 指向要从pcf8591读出数据的buf
*@param channel_num 读出PCF8591值通道个数,PCF8591共4个通道,故最大长度为4
*
*@return false : 失败, true : 成功
* 
*
*@note pcf8591 可以连续转换4个通道
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
*@brief 初始化pcf8591
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


#ifndef __MAIN_H__
#define __MAIN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <intrins.h>
#include <math.h>
#include <reg52.h>

/*---------------------------------------------------------------------------*/
#define __enable_irq()        EA = 1///<使能全局中断
#define __disable_irq()       EA = 0///<失能全局中断

///定义true/false
#ifndef true
#define	true	      ( 1 )
#endif

#ifndef false
#define	false	      ( 0 )
#endif


///对已有变量进行重命名
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned int uint16_t;
typedef int int16_t;

typedef unsigned long int uint32_t;
typedef long int int32_t;

typedef bit bool;

typedef uint16_t clock_time_t;

/*---------------------------------------------------------------------------*/
///常用宏定义
#define   INC_LIMIT(VAL,LIMIT_VAL)    (VAL) < (LIMIT_VAL) ? (++VAL): (VAL = (LIMIT_VAL))
#define   DEC_LIMIT(VAL,LIMIT_VAL)    (VAL) > (LIMIT_VAL) ? (--VAL): (VAL = (LIMIT_VAL))
#define   ENUM_TO_STR(e)              (#e)

#define CHARISNUM(x)    ((x) >= '0' && (x) <= '9')
#define CHAR2NUM(x)     ((x) - '0')
#define CHARTONUM(x)    ((x) - '0')
/*---------------------------------------------------------------------------*/
#define UART_DEV_DRIVER
#define TIMER0_DEV_DRIVER
#define SOFT_SERIAL
#define LCD1602_DEV_DRIVER
#define __DHT11_DEV_DRIVER__
#define AD_DA_PFC8591_DEV_DRIVER
#define __ESP8266_AP_MODE__
#define USE_RINGBUF
#define BUTTON_DEV_DRIVER
/*---------------------------------------------------------------------------*/
#ifdef UART_DEV_DRIVER
void uart_init(void);
void uart_put_char(uint8_t ch);
void uart_block_send(uint8_t * buf, uint16_t len);
uint16_t uart_block_receive(uint8_t *pData, uint16_t Size, uint32_t Timeout);
void print_string(uint8_t code *puts);
void uart_rx_irq(uint8_t ch);
void uart_tx_irq(void);
#endif
/*---------------------------------------------------------------------------*/
#ifdef TIMER0_DEV_DRIVER
extern volatile clock_time_t clock_ticks;
#define HAL_GetTick() clock_ticks

#define time_event_define(name) \
  clock_time_t time_event_base##name;\
  bool is_start_time_event##name = false

#define time_event_declare(name) \
  extern clock_time_t time_event_base##name;\
  extern bool is_start_time_event##name

#define time_event_start(name) \
  do{\
      time_event_base##name = HAL_GetTick();\
      is_start_time_event##name = true;\
    }while(0)

#define time_event_stop(name) \
  do{\
      is_start_time_event##name = false;\
    }while(0)
		
#define time_event_loop(name, time_event_timeout_handle, timeout) \
  do{\
    if(is_start_time_event##name && (clock_time_t)(HAL_GetTick() - time_event_base##name) > timeout){ \
        is_start_time_event##name = false;\
        time_event_timeout_handle();\
      }\
    }while(0)

void timer0_mode1_init(void);
void timer0_irq(void);
void timer0_mode2_init(void);
void HAL_Delay(uint32_t Delay);
#endif
/*---------------------------------------------------------------------------*/
#ifdef SOFT_SERIAL
void soft_serial_init(void);
void soft_serial_putchar(uint8_t const ch);
void soft_serial_transimt(const uint8_t * buf, uint16_t size);
#endif
/*---------------------------------------------------------------------------*/
#ifdef LCD1602_DEV_DRIVER
void lcd1602_init(void);
void lcd1602_display_char(    uint8_t      x, uint8_t y, uint8_t ch );
void lcd1602_display_string(    uint8_t       x, uint8_t y, uint8_t * str );
void lcd1602_clear_display(void);
#endif
/*---------------------------------------------------------------------------*/
#ifdef __DHT11_DEV_DRIVER__
bool dht11_read_data(uint8_t *temperature, uint8_t * humidity);
#endif
/*---------------------------------------------------------------------------*/
#ifdef AD_DA_PFC8591_DEV_DRIVER
void ad_da_pcf8591_init(void);
bool adc_pcf8591_read_ad(uint8_t channel, uint8_t* pdat,uint16_t channel_num);
bool adc_pcf8591_read_voltage_mv(uint8_t channel, uint16_t* pdat,uint16_t channel_num);
bool dac_pcf8591_from_ad_val(uint8_t ad_value);
bool dac_pcf8591_from_voltage_mv(uint16_t voltage_mv);
#endif
/*---------------------------------------------------------------------------*/
#ifdef __ESP8266_AP_MODE__
void esp8266_uart_rec_irq(uint8_t ch);
void esp8266_init(void);
void esp8266_loop(void);
bool esp8266_send_data(const uint8_t *dat, uint8_t dat_len);
void remote_data_handle(const uint8_t *buf, uint8_t len);
#endif
/*---------------------------------------------------------------------------*/
#ifdef USE_RINGBUF
struct ringbuf {
  uint8_t *dat;
  uint8_t mask;
  /* XXX these must be 8-bit quantities to avoid race conditions. */
  uint8_t put_ptr, get_ptr;
};
void ringbuf_init(struct ringbuf *r, uint8_t *a,uint8_t size_power_of_two);
int ringbuf_put(struct ringbuf *r, uint8_t c);
int ringbuf_get(struct ringbuf *r);
int ringbuf_size(struct ringbuf *r);
int ringbuf_elements(struct ringbuf *r);
#endif
/*---------------------------------------------------------------------------*/
#ifdef BUTTON_DEV_DRIVER
bool is_key_down(uint8_t key_num);
#endif
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */





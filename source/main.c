/******************************************************************************
文 件 名   : main.c

@file main.c
@brief 主函数文件
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/

#define DISPLAY_INDEX_CO      ( 0 )///<一氧化碳显示
#define DISPLAY_INDEX_YANWU   ( 1 )///<烟雾显示
#define DISPLAY_INDEX_CH4     ( 2 )///<甲醛

sbit beep_pin = P3^7;
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
time_event_define(get_sensor_value);

static uint8_t pcf8591_ad[3];
static idata uint8_t display_buf[64];

static uint8_t display_index = DISPLAY_INDEX_CO;

static uint8_t CO_warning = 150;
static uint8_t SMOKE_warning = 150;
static uint8_t CH4_warning = 150;
/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/

/**
*@brief 显示告警值
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void display_warning_string(void)
{
  if(display_index == DISPLAY_INDEX_CO){
    sprintf(display_buf, "%03bu", CO_warning);
    lcd1602_display_string(sizeof("Warning:") - 1, 1, display_buf);
  }else if(display_index == DISPLAY_INDEX_YANWU){
    sprintf(display_buf, "%03bu", SMOKE_warning);
    lcd1602_display_string(sizeof("Warning:") - 1, 1, display_buf);
  }else if(display_index == DISPLAY_INDEX_CH4){
    sprintf(display_buf, "%03bu", CH4_warning);
    lcd1602_display_string(sizeof("Warning:") - 1, 1, display_buf);
  }
}

/*---------------------------------------------------------------------------*/
/**
*@brief 显示固定字符
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void display_fix_string(void)
{
  if(display_index == DISPLAY_INDEX_CO){
    lcd1602_display_string(0, 0, "CO:");
  }else if(display_index == DISPLAY_INDEX_YANWU){
    lcd1602_display_string(0, 0, "Smoke:");
  }else if(display_index == DISPLAY_INDEX_CH4){
    lcd1602_display_string(0, 0, "HCHO:");
  }
  lcd1602_display_string(0, 1, "Warning:");
}
/*---------------------------------------------------------------------------*/
/**
*@brief 显示固定字符
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void display_sensor_value(void)
{
  if(display_index == DISPLAY_INDEX_CO){
    sprintf(display_buf, "%bu  ", pcf8591_ad[0]);
    lcd1602_display_string(sizeof("CO:"), 0, display_buf);
  }else if(display_index == DISPLAY_INDEX_YANWU){
    sprintf(display_buf, "%bu  ", pcf8591_ad[1]);
    lcd1602_display_string(sizeof("Smoke:"), 0, display_buf);
  }else if(display_index == DISPLAY_INDEX_CH4){
    sprintf(display_buf, "%bu  ", pcf8591_ad[2]);
    lcd1602_display_string(sizeof("HCHO:"), 0, display_buf);
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 按键查询
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void key_poll(void)
{
  if(is_key_down(0)){///<切换显示
    printf("K0\r\n");
    display_index++;
    if(display_index > DISPLAY_INDEX_CH4){
      display_index = DISPLAY_INDEX_CO;
    }
    lcd1602_display_string(0, 0, "             ");
    display_fix_string();
    display_sensor_value();
    display_warning_string();
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 告警查询
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void warning_poll(void)
{
  bool is_CO_warning = false;
  bool is_SMOKE_warning = false;
  bool is_CH4_warning = false;
  uint8_t len;
  uint8_t len_total;


  if(pcf8591_ad[0] > CO_warning){
    printf("CO_warning\r\n");
    is_CO_warning = true;
  }

  if(pcf8591_ad[1] > SMOKE_warning){
    printf("SMOKE_warning\r\n");
    is_SMOKE_warning = true;
  }

  if(pcf8591_ad[2] > CH4_warning){
    printf("CH4_warning\r\n");
    is_CH4_warning = true;
  }

  if(is_CO_warning || is_SMOKE_warning || is_CH4_warning){
    beep_pin = 0;
    len = 0;
    len_total = 0;
    if(is_CO_warning){
      len = sizeof("CO,") - 1;
      memcpy(&display_buf[len_total], "CO,", len);
      len_total += len;
    }
    if(is_SMOKE_warning){
      len = sizeof("SMOKE,") - 1;
      memcpy(&display_buf[len_total], "SMOKE,", len);
      len_total += len;
    }
    if(is_CH4_warning){
      len = sizeof("HCHO") - 1;
      memcpy(&display_buf[len_total], "HCHO", len);
      len_total += len;
    }
    len = sizeof(" warning\r\n") - 1;
    memcpy(&display_buf[len_total], " warning\r\n", len);
    len_total += len;
    esp8266_send_data(display_buf, len_total);
  }else{
    beep_pin = 1;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 读取按键值
*
*
*@param 
*
*@return 
* 
*
*@note 
*
*/
static void read_sensor_value(void)
{
  adc_pcf8591_read_ad(0, pcf8591_ad, 3);
  
//	  printf("ad1 = %bu\r\n", pcf8591_ad[0]);
//	  printf("ad2 = %bu\r\n", pcf8591_ad[1]);
//	  printf("ad3 = %bu\r\n", pcf8591_ad[2]);
  
 
  display_sensor_value();
  warning_poll();
  time_event_start(get_sensor_value);
}
/*---------------------------------------------------------------------------*/
/**
*@brief 程序入口
*
*
*@param none
*
*@return
* 
*
*@note 程序入口函数main
*
*/
void main(void)
{
  soft_serial_init();
  uart_init();
  lcd1602_init();
  ad_da_pcf8591_init();
  printf("Init...\r\n");
  timer0_mode1_init();
  esp8266_init();
  __enable_irq();
  display_fix_string();
  read_sensor_value();
  display_warning_string();
  while(1){
    esp8266_loop();
    key_poll();
    time_event_loop(get_sensor_value, read_sensor_value, 3000);
  }
}
/*---------------------------------------------------------------------------*/
int32_t ParseNumber(const char* ptr) {
    uint8_t minus = 0, i = 0;
    int32_t sum = 0;
    
    if (*ptr == '-') {                                      /* Check for minus character */
        minus = 1;
        ptr++;
        i++;
    }
    while (CHARISNUM(*ptr)) {                               /* Parse number */
        sum = 10 * sum + CHARTONUM(*ptr);
        ptr++;
        i++;
    }
    
    if (minus) {                                            /* Minus detected */
        return 0 - sum;
    }
    return sum;                                             /* Return number */
}

/*---------------------------------------------------------------------------*/
void remote_data_handle(const uint8_t *buf, uint8_t len)
{
  uint8_t * p = (uint8_t *)buf;
  int32_t num;
  p[len] = ',';
  //soft_serial_transimt(buf, len);
  if(len > 3){
    if(buf[0] == 'G'){
      if(memcmp(buf, "GET CO", sizeof("GET CO") - 1) == 0){
        printf("get CO value\r\n");
        len = sprintf(display_buf, "CO %bu\r\n", pcf8591_ad[0]);
        esp8266_send_data(display_buf, len);
      }else if(memcmp(buf, "GET SM", sizeof("GET SM") - 1) == 0){
        printf("get smoking value\r\n");
        len = sprintf(display_buf, "Smoke: %bu\r\n", pcf8591_ad[1]);
        esp8266_send_data(display_buf, len);
      }else if(memcmp(buf, "GET HC", sizeof("GET HC") - 1) == 0){
        printf("get HCHO value\r\n");
        len = sprintf(display_buf, "HCHO %bu\r\n", pcf8591_ad[2]);
        esp8266_send_data(display_buf, len);
      }
    }else if(buf[0] == 'S'){
      if(memcmp(buf, "SET CO", sizeof("SET CO") - 1) == 0){///<SET CO 123
        printf("set CO value\r\n");
        num = ParseNumber(&buf[7]);
        printf("num = %ld\r\n", num);
        if(num > 0 && num < 250){
          CO_warning = num;
          display_warning_string();
        }
      }else if(memcmp(buf, "SET SM", sizeof("SET SM") - 1) == 0){
        printf("set smoking value\r\n");
        num = ParseNumber(&buf[7]);
        printf("num = %ld\r\n", num);
        if(num > 0 && num < 250){
          SMOKE_warning = num;
          display_warning_string();
        }
      }else if(memcmp(buf, "SET HC", sizeof("SET HC") - 1) == 0){
        printf("set HCHO value\r\n");
        num = ParseNumber(&buf[7]);
        printf("num = %ld\r\n", num);
        if(num > 0 && num < 250){
          CH4_warning = num;
          display_warning_string();
        }
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void timer0_irq(void)
{
  ++clock_ticks;
}
/*---------------------------------------------------------------------------*/


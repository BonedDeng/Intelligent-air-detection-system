/******************************************************************************
文 件 名   : esp8266_ap.c

@file esp8266_ap.c
@brief ESP8266作为AP模式驱动, esp8266作为服务器,由其他模块连接
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef __ESP8266_AP_MODE__

/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/
#if 0
#define log(X)    soft_serial_print_string(X)
#else
#define log(X)
#endif

#define esp8266_gpio_rst_set()    
#define esp8266_gpio_rst_reset()  

#define ESP8266_UART_REC_MAX      ( 128 )///<ESP8266中断接收buf长度,必须是2, 4, 8, 16, 32, 64, 128...
#define ESP8266_LINE_MAX          ( 64 )
#define ESP8266_SEND_MAX          ( 64 )



/*----------------------------------------------*
 * 枚举定义                            *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                              *
 *----------------------------------------------*/
typedef enum{
  ESP8266_CMD_IDEL = 0,
  ESP8266_CMD_INIT,
  ESP8266_CMD_AT,
  ESP8266_CMD_ATE0,
  ESP8266_CMD_SET_AP_MODE,
  ESP8266_CMD_RESTART,
  ESP8266_CMD_WAIT_RESTART,
  ESP8266_CMD_RE_ATE0,
  ESP8266_CMD_CWSAP,///<配置wifi名称和密码
  ESP8266_CMD_CIPMUX,///<开启多连接
  ESP8266_CMD_SET_SERVER_PORT,
  ESP8266_CMD_SERVER_OK,
  ESP8266_CMD_SEND_DATA_START,
  ESP8266_CMD_SEND_DATA,
  ESP8266_CMD_SEND_DATA_OK,
}esp8266_cmd_e;

typedef enum{
  ESP8266_TX_STATE_IDEL = 0,
  ESP8266_TX_STATE_SENDING,
  ESP8266_TX_STATE_WAITING_RESPONS,
  ESP8266_TX_STATE_RESPONS_OK,
  ESP8266_TX_STATE_RESPONS_ERR,
  ESP8266_TX_STATE_RESPONS_TIMEOUT,
  ESP8266_TX_STATE_OVER_TRY_CNT
}esp8266_tx_cmd_state_e;///<esp8266发送命令状态

/*----------------------------------------------*
 * 外部函数原型说明                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                                   *
 *----------------------------------------------*/
static void esp8266_rec_data(const uint8_t *buf, uint8_t len);
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

static struct ringbuf esp8266_uart_rec_ringbuf;///<ES8266串口接收数据环形缓冲区
static clock_time_t esp8266_timeout = 0;///<ESP8266超时软件定时器
static esp8266_cmd_e esp8266_cmd;

static xdata uint8_t esp8266_rec_buf[ESP8266_UART_REC_MAX];///<GSM串口接收buf
static xdata uint8_t esp8266_line_buf[ESP8266_LINE_MAX];///<GSM接收行解析缓冲
static xdata uint8_t esp8266_send_buf[ESP8266_SEND_MAX];
static uint8_t esp8266_line_index;

static uint8_t esp8266_send_data_len = 0;
static esp8266_tx_cmd_state_e esp8266_tx_cmd_state;
static uint8_t esp8266_tx_try_cnt;///<esp8266发送命令尝试次数

time_event_define(esp8266_rec_data);
/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/
static code uint8_t AT_Cmd_Sync[] = "AT\r\n";
static code uint8_t AT_Cmd_ATE0[] = "ATE0\r\n";
static code uint8_t AT_Cmd_SET_AP[] = "AT+CWMODE=2\r\n";///<设置AP模式
static code uint8_t AT_Cmd_RST[] = "AT+RST\r\n";
static code uint8_t AT_Cmd_CWSAP[] = "AT+CWSAP=\"esp8266\",\"12345678\",1,4\r\n";///<配置wifi名称和密码
static code uint8_t AT_Cmd_CIPMUX[] = "AT+CIPMUX=1\r\n";///<开启多连接
static code uint8_t AT_Cmd_SET_SERVER_PORT[] = "AT+CIPSERVER=1,12345\r\n";///<开启服务器, 设置端口号为12345
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266串口发送
*
*
*@param buf: 要发送的数据
*@param len: 要发送的数据长度
*
*@return 
* 
*
*@note 
*
*/
static void esp8266_uart_send_ll(const uint8_t *buf, uint16_t len)
{
  uart_block_send((uint8_t * )buf, len);
}
/*---------------------------------------------------------------------------*/
/**
*@brief esp8266接收中断回调函数
*
*
*@param ch:接收中断输入字符
*
*@return 
* 
*
*@note 
*
*/
void esp8266_uart_rec_irq(uint8_t ch)
{
  ringbuf_put(&esp8266_uart_rec_ringbuf, ch);
}
/*---------------------------------------------------------------------------*/
/**
*@brief esp8266复位
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
static void esp8266_reset(void)
{
  esp8266_gpio_rst_reset();
  HAL_Delay(150);
  esp8266_gpio_rst_set();
  esp8266_timeout = HAL_GetTick();
}
/*---------------------------------------------------------------------------*/
/**
*@brief 字符串转换为数字
*
*
*@param ptr: 字符串
*
*@return 
* 
*
*@note 
*
*/
static int32_t ParseNumber(char* ptr) 
{
	uint8_t minus = 0;
	int32_t sum = 0;
	uint8_t i = 0;
	
	/* Check for minus character */
	if (*ptr == '-') {
		minus = 1;
		ptr++;
		i++;
	}
	
	/* Parse number */
	while (CHARISNUM(*ptr)) {
		sum = 10 * sum + CHAR2NUM(*ptr);
		ptr++;
		i++;
	}
	
	/* Minus detected */
	if (minus) {
		return 0 - sum;
	}
	
	/* Return number */
	return sum;
}
/*---------------------------------------------------------------------------*/
/**
*@brief AT命令处理
*
*
*@param dat : 指向AT命令指针
*@param len : 数据长度
*
*@return 
* 
*
*@note 
*
*/
static void esp8266_at_response_handle(const char *dat, uint8_t len)
{
  const char *ptr = dat;
  uint8_t valid_len = len;
  uint8_t i;

  time_event_stop(esp8266_rec_data);
  
  for(i = 0; i < len; i++){
    if(*ptr == '\r' || *ptr == '\n' || *ptr == ' ' || *ptr == '\t'){
      ptr++;
      valid_len--;
    }else{
      break;
    }
  }

  if(valid_len == 0){
    return;
  }

  if(ptr[0] == 'A' && ptr[1] == 'T'){
    return;
  }

  if(ptr[0] == 'O' && ptr[1] == 'K'){
    if(esp8266_cmd != ESP8266_CMD_SEND_DATA_START){
      if(esp8266_tx_cmd_state == ESP8266_TX_STATE_WAITING_RESPONS){
        esp8266_tx_cmd_state = ESP8266_TX_STATE_RESPONS_OK;
      }
    }
    return;
  }

  if(ptr[0] == '>'){
    if(esp8266_cmd == ESP8266_CMD_SEND_DATA_START){
      esp8266_tx_cmd_state = ESP8266_TX_STATE_RESPONS_OK;
    }
    return;
  }

  if(ptr[0] == '+'){
    if(strncmp(ptr, "+IPD", 4) == 0){///<+IPD,0,3:123
      i = CHAR2NUM(ptr[5]);
      printf("Receive ConnNumber %bu\r\n", i);
      valid_len = ParseNumber(&ptr[7]);
      printf("Receive len %bu\r\n", valid_len);
      i = 0;
		  while (ptr[i] != ':') {
			  i++;
		  }
      i++;
      esp8266_rec_data(&ptr[i], valid_len);
      return;
    }
  }else if(ptr[0] == 'S'){
    if(strncmp(ptr, "SEND OK", 7) == 0){
      if(esp8266_cmd == ESP8266_CMD_SEND_DATA){
        esp8266_tx_cmd_state = ESP8266_TX_STATE_RESPONS_OK;
        return;
      }
    }
  }
  if(valid_len == 11){
    if(strncmp(&ptr[1], ",CONNECT", 8) == 0){
      i = CHAR2NUM(ptr[0]);
      printf("connect ok %bu\r\n", i);
      return;
    }
  }else if(valid_len == 10){
    if(strncmp(&ptr[1], ",CLOSED", 7) == 0){
      i = CHAR2NUM(ptr[0]);
      printf("disconnect ok %bu\r\n", i);
      return;
    }
  }
  printf("Unknow cmd(%bu):%s\r\n",valid_len, ptr);
}
/*---------------------------------------------------------------------------*/
/**
*@brief esp8266收到新数据处理
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
static void esp8266_rec_data_handle(void)
{
  //soft_serial_transimt(esp8266_line_buf, esp8266_line_index);
  esp8266_at_response_handle((const char *)esp8266_line_buf, esp8266_line_index);
  esp8266_line_index = 0;
  memset(esp8266_line_buf, 0, ESP8266_LINE_MAX);
  time_event_stop(esp8266_rec_data);
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266接收数据处理
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
static void esp8266_rec_handle(void)
{
  int c;  
  
  c = ringbuf_get(&esp8266_uart_rec_ringbuf);
  if(c == -1){
    return;
  }

  time_event_start(esp8266_rec_data);
  
  if(esp8266_line_index < ESP8266_LINE_MAX - 1){
    esp8266_line_buf[esp8266_line_index++] = (uint8_t)c;
  }else{
    esp8266_line_buf[esp8266_line_index] = (uint8_t)'\0';
    esp8266_at_response_handle((const char *)esp8266_line_buf, esp8266_line_index);
    esp8266_line_index = 0;
  }
  if(c == 0x0a || c == '>'){    
    esp8266_line_buf[esp8266_line_index] = (uint8_t)'\0';
    esp8266_at_response_handle((const char *)esp8266_line_buf, esp8266_line_index);
    esp8266_line_index = 0;    
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266初始化
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
void esp8266_init(void)
{
  ringbuf_init(&esp8266_uart_rec_ringbuf, esp8266_rec_buf, ESP8266_UART_REC_MAX);
  esp8266_cmd = ESP8266_CMD_INIT;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 复位ESP8266发送状态
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
static void reset_esp8266_tx_state(void)
{
  esp8266_tx_cmd_state = ESP8266_TX_STATE_IDEL;
  esp8266_tx_try_cnt = 0;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 重新开始GSM发送状态
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
static void restart_esp8266_tx(void)
{
  esp8266_tx_cmd_state = ESP8266_TX_STATE_IDEL;
  esp8266_tx_try_cnt++;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266串口发送命令
*
*
*@param buf : 发送命令字符指针
*@param len : 发送命令长度
*@param try_cnt : 重试次数
*@param timeout : 超时时间
*
*@return 
* 
*
*@note 
*
*/
static esp8266_tx_cmd_state_e esp8266_uart_send(const uint8_t *buf, uint16_t len, uint8_t try_cnt, clock_time_t timeout)
{
  switch(esp8266_tx_cmd_state){
    case ESP8266_TX_STATE_IDEL:
      esp8266_tx_cmd_state = ESP8266_TX_STATE_SENDING;
      esp8266_uart_send_ll(buf, len);
      esp8266_timeout = HAL_GetTick();
      esp8266_tx_cmd_state = ESP8266_TX_STATE_WAITING_RESPONS;
      break;
    case ESP8266_TX_STATE_WAITING_RESPONS:
      if((clock_time_t)(HAL_GetTick() - esp8266_timeout) > timeout){
        esp8266_tx_cmd_state = ESP8266_TX_STATE_RESPONS_TIMEOUT;
      }
      break;
    default:
      break;
  }

  if(try_cnt <= esp8266_tx_try_cnt){
    esp8266_tx_cmd_state = ESP8266_TX_STATE_OVER_TRY_CNT;
  }

  return esp8266_tx_cmd_state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266发送状态检查
*
*
*@param state : 当前状态值
*@param next_cmd : 如果命令成功,下一个要执行的命令
*
*@return 
* 
*
*@note 
*
*/
static void esp8266_tx_state_check(esp8266_tx_cmd_state_e state, esp8266_cmd_e next_cmd)
{
  if(state == ESP8266_TX_STATE_RESPONS_OK){
    esp8266_cmd = next_cmd;
    reset_esp8266_tx_state();
  }else if(state == ESP8266_TX_STATE_RESPONS_ERR || state == ESP8266_TX_STATE_RESPONS_TIMEOUT){
    restart_esp8266_tx();
  }else if(state == ESP8266_TX_STATE_OVER_TRY_CNT){
    reset_esp8266_tx_state();
    esp8266_cmd = ESP8266_CMD_INIT;
    
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief esp8266发送数据开始
*
*
*@param try_cnt : 重试次数
*@param timeout : 超时时间
*
*@return static
* 
*
*@note 
*
*/
static esp8266_tx_cmd_state_e esp8266_uart_send_data_len(uint8_t try_cnt, clock_time_t timeout)
{
  uint8_t buf[4];
  uint8_t len;
  
  switch(esp8266_tx_cmd_state){
    case ESP8266_TX_STATE_IDEL:
      esp8266_tx_cmd_state = ESP8266_TX_STATE_SENDING;
      esp8266_uart_send_ll("AT+CIPSEND=0,", sizeof("AT+CIPSEND=0,") - 1);
      len = sprintf(buf, "%bu\r\n", esp8266_send_data_len);
      esp8266_uart_send_ll(buf, len);
      
      esp8266_timeout = HAL_GetTick();
      esp8266_tx_cmd_state = ESP8266_TX_STATE_WAITING_RESPONS;
      break;
    case ESP8266_TX_STATE_WAITING_RESPONS:
      if((clock_time_t)(HAL_GetTick() - esp8266_timeout) > timeout){
        esp8266_tx_cmd_state = ESP8266_TX_STATE_RESPONS_TIMEOUT;
      }
      break;
    default:
      break;
  }

  if(try_cnt <= esp8266_tx_try_cnt){
    esp8266_tx_cmd_state = ESP8266_TX_STATE_OVER_TRY_CNT;
  }
  return esp8266_tx_cmd_state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266发送数据状态检查
*
*
*@param state : 当前状态值
*@param next_cmd : 如果命令成功,下一个要执行的命令
*
*@return 
* 
*
*@note 
*
*/
static void esp8266_tx_data_state_check(esp8266_tx_cmd_state_e state, esp8266_cmd_e next_cmd)
{
  if(state == ESP8266_TX_STATE_RESPONS_OK){
    esp8266_cmd = next_cmd;
    reset_esp8266_tx_state();
  }else if(state == ESP8266_TX_STATE_RESPONS_ERR || state == ESP8266_TX_STATE_RESPONS_TIMEOUT){
    restart_esp8266_tx();
  }else if(state == ESP8266_TX_STATE_OVER_TRY_CNT){
    reset_esp8266_tx_state();
    esp8266_cmd = ESP8266_CMD_IDEL;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief ESP8266处理
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
static void esp8266_handle(void)
{
  uint8_t len;
  esp8266_tx_cmd_state_e state;
  
  switch(esp8266_cmd){
    case ESP8266_CMD_IDEL:
      break;
    case ESP8266_CMD_INIT:
      log("esp8266 reset\r\n");
      esp8266_cmd = ESP8266_CMD_AT;
      esp8266_reset();
      reset_esp8266_tx_state();
      break;
    case ESP8266_CMD_AT:
      len = sizeof(AT_Cmd_Sync) - 1;
      state = esp8266_uart_send(AT_Cmd_Sync, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_ATE0);
      break;
    case ESP8266_CMD_ATE0:
      len = sizeof(AT_Cmd_ATE0) - 1;
      state = esp8266_uart_send(AT_Cmd_ATE0, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_SET_AP_MODE);
      break;
    case ESP8266_CMD_SET_AP_MODE:
      len = sizeof(AT_Cmd_SET_AP) - 1;
      state = esp8266_uart_send(AT_Cmd_SET_AP, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_RESTART);
      break;
    case ESP8266_CMD_RESTART:
      len = sizeof(AT_Cmd_RST) - 1;
      state = esp8266_uart_send(AT_Cmd_RST, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_WAIT_RESTART);
      break;
    case ESP8266_CMD_WAIT_RESTART:
      HAL_Delay(500);///<等待重启
      esp8266_cmd = ESP8266_CMD_RE_ATE0;
      break;
    case ESP8266_CMD_RE_ATE0:
      len = sizeof(AT_Cmd_ATE0) - 1;
      state = esp8266_uart_send(AT_Cmd_ATE0, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_CWSAP);
      break;
    case ESP8266_CMD_CWSAP:
      len = sizeof(AT_Cmd_CWSAP) - 1;
      state = esp8266_uart_send(AT_Cmd_CWSAP, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_CIPMUX);
      break;
    case ESP8266_CMD_CIPMUX:
      len = sizeof(AT_Cmd_CIPMUX) - 1;
      state = esp8266_uart_send(AT_Cmd_CIPMUX, len, 3, 3000);
      esp8266_tx_state_check(state, ESP8266_CMD_SET_SERVER_PORT);
      break;
    case ESP8266_CMD_SET_SERVER_PORT:
      len = sizeof(AT_Cmd_SET_SERVER_PORT) - 1;
      state = esp8266_uart_send(AT_Cmd_SET_SERVER_PORT, len, 3, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_SERVER_OK);
      break;
    case ESP8266_CMD_SERVER_OK:
      log("esp8266 as server ok\r\n");
      esp8266_cmd = ESP8266_CMD_IDEL;
      break;
    case ESP8266_CMD_SEND_DATA_START:
       state = esp8266_uart_send_data_len(2, 1000);
       esp8266_tx_data_state_check(state, ESP8266_CMD_SEND_DATA);
       break;
    case ESP8266_CMD_SEND_DATA:
      state = esp8266_uart_send(esp8266_send_buf, esp8266_send_data_len, 1, 1000);
      esp8266_tx_state_check(state, ESP8266_CMD_SEND_DATA_OK);
      break;
    case ESP8266_CMD_SEND_DATA_OK:
      printf("send data ok\r\n");
      esp8266_cmd = ESP8266_CMD_IDEL;
      break;
    default:
      break;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief 接收到数据
*
*
*@param buf : 数据buf
*@param len : 接收收据长度
*
*@return 
* 
*
*@note 
*
*/
static void esp8266_rec_data(const uint8_t *buf, uint8_t len)
{
  remote_data_handle(buf, len);
  //soft_serial_transimt(buf, len);
}
/*---------------------------------------------------------------------------*/

bool esp8266_send_data(const uint8_t *dat, uint8_t dat_len)
{
  if(esp8266_cmd != ESP8266_CMD_IDEL || dat == NULL){
    return false;
  }

  esp8266_send_data_len = dat_len;
  if(esp8266_send_data_len > ESP8266_SEND_MAX){
    printf("Send data over send buf\r\n");
    esp8266_send_data_len = ESP8266_SEND_MAX;
  }
  memcpy(esp8266_send_buf, dat, esp8266_send_data_len);
  esp8266_cmd = ESP8266_CMD_SEND_DATA_START;
  
  return true;
}

/*---------------------------------------------------------------------------*/
/**
*@brief esp8266任务
*
*
*@param 
*
*@return 
* 
*
*@note 循环调用
*
*/
void esp8266_loop(void)
{
  time_event_loop(esp8266_rec_data, esp8266_rec_data_handle, 600);
  esp8266_rec_handle();
  esp8266_handle();
}
/*---------------------------------------------------------------------------*/
#endif//__ESP8266_AP_MODE__


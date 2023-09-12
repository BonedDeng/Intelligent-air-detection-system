/******************************************************************************
文 件 名   : gsm.c

@file gsm.c
@brief GSM驱动程序
******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "main.h"

#ifdef GSM_DEV_DRIVER
/*----------------------------------------------*
 * 宏定义                                 *
 *----------------------------------------------*/

sbit gsm_reset_pin = P2^0;

#define GSM_UART_REC_MAX      ( 128 )///<GSM中断接收buf长度,必须是2, 4, 8, 16, 32, 64, 128...
#define GSM_LINE_MAX          ( 64 )///<GSM接收最大行长度

#define GSM_UART_SEND_MAX     ( 64 )///<GSM发送buf最大长度
#define GSM_PHONE_NUMBER_LEN  ( 11 )///<GSM电话号码长度

#define gsm_gpio_rst_set()    gsm_reset_pin = 1
#define gsm_gpio_rst_reset()  gsm_reset_pin = 0

#define END                     ( 0x0a )
#define CHARISNUM(x)            ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)            ((x) - '0')
/*----------------------------------------------*
 * 枚举定义                            *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                              *
 *----------------------------------------------*/
typedef enum{
  GSM_SMS_IDEL = 0,///<GSM空闲状态
  GSM_SMS_INIT,///<GSM初始化
  GSM_SMS_WAIT_RESTART,///<等待GSM重启完成
  GSM_CMD_AT,///<同步串口
  GSM_CMD_FACTORY_SETTINGS,///<恢复出厂设置
  GSM_CMD_ATE0,///<关闭回显
  GSM_CMD_PIN,///<查询SIM卡状态
  GSM_CMD_CSQ,///<查询信号质量
  GSM_CMD_CMGF,///<设置短信模式:0 --- PDU, 1 --- txt
  GSM_CMD_CSCS,///<设置短信字符集
  GSM_CMD_CNMI,///<设置有短信时主动提示
  GSM_SMS_REGISTER_SUCCESS,///<GSM初始化成功
  GSM_SMS_START_SEND,///<GSM启动发送
  GSM_SMS_SENDING,///<GSM正在发送
  GSM_SMS_SEND_SUCCESS,///<GSM发送成功
}gsm_cmd_t;

typedef enum{
  GSM_TX_STATE_IDEL = 0,
  GSM_TX_STATE_SENDING,
  GSM_TX_STATE_WAITING_RESPONS,
  GSM_TX_STATE_RESPONS_OK,
  GSM_TX_STATE_RESPONS_ERR,
  GSM_TX_STATE_RESPONS_TIMEOUT,
  GSM_TX_STATE_OVER_TRY_CNT
}gsm_tx_cmd_state_e;

/*----------------------------------------------*
 * 外部函数原型说明                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
static gsm_cmd_t gsm_cmd = GSM_SMS_INIT;///<GSM当前正在执行的命令
static gsm_tx_cmd_state_e gsm_tx_cmd_state = GSM_TX_STATE_IDEL;///<GSM当前串口发送命令状态
static clock_time_t gsm_timeout = 0;///<GSM超时软件定时器
static uint8_t gsm_cmd_try_cnt = 0;///<GSM命令重试次数

static struct ringbuf gsm_uart_rec_ringbuf;///<GSM串口接收数据环形缓冲区

static xdata uint8_t gsm_rec_buf[GSM_UART_REC_MAX];///<GSM串口接收buf
static xdata uint8_t gsm_line_buf[GSM_LINE_MAX];///<GSM接收行解析缓冲
static xdata uint8_t gsm_send_buf[GSM_UART_SEND_MAX];///<GSM发送信息数据buf

static uint8_t gsm_line_index = 0;///<GSM接收数据计数
static idata uint8_t gsm_phone_num_buf[GSM_PHONE_NUMBER_LEN];///<GSM发送信息对方电话号码buf
static uint8_t gsm_send_index = 0;///<GSM发送信息数据长度

time_event_define(gsm_state_check_event);
static bool is_gsm_state_check = false;
/*----------------------------------------------*
 * 常量定义                                       *
 *----------------------------------------------*/
static code uint8_t AT_Cmd_Sync[] = "AT\r\n";
static code uint8_t AT_Cmd_Factory_Set[] = "AT&F\r\n";
static code uint8_t AT_Cmd_ATE0[] = "ATE0\r\n";
static code uint8_t AT_Cmd_Pin[] = "AT+CPIN?\r\n";
static code uint8_t AT_Cmd_CSQ[] = "AT+CSQ\r\n";
static code uint8_t AT_Cmd_CMGF[] = "AT+CMGF=1\r\n";
static code uint8_t AT_Cmd_CSCS[] = "AT+CSCS=\"GSM\"\r\n";
static code uint8_t AT_Cmd_CNMI[] = "AT+CNMI=2,1\r\n";
/*---------------------------------------------------------------------------*/
/**
*@brief GSM串口发送
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
static void gsm_uart_send_ll(const uint8_t *buf, uint16_t len)
{
  uart_block_send((uint8_t * )buf, len);
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM接收中断回调函数
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
void gsm_uart_rec_irq(uint8_t ch)
{
  ringbuf_put(&gsm_uart_rec_ringbuf, ch);
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM复位
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
static void gsm_reset(void)
{
  gsm_gpio_rst_reset();
  HAL_Delay(150);
  gsm_gpio_rst_set();
  gsm_timeout = HAL_GetTick();
  is_gsm_state_check = false;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 复位GSM发送状态
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
static void reset_gsm_tx_state(void)
{
  gsm_tx_cmd_state = GSM_TX_STATE_IDEL;
  gsm_cmd_try_cnt = 0;
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
static void restart_gsm_tx(void)
{
  gsm_tx_cmd_state = GSM_TX_STATE_IDEL;
  gsm_cmd_try_cnt++;
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM串口发送命令
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
static gsm_tx_cmd_state_e gsm_uart_send(const uint8_t *buf, uint16_t len, uint8_t try_cnt, clock_time_t timeout)
{
  switch(gsm_tx_cmd_state){
    case GSM_TX_STATE_IDEL:
      gsm_tx_cmd_state = GSM_TX_STATE_SENDING;
      gsm_uart_send_ll(buf, len);
      gsm_timeout = HAL_GetTick();
      gsm_tx_cmd_state = GSM_TX_STATE_WAITING_RESPONS;
      break;
    case GSM_TX_STATE_WAITING_RESPONS:
      if((clock_time_t)(HAL_GetTick() - gsm_timeout) > timeout){
        gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_TIMEOUT;
      }
      break;
    default:
      break;
  }

  if(try_cnt <= gsm_cmd_try_cnt){
    gsm_tx_cmd_state = GSM_TX_STATE_OVER_TRY_CNT;
  }

  return gsm_tx_cmd_state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM开始发送短信
*
*
*@param number : 电话号码
*@param try_cnt : 重试次数
*@param timeout : 超时时间
*
*@return 
* 
*
*@note 
*
*/
static gsm_tx_cmd_state_e gsm_sms_start_send(const uint8_t *number, uint8_t try_cnt, clock_time_t timeout)
{
  switch(gsm_tx_cmd_state){
    case GSM_TX_STATE_IDEL:
      gsm_tx_cmd_state = GSM_TX_STATE_SENDING;
      gsm_uart_send_ll((const uint8_t *)"AT+CMGS=\"", sizeof("AT+CMGS=\"") - 1);
      gsm_uart_send_ll(number, 11);
      gsm_uart_send_ll((const uint8_t *)"\"\r\n", sizeof("\"\r\n") - 1);
      gsm_timeout = HAL_GetTick();
      gsm_tx_cmd_state = GSM_TX_STATE_WAITING_RESPONS;
      break;
    case GSM_TX_STATE_WAITING_RESPONS:
      if((clock_time_t)(HAL_GetTick() - gsm_timeout) > timeout){
        gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_TIMEOUT;
      }
      break;
    default:
      break;
  }

  if(try_cnt <= gsm_cmd_try_cnt){
    gsm_tx_cmd_state = GSM_TX_STATE_OVER_TRY_CNT;
  }

  return gsm_tx_cmd_state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM发送短信
*
*
*@param dat : 发送短信字符指针
*@param len : 发送长度
*@param try_cnt : 重试次数
*@param timeout : 超时时间
*
*@return 
* 
*
*@note 
*
*/
static gsm_tx_cmd_state_e gsm_sms_sending(const uint8_t * dat, uint8_t len, uint8_t try_cnt, clock_time_t timeout)
{
  uint8_t ch;
  
  switch(gsm_tx_cmd_state){
    case GSM_TX_STATE_IDEL:
      gsm_tx_cmd_state = GSM_TX_STATE_SENDING;
      ch = 0x1a;
      gsm_uart_send_ll(dat, len);
      gsm_uart_send_ll(&ch , 1);
      gsm_timeout = HAL_GetTick();
      gsm_tx_cmd_state = GSM_TX_STATE_WAITING_RESPONS;
      break;
    case GSM_TX_STATE_WAITING_RESPONS:
      if((clock_time_t)(HAL_GetTick() - gsm_timeout) > timeout){
        gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_TIMEOUT;
      }
      break;
    default:
      break;
  }

  if(try_cnt <= gsm_cmd_try_cnt){
    gsm_tx_cmd_state = GSM_TX_STATE_OVER_TRY_CNT;
  }

  return gsm_tx_cmd_state;
}
/*---------------------------------------------------------------------------*/
/**
*@brief 字符串转换为数字
*
*
*@param ptr : 字符串指针
*@param cnt :
*
*@return 
* 
*
*@note 
*
*/
int32_t ParseNumber(const char* ptr, uint8_t* cnt) {
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
  if (cnt != NULL) {                                      /* Save number of characters used for number */
    *cnt = i;
  }
  if (minus) {                                            /* Minus detected */
    return 0 - sum;
  }
  return sum;                                             /* Return number */
}
/*---------------------------------------------------------------------------*/
/**
*@brief 解析信号质量
*
*
*@param str: 信号质量字符串
*
*@return 
* 
*
*@note 
*
*/
static void parse_CSQ(const char *str)
{
  uint8_t temp;
  int8_t rssi = 0;

  temp = ParseNumber(str, NULL);
  if(temp == 99 || temp == 199){
    printf("No signal\r\n");
    if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
      gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_ERR;
    }
  }else if(temp >= 2 && temp <= 30){
    rssi = (temp * 2) - 113;
  }else if(temp == 0){
    rssi = -113;
  }else if(temp == 1){
    rssi = -111;
  }else if(temp == 31){
    rssi = -51;
  }else{
    printf("Unknown rssi:%bu\r\n", temp);
  }
  printf("rssi = %bd\r\n", rssi);
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
static void at_response_handle(const char *dat, uint8_t len)
{
  const char *ptr = dat;
  uint8_t valid_len = len;
  uint8_t i;

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
    if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
      gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_OK;
    }
    return;
  }

  if(ptr[0] == '>'){
    if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
      gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_OK;
    }
    return;
  }

  if(ptr[0] == '+'){
    if(strncmp(ptr, "+CME ERROR:", 11) == 0){
      if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
        gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_ERR;
      }
      return;
    }

    if(strncmp(ptr, "+CMS ERROR:", 11) == 0){///<发送短信失败
      if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
        gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_ERR;
      }
      return;
    }
  }

  if((ptr[0] == 'E') && strncmp(ptr, "ERROR", 5) == 0){
    if(gsm_tx_cmd_state == GSM_TX_STATE_WAITING_RESPONS){
      gsm_tx_cmd_state = GSM_TX_STATE_RESPONS_ERR;
    }
    return;
  }else if((ptr[0] == 'R') && strncmp(ptr, "RING", 4) == 0){
    gsm_uart_send_ll((const uint8_t *)"ATH\r\n", sizeof("ATH\r\n") - 1);
    return;
  }

  if((gsm_cmd == GSM_CMD_CSQ) && (strncmp(ptr, "+CSQ", 4) == 0)){
    parse_CSQ(ptr + 6);
  }else if(gsm_cmd == GSM_SMS_SENDING && (strncmp(ptr, "+CMGS:", 6) == 0)){
    printf("%s\r\n", ptr);
  }else{
    printf("Unknow at cmd:%s\r\n", ptr);
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM接收数据处理
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
static void gsm_rec_handle(void)
{
  int c;

  c = ringbuf_get(&gsm_uart_rec_ringbuf);
  if(c == -1){
    return;
  }

  if(gsm_line_index < GSM_LINE_MAX - 1){
    gsm_line_buf[gsm_line_index++] = (uint8_t)c;
  }else{
    gsm_line_buf[gsm_line_index] = (uint8_t)'\0';
    at_response_handle((const char *)gsm_line_buf, gsm_line_index);
  }
  if(c == END || c == '>'){
    gsm_line_buf[gsm_line_index] = (uint8_t)'\0';
    at_response_handle((const char *)gsm_line_buf, gsm_line_index);
    gsm_line_index = 0;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM初始化
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
void gsm_init(void)
{
  ringbuf_init(&gsm_uart_rec_ringbuf, gsm_rec_buf, GSM_UART_REC_MAX);
  gsm_cmd = GSM_SMS_INIT;
  is_gsm_state_check = false;
  time_event_start(gsm_state_check_event);
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM发送状态检查
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
static void gsm_tx_state_check(gsm_tx_cmd_state_e state, gsm_cmd_t next_cmd)
{
  if(state == GSM_TX_STATE_RESPONS_OK){
    if(is_gsm_state_check &&(gsm_cmd == GSM_CMD_CSQ)){
      is_gsm_state_check = false;
      gsm_cmd = GSM_SMS_IDEL;
    }else{
      gsm_cmd = next_cmd;
    }
    reset_gsm_tx_state();
  }else if(state == GSM_TX_STATE_RESPONS_ERR || state == GSM_TX_STATE_RESPONS_TIMEOUT){
    restart_gsm_tx();
  }else if(state == GSM_TX_STATE_OVER_TRY_CNT){
    gsm_cmd = GSM_SMS_INIT;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM处理
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
static void gsm_handle(void)
{
  gsm_tx_cmd_state_e state;
  uint8_t len;

  switch(gsm_cmd){
    case GSM_SMS_IDEL:
      break;
    case GSM_SMS_INIT:
      printf("gsm reset\r\n");
      gsm_cmd = GSM_SMS_WAIT_RESTART;
      gsm_reset();
      reset_gsm_tx_state();
      break;
    case GSM_SMS_WAIT_RESTART:
      if((clock_time_t)(HAL_GetTick() - gsm_timeout) > 15000){///<等待GSM模块启动
        gsm_cmd = GSM_CMD_AT;
      }
      break;
    case GSM_CMD_AT:
      len = sizeof(AT_Cmd_Sync) - 1;
      state = gsm_uart_send(AT_Cmd_Sync, len, 50, 1000);
      gsm_tx_state_check(state, GSM_CMD_FACTORY_SETTINGS);
      break;
    case GSM_CMD_FACTORY_SETTINGS:
      len = sizeof(AT_Cmd_Factory_Set) - 1;
      state = gsm_uart_send(AT_Cmd_Factory_Set, len, 3, 1000);
      gsm_tx_state_check(state, GSM_CMD_ATE0);
      break;
    case GSM_CMD_ATE0:
      len = sizeof(AT_Cmd_ATE0) - 1;
      state = gsm_uart_send(AT_Cmd_ATE0, len, 3, 1000);
      gsm_tx_state_check(state, GSM_CMD_PIN);
      break;
    case GSM_CMD_PIN:
      len = sizeof(AT_Cmd_Pin) - 1;
      state = gsm_uart_send(AT_Cmd_Pin, len, 3, 3000);
      gsm_tx_state_check(state, GSM_CMD_CSQ);
      break;
    case GSM_CMD_CSQ:
      len = sizeof(AT_Cmd_CSQ) - 1;
      state = gsm_uart_send(AT_Cmd_CSQ, len, 3, 1000);
      gsm_tx_state_check(state, GSM_CMD_CMGF);
      break;
    case GSM_CMD_CMGF:
      len = sizeof(AT_Cmd_CMGF) - 1;
      state = gsm_uart_send(AT_Cmd_CMGF, len, 3, 1000);
      gsm_tx_state_check(state, GSM_CMD_CSCS);
      break;
    case GSM_CMD_CSCS:
      len = sizeof(AT_Cmd_CSCS) - 1;
      state = gsm_uart_send(AT_Cmd_CSCS, len, 3, 1000);
      gsm_tx_state_check(state, GSM_CMD_CNMI);
      break;
    case GSM_CMD_CNMI:
      len = sizeof(AT_Cmd_CNMI) - 1;
      state = gsm_uart_send(AT_Cmd_CNMI, len, 3, 1000);
      gsm_tx_state_check(state, GSM_SMS_REGISTER_SUCCESS);
      break;
    case GSM_SMS_REGISTER_SUCCESS:
      printf("GSM_SMS_REGISTER_SUCCESS\r\n");
      gsm_cmd = GSM_SMS_IDEL;
      break;
    case GSM_SMS_START_SEND:
      state = gsm_sms_start_send(gsm_phone_num_buf, 2, 5000);
      gsm_tx_state_check(state, GSM_SMS_SENDING);
      break;
    case GSM_SMS_SENDING:
      state = gsm_sms_sending((const uint8_t *)gsm_send_buf, gsm_send_index, 2, 30000);
      gsm_tx_state_check(state, GSM_SMS_SEND_SUCCESS);
      break;
    case GSM_SMS_SEND_SUCCESS:
      printf("GSM_SMS_SEND_SUCCESS\r\n");
      gsm_cmd = GSM_SMS_IDEL;
      break;
    default:
      break;
  }
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM状态检查
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
static void gsm_state_check(void)
{
  static uint8_t second = 0;
  
  ++second;
  if(second > 250){///<每250S检查一次
    if(gsm_cmd == GSM_SMS_IDEL){
      gsm_cmd = GSM_CMD_PIN;
      is_gsm_state_check = true;
      second = 0;
    }else{
      second = 250 - 30;///<延时30S检查
    }
  }

  time_event_start(gsm_state_check_event);
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM发送ASCII字符短信
*
*
*@param number : 11位短信号码(字符串)
*@param dat : 发送的数据
*@param dat_len : 发送的数据长度
*
*@return 
* 
*
*@note 
*
*/
bool gsm_sms_send(const uint8_t *number, const uint8_t *dat, uint8_t dat_len)
{
  if(gsm_cmd != GSM_SMS_IDEL || number == NULL || dat == NULL){
    return false;
  }
  memcpy(gsm_phone_num_buf, number, 11);
  if(dat_len > GSM_UART_SEND_MAX){
    printf("Send data over send buf\r\n");
    dat_len = GSM_UART_SEND_MAX;
  }
  memcpy(gsm_send_buf, dat, dat_len);
  gsm_send_index = dat_len;
  gsm_cmd = GSM_SMS_START_SEND;

  return true;
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM是否空闲状态
*
*
*@param 
*
*@return true---空闲 false---忙
* 
*
*@note 
*
*/
bool is_gsm_idel(void)
{
  if(gsm_cmd == GSM_SMS_IDEL){
    return true;
  }

  return false;
}
/*---------------------------------------------------------------------------*/
/**
*@brief GSM任务
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
void gsm_loop(void)
{
  gsm_rec_handle();
  gsm_handle();
  time_event_loop(gsm_state_check_event, gsm_state_check, 1000);
}
/*---------------------------------------------------------------------------*/
#endif//GSM_DEV_DRIVER


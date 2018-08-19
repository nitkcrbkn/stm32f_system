/* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * システムのタスクを記述
 *
 */

#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "SystemTaskManager.h"
#include "MW_USART.h"
#include "MW_I2C.h"
#include "MW_GPIO.h"
#include "DD_Gene.h"
#include "message.h"
#include "app.h"
#include "DD_RC.h" 
#include "MW_IWDG.h"

volatile uint32_t g_SY_system_counter;
volatile uint8_t g_rc_data[RC_DATA_NUM];
static uint8_t rc_rcv[RC_DATA_NUM];
volatile led_mode_t g_led_mode = lmode_1;
static volatile unsigned int count_for_rc = 0;

static
int SY_init(void);
static
int SY_I2CConnTest(int timeout);
static
int SY_doAppTasks(void);
static
int SY_clockInit(void);
static
void SY_GPIOInit(void);


#if !_NO_DEVICE
static
int SY_doDevDriverTasks(void);
#endif

int main(void){
  int ret;

  //システムを初期化します
  ret = SY_init();
  if( ret ){
    message("err", "initialize Faild%d", ret);
    MW_waitForMessageTransitionComplete(100);
    return EXIT_FAILURE;
  }
  //(未実装)I2Cデバイスのチェックをします
  ret = SY_I2CConnTest(10);
  if( ret ){
    message("err", "I2CConnectionTest Faild%d", ret);
    MW_waitForMessageTransitionComplete(100);
    return EXIT_FAILURE;
    }

  //ここから開始します。
  g_SY_system_counter = 0;

  message("msg", "start!!\n");
  MW_printf("\033[2J\033[1;1H");
  flush();

  //アプリケーションを開始するためのループです。
  while( 1 ){
    //ウォッチドックタイマーをリセットします
    MW_IWDGClr();//reset counter of watch dog  

    //個々のアプリケーションの実行をします。
    SY_doAppTasks();
    //もしメッセージを出すタイミングであれば
    if( g_SY_system_counter % _MESSAGE_INTERVAL_MS < _INTERVAL_MS ){
      MW_printf("\033[1;1H");//カーソルを(1,1)にセットして
      DD_RCPrint((uint8_t*)g_rc_data);//RCのハンドラを表示します
      DD_print();//各デバイスハンドラを表示します
      MW_printf("$%d",(int)g_led_mode);//LEDのモードも表示します
      flush(); /* out message. */
    }
    //タイミング待ちを行います
    while( g_SY_system_counter % _INTERVAL_MS != _INTERVAL_MS / 2 - 1 ){
    }
#if !_NO_DEVICE
    //デバイスがあれば、各デバイスタスクを実行します。これはハンドラに格納されているデータをMDに転送する内容などが含まれます。
    ret = SY_doDevDriverTasks();
#endif
    //エラー処理です
    if( ret ){
      message("err", "Device Driver Tasks Faild%d", ret);
      return EXIT_FAILURE;
    }
    //タイミング待ちを行います
    while( g_SY_system_counter % _INTERVAL_MS != 0 ){
    }
    //もし一定時間以上応答がない場合はRCが切断されたとみなし、リセットをかけます。
#if DD_USE_RC
    count_for_rc++;
    if(count_for_rc >= 20){
      message("err","RC disconnected!");
      while(1);
    }
#endif
  }
} /* main */

void SY_wait(int ms){
  volatile uint32_t time;
  time = g_SY_system_counter;
  while(time + ms > g_SY_system_counter)
    MW_IWDGClr();//reset counter of watch dog
  MW_IWDGClr();//reset counter of watch dog
}

static
int SY_doAppTasks(void){
  return appTask();
}

#if !_NO_DEVICE
static
int SY_doDevDriverTasks(void){
  return DD_doTasks();
}
#endif

static
int SY_I2CConnTest(int timeout){
  UNUSED(timeout);
  return EXIT_SUCCESS;
}

static
int SY_init(void){
  int ret;
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
  **/
  if( HAL_Init()){
    return EXIT_FAILURE;
  }

  ret = SY_clockInit();
  /* Configure the system clock */
  if( ret ){
    return EXIT_FAILURE;
  }

  /*UART initialize*/
  MW_USARTInit(USART2ID);

  /*Initialize printf null transit*/
  flush();
#if !_NO_DEVICE
  ret = DD_initialize();
  if(ret){
    return ret;
  }
#endif

  /*Initialize GPIO*/
  SY_GPIOInit();

  appInit();
  
#if DD_USE_RC
  message("msg", "wait for RC connection...");
  if( DD_RCInit((uint8_t*)g_rc_data, 100000) ){
    message("err", "RC initialize faild!\n");
    return EXIT_FAILURE;
  }
  message("msg", "RC connected sucess");
#endif
  
  /*initialize IWDG*/
  message("msg", "IWDG initialize");
  MW_SetIWDGPrescaler(IWDG_PRESCALER_16);//clock 40kHz --> 1/16 -->2500Hz
  MW_SetIWDGReload(250);//Reload volue is 250. reset time(100ms)
  ret = MW_IWDGInit(); 
  if(ret){
    message("err", "IWDG initialize failed!\n");
    return ret;
  }
  
  appInit();

  return EXIT_SUCCESS;
} /* SY_init */

/**
   oscの設定を行います。
 */
static
int SY_clockInit(void){
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if( HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK ){
    message("err", "oscConfigFaild");
    return EXIT_FAILURE;
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if( HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK ){
    message("err", "clockConfigFaild");
    return EXIT_FAILURE;
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if( HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK ){
    message("err", "HAL_RCCEx_PeriphCLKConfig");
    return EXIT_FAILURE;
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  return EXIT_SUCCESS;
} /* SY_clockInit */

static
void SY_GPIOInit(void){
  ENABLECLKGPIOA();
  ENABLECLKGPIOB();
  ENABLECLKGPIOC();
  ENABLECLKGPIOD();

  /*Configure GPIO pin : PC13 */
  MW_SetGPIOPin(GPIO_PIN_13);
  MW_SetGPIOMode(GPIO_MODE_INPUT);
  MW_SetGPIOPull(GPIO_NOPULL);
  MW_GPIOInit(GPIOCID);

  /*Configure GPIO pin : PA5 */
  MW_SetGPIOPin(GPIO_PIN_5);
  MW_SetGPIOMode(GPIO_MODE_OUTPUT_PP);
  MW_SetGPIOSpeed(GPIO_SPEED_FREQ_LOW);
  MW_GPIOInit(GPIOAID);

  /*Configure GPIO pin : PC4 */
  MW_SetGPIOPin(GPIO_PIN_4);
  MW_SetGPIOMode(GPIO_MODE_IT_RISING);
  MW_SetGPIOPull(GPIO_NOPULL);
  MW_GPIOInit(GPIOCID);

  /*Configure GPIO pin Output Level */
  MW_GPIOWrite(GPIOAID, GPIO_PIN_5, 0);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle){
  UNUSED(UartHandle);
  if(DD_RCTask(rc_rcv, (uint8_t*)g_rc_data)!=0)message("err","rc err");
  count_for_rc = 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle){
  UNUSED(UartHandle);
  MW_messageTransitionCompletedCallBack();
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
  UNUSED(hi2c);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
  UNUSED(hi2c);
}

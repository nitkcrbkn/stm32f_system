/* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * 共通部分のコードと他のレイヤーとの通信部分
 *
 * ・I2Cのみのサポート
 */
#include <stdio.h>
#include <stdlib.h>
#include "app.h"
#include "MW_I2C.h"
#include "DD_Gene.h"
#include "DD_MD.h"
#include "DD_AB.h"
#include "DD_SS.h"
#include "DD_ENCODER.h"
#include "message.h"

/*I2Cのサポート用関数*/
int DD_I2C1Send(uint8_t add, const uint8_t *data, uint8_t size){
  int ret = MW_I2C1Transmit(add, data, size);
  if(ret)message("err","trans faild at (%x) size %d,data[0]=%d",add,size,data[0]);
  return ret;
}
int DD_I2C2Send(uint8_t add, const uint8_t *data, uint8_t size){
  int ret = MW_I2C2Transmit(add, data, size);
  if(ret)message("err","trans faild at (%x) size %d,data[0]=%d",add,size,data[0]);
  return ret;
}
int DD_I2C1Receive(uint8_t add, uint8_t *data, uint8_t size){
  return MW_I2C1Receive(add, data, size);
}
int DD_I2C2Receive(uint8_t add, uint8_t *data, uint8_t size){
  return MW_I2C2Receive(add, data, size);
}

/*DeviceDriverのタスク*/
int DD_doTasks(void){
  int i;
  int ret;
#if DD_NUM_OF_MD
  for( i = 0; i < DD_NUM_OF_MD; i++ ){
    ret = DD_send2MD(&g_md_h[i]);
    if( ret ){
      return ret;
    }
  }
#endif
#if DD_NUM_OF_AB
  for( i = 0; i < DD_NUM_OF_AB; i++ ){
    ret = DD_send2AB(&g_ab_h[i]);
    if( ret ){
      return ret;
    }
  }
#endif
#if DD_NUM_OF_SV
  ret = SV_SetRad(&g_sv_h);
  if( ret ){
      return ret;
    }
#endif
#if DD_NUM_OF_SS
  for(i=0; i<DD_NUM_OF_SS; i++){
    ret = DD_receive2SS(&g_ss_h[i]);
    if( ret ){
      return ret;
    }
  }
#endif
#if DD_USE_ENCODER1
  ret = DD_encoder1update();
  if( ret ){
      return ret;
    }
#endif
#if DD_USE_ENCODER2
  ret = DD_encoder2update();
  if( ret ){
      return ret;
    }
#endif

  return EXIT_SUCCESS;
}

/*Deviceのハンドラー表示用関数*/
void DD_print(void){
  int i;
#if DD_NUM_OF_MD
  for( i = 0; i < DD_NUM_OF_MD; i++ ){
    DD_MDHandPrint(&g_md_h[i]);
  }
#endif
#if DD_NUM_OF_AB
  for( i = 0; i < DD_NUM_OF_AB; i++ ){
    DD_ABHandPrint(&g_ab_h[i]);
  }
#endif
#if DD_NUM_OF_SV
  SV_print(&g_sv_h);
#endif
#if DD_NUM_OF_SS
  for(i=0; i<DD_NUM_OF_SS; i++){
    DD_SSHandPrint(&g_ss_h[i]);
  }
#endif
#if DD_USE_ENCODER1 || DD_USE_ENCODER2
  DD_encoderprint();
#endif
}

/*初期化関数*/
int DD_initialize(void){
  int ret;
  
  /* Initialize all configured peripherals */
  MW_SetI2CClockSpeed(I2C1ID, _I2C_SPEED_BPS);
  ret = MW_I2CInit(I2C1ID);
  if( ret ){
    return EXIT_FAILURE;
  }

  /*I2C2を用いたセンサを使用する場合、I2C2のInitを実行*/
#if DD_NUM_OF_SS
  MW_SetI2CClockSpeed(I2C2ID, _I2C_SPEED_BPS);
  ret = MW_I2CInit(I2C2ID);
  if( ret ){
    return EXIT_FAILURE;
  }
#endif
  
#if DD_USE_ENCODER1
  ret = DD_InitEncoder1();
    if(ret)  {
      message("error","Fialed initialize encoder1!");
      return EXIT_FAILURE;
    }
#endif
#if DD_USE_ENCODER2
    ret = DD_InitEncoder2();
    if(ret)  {
      message("error","Fialed initialize encoder2!");
      return EXIT_FAILURE;
    }
#endif

#if DD_NUM_OF_SV
  ret = SV_Init(&g_sv_h);
  if( ret ){
    return EXIT_FAILURE;
  }
#endif
  
  return EXIT_SUCCESS;
}

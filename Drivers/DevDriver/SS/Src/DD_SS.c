/* ===Kisarazu RBKN Library===
 *
 * autor          : Idutsu
 * version        : v0.10
 * last update    : 20180501
 *
 * **overview***
 * SSの通信プロトコルを定める。
 *
 * ・I2Cのみのサポート
 */
#include <stdint.h>
#include <stdbool.h>
#include "message.h"
#include "DD_SS.h"
#include "DD_Gene.h"
#include "app.h"
/*
 *2018年第31回ロボコンでの自動ロボット自立制御の為追加
 *deviceDefinition.cで使用するセンサドライバの数、初期設定を行い
 *使用する
 */

static int ReadPoint  = 0;
static int WritePoint = 0;
static uint8_t RingBuf[RINGSIZE];
static uint8_t request_num;
static bool Change_ReadSensor = true;
static DD_SSCom_Phase phase = TRANSMIT_PHASE;

/*受信及び送信完了コールバックにより繰り返し実行される*/
int DD_receive2SS(void){
  /*I2Cのアドレス、受信データを格納する配列とそのサイズを
   *受信用の関数に引数として渡し、返り値として成功か否かを 
   *返す
   *引数として渡した配列に受信データが格納される
   *引数として渡す配列は、アドレスを渡す
   */
  int ret=0;

#if DD_NUM_OF_SS //app.hでDD_NUM_OF_SSを0にした場合に起きるエラー対策

  if(Change_ReadSensor){ //受信を行うセンサの切り替えフラグが立っている場合
    if(Empty_Check()){
      return 0;
    }else{ //受信要求が空でない場合受信要求を読み込む
      DD_SSPullReceiveRequest(&request_num);
      Change_ReadSensor = false;
    }
  }

  switch(g_ss_h[request_num].type){
  case D_STYP_READ:
    ret = DD_I2C2Receive(g_ss_h[request_num].add, g_ss_h[request_num].receive_data, g_ss_h[request_num].receive_data_size);
    if(ret){
      return ret;
    }
    Change_ReadSensor = true; //受信センサ切り替えフラグを立てる
    break;

  case D_STYP_WRITE_READ:
    if(phase == TRANSMIT_PHASE){      //送信フェーズの場合データ送信を行う
      ret = DD_I2C2Send(g_ss_h[request_num].add, g_ss_h[request_num].send_data, g_ss_h[request_num].send_data_size);
      if(ret){
	return ret;
      }
      phase = RECEIVE_PHASE;  //受信フェーズへ切り替え
    }else if(phase == RECEIVE_PHASE){ //受信フェーズの場合データ受信を行う
      ret = DD_I2C2Receive(g_ss_h[request_num].add, g_ss_h[request_num].receive_data, g_ss_h[request_num].receive_data_size);
      if(ret){
	return ret;
      }
      phase = TRANSMIT_PHASE; //送信フェーズへ戻す
      Change_ReadSensor = true; //受信センサ切り替えフラグを立てる
    }
    break;
  }

#endif

  return ret;
}


/*
 * *SS handlerを表示。
 *
 * SS(Add:hex):[データサイズ],[受信データ]*データサイズ分
 */

void DD_SSHandPrint(DD_SSHand_t *dmd){
  MW_printf("SS(%02x):[%d]", dmd->add,dmd->receive_data_size);
  for(int i=0;i<dmd->receive_data_size;i++){
    MW_printf("[%02x]",dmd->receive_data[i]);
  }
  MW_printf("\n");
}

/*受信要求をリングバッファ(FIFO)に格納する関数*/
int DD_SSPutReceiveRequest(uint8_t num){
  int next = (WritePoint + 1) % RINGSIZE;
  if(next == ReadPoint){ //バッファが埋まった場合
    return -1;
  }
  RingBuf[WritePoint] = num;
  WritePoint = next;
  return 0;
}

/*受信要求をリングバッファ(FIFO)から受け取る関数*/
int DD_SSPullReceiveRequest(uint8_t *num){
  if(ReadPoint != WritePoint){ //バッファが空でない場合
    *num = RingBuf[ReadPoint];
    ReadPoint = (ReadPoint + 1) % RINGSIZE;
  }
  return 0;
}

/*受信要求バッファ(FIFO)が空かを確認する関数*/
int Empty_Check(void){
  if(WritePoint == ReadPoint){
    return -1; //バッファが空の場合
  }else{
    return 0;  //バッファが空でない場合
  }
}

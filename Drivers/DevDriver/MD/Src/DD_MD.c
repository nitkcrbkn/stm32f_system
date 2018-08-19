/* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * MDの通信プロトコルを定める。
 *
 * ・I2Cのみのサポート
 */
#include <stdint.h>
#include "message.h"
#include "DD_MD.h"
#include "DD_Gene.h"

/*
 * ２バイト送信。
 * [0(7:4)|dir(3:2)|duty[9:8](1:0)][duty[7:0](7:0)]
 *
 * dirは0...free,1...forward,2...back,3...brake
 */

int DD_send2MD(DD_MDHand_t *dmd){
  uint8_t data[2];
  const uint8_t sizeof_data = 2;
  uint16_t val;

  /*upto 999*/
  val = dmd->duty / 10;
  if(val >= 1000){
    message("err","[%x]duty over flow(%d)",dmd->add,dmd->duty);
  }
  
  /* Merge Data merge */
  data[0] = val >> 8 |
            (uint8_t)dmd->mode << 2;
  data[1] = val;

  /* Send data */
  return DD_I2C1Send(dmd->add, data, sizeof_data);
}

/*
 * *MD handlerを表示。
 *
 * MD(Add:hex):[Fr,Fw,Bw,Br],[duty:dec]
 */

void DD_MDHandPrint(DD_MDHand_t *dmd){
  MW_printf("MD(%02x):[", dmd->add);
  switch( dmd->mode ){
  case D_MMOD_FREE:
    MW_printf("Fr");
    break;
  case D_MMOD_FORWARD:
    MW_printf("Fw");
    break;
  case D_MMOD_BACKWARD:
    MW_printf("Bw");
    break;
  case D_MMOD_BRAKE:
    MW_printf("Br");
    break;
  }
  MW_printf("],[%4d]\n", dmd->duty);
}


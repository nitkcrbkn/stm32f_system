/* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * ABの通信プロトコルを定める。
 *
 * ・I2Cのみのサポート
 */

#include <stdint.h>
#include "DD_Gene.h"
#include "DD_AB.h"
#include "message.h"

/*
 * [data(7:0)][~data(7:0)]
 *
 * ２バイト送信。dataのそれぞれのビットに各電磁弁のOnOff情報を入れる。
 * 開放、未使用時は0とする。1でOnとなる。
 */
int DD_send2AB(DD_ABHand_t *dab){
  uint8_t data[2];
  const uint8_t sizeof_data = 2;

  /* Merge data */
  data[0] = dab->dat;
  data[1] = ~dab->dat;

  /* Send data */
  return DD_I2C1Send(dab->add, data, sizeof_data);
}

/*
 * 表示内容
 * AB(Add:hex):[data:binary]
 */
void DD_ABHandPrint(DD_ABHand_t *dab){
  MW_printf("AB(%02x):[%08b]", dab->add, dab->dat);
}

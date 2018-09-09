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
#ifndef __DD_GENE_H
#define __DD_GENE_H

#include "DD_MD.h"
#include "DD_AB.h"
#include "DD_SS.h"
#include "DD_SV.h"
#include "DD_ENCODER.h"

#define _I2C_SPEED_BPS 100000

#if DD_NUM_OF_MD
extern DD_MDHand_t g_md_h[DD_NUM_OF_MD];
#endif
#if DD_NUM_OF_AB
extern DD_ABHand_t g_ab_h[DD_NUM_OF_AB];
#endif
#if DD_NUM_OF_SS
extern DD_SSHand_t g_ss_h[DD_NUM_OF_SS];
#endif
#if DD_NUM_OF_SV
extern DD_SV_t g_sv_h;
#endif

/*I2Cのサポート用関数*/
int DD_I2C1Send(uint8_t add,const uint8_t *data,uint8_t size);
int DD_I2C2Send(uint8_t add,const uint8_t *data, uint8_t size);
int DD_I2C1Receive(uint8_t add, uint8_t *data, uint8_t size);
int DD_I2C2Receive(uint8_t add, uint8_t *data, uint8_t size);
/*Deviceのハンドラーの表示用関数*/
void DD_print(void);
/*DeviceDriverのタスク*/
int DD_doTasks(void);
/*初期化関数*/
int DD_initialize(void);


#endif

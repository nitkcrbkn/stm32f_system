/* ===Kisarazu RBKN Library===
 *
 * autor          : Wakabayashi
 * version        : v0.10
 * last update    : 20160821
 *
 * **overview***。
 *
 * サポートデバイス:NXP社PCA9685
 */
#include <stdlib.h>
#include "message.h"
#include "DD_Gene.h"
#include "app.h"
#include "stm32f1xx_hal.h"

static uint8_t ch = 0;

static uint8_t SV_Read8(uint8_t _i2caddr,const uint8_t addr) {
  uint8_t Rdata;
  /* Send register data */
  DD_I2C1Send(_i2caddr, &addr, 1);
  
  /* Receive data */
  DD_I2C1Receive(_i2caddr,&Rdata , 1);

  return Rdata;
}

static int SV_Write8(uint8_t _i2caddr,uint8_t addr, uint8_t d) {
  uint8_t data[2];
  
  /* Set register data */
  data[0] = addr;
  /* Set data */
  data[1] = d;
  /* Send data */
  return DD_I2C1Send(_i2caddr, data, 2);

}

int SV_Init(DD_SV_t *dsv) {

  SV_Write8(dsv->i2cadd,PCA9685_MODE1, 0x0);
  
  uint8_t oldmode = SV_Read8(dsv->i2cadd,PCA9685_MODE1);
  uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep

  SV_Write8(dsv->i2cadd,PCA9685_MODE1, newmode); // go to sleep
  /*setting PWM frequency) 50Hz*/
  SV_Write8(dsv->i2cadd,PCA9685_PRESCALE, 135); //set prescaler (25000000(Hz)/4096*50(Hz)*0.9)-1
  // Correct for overshoot in the frequency setting (see issue #11).
  
  SV_Write8(dsv->i2cadd,PCA9685_MODE1, oldmode);

  HAL_Delay(10);
  SV_Write8(dsv->i2cadd,PCA9685_MODE1, oldmode | 0xa1);  //  This sets the MODE1 register to turn on auto increment.
  return EXIT_SUCCESS;
}

int SV_SetRad(DD_SV_t *dsv) {
  uint8_t data[5];

  if(dsv->val[ch]>4095)
    {
      message("err","servo[%d] is over flow",ch);
      dsv->val[ch] = 4095;
    }
  
  data[0] = LED0_ON_L+(ch<<3);
  data[1] = 0x00;
  data[2] = 0x00;
  data[3] = dsv->val[ch];//off time is always 0.
  data[4] = dsv->val[ch]>>8;

  ch++;
  if(ch==DD_NUM_OF_SV)ch = 0;
  /* Send data */
  return DD_I2C1Send(dsv->i2cadd, data, 5);  
}

int SV_print(DD_SV_t *dsv)
{
  MW_printf("SV[%d](%d)\n",ch, dsv->val[ch]);
  return EXIT_SUCCESS;
}

/*
 * I2C.c
 *
 *  Created on: 2016/06/02
 *      Author: evaota
 */
#include "MW_I2C.h"
#include <stdlib.h>

static I2C_HandleTypeDef hi2c1 = {
  .Instance = I2C1,
  .Init = {
    .ClockSpeed = 100000,
    .DutyCycle = I2C_DUTYCYCLE_2,
    .OwnAddress1 = 0x00,
    .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
    .DualAddressMode = I2C_DUALADDRESS_DISABLE,
    .OwnAddress2 = 0,
    .GeneralCallMode = I2C_GENERALCALL_DISABLE,
    .NoStretchMode = I2C_NOSTRETCH_DISABLE,
  }
};
I2C_HandleTypeDef hi2c2 = {
  .Instance = I2C2,
  .Init = {
    .ClockSpeed = 100000,
    .DutyCycle = I2C_DUTYCYCLE_2,
    .OwnAddress1 = 0x00,
    .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
    .DualAddressMode = I2C_DUALADDRESS_DISABLE,
    .OwnAddress2 = 0,
    .GeneralCallMode = I2C_GENERALCALL_DISABLE,
    .NoStretchMode = I2C_NOSTRETCH_DISABLE,
  }
};

static I2C_HandleTypeDef *i2cid[2] = {
  &hi2c1,
  &hi2c2,
};

void MW_SetI2CClockSpeed(i2cid_t id, uint32_t ClockSpeed){
  assert_param(IS_I2C_ID(id));
  i2cid[(uint32_t)id]->Init.ClockSpeed = ClockSpeed;
}

/* I2C init function */
int MW_I2CInit(i2cid_t id){
  if( HAL_I2C_Init(i2cid[(uint32_t)id]) != HAL_OK ){
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int32_t MW_I2C1Transmit(uint8_t address, const uint8_t *data, uint16_t size){
  if( HAL_I2C_Master_Transmit(&hi2c1, address << 1, (uint8_t*)data, size, 10) != HAL_OK ){
    return -1;
  }
  return 0;
}

int32_t MW_I2C2Transmit(uint8_t address, const uint8_t *data, uint16_t size){
  if(had_completed_tx){ //送信完了フラグが立っていれば送信を行う
    if( HAL_I2C_Master_Transmit_DMA(&hi2c2, address << 1, (uint8_t*)data, size) != HAL_OK ){
      return -1;
    }
    had_completed_tx = false; //送信完了フラグを下げる
  }
  return 0;
}

int32_t MW_I2C1Receive(uint8_t address, uint8_t *data, uint16_t size){
  if( HAL_I2C_Master_Receive(&hi2c1, address << 1, data, size, 10) != HAL_OK ){
    return -1;
  }
  return 0;
}

int32_t MW_I2C2Receive(uint8_t address, uint8_t *data, uint16_t size){
  if(had_completed_rx){ //受信完了フラグが立っていれば受信を行う
    if( HAL_I2C_Master_Receive_DMA(&hi2c2, address << 1, data, size) != HAL_OK ){
      return -1;
    }
    had_completed_rx = false; //受信完了フラグを下げる
  }
  return 0;
}

//送信コールバックによって送信完了フラグを立てる関数
void MW_I2C2TransitionCompletedCallBack(void){
  had_completed_tx = true;
}

//受信コールバックによって受信完了フラグを立てる関数
void MW_I2C2ReceptionCompletedCallBack(void){
  had_completed_rx = true;
}

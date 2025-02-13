#ifndef MP2762A_H
#define MP2762A_H
#include "HAL_ESP.h"


extern HAL_ESP hal;
typedef struct {
    uint16_t uvp; // mV
    uint16_t ovp; // mV
  } MP2762A_config_t;

int bq769x0_reg_update_byte(uint8_t reg, uint8_t mask, uint8_t value);


#endif // MP2762A_H
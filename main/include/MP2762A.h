#ifndef MP2762A_H
#define MP2762A_H
#include "HAL_ESP.h"


extern HAL_ESP hal;
typedef struct {
    uint16_t uvp; // mV
    uint16_t ovp; // mV
  } MP2762A_config_t;

int MP2762A_Configure(MP2762A_config_t config);


#endif // MP2762A_H
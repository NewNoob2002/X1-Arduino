#ifndef MP2762A_H
#define MP2762A_H
#include "HAL_ESP.h"


extern HAL_ESP hal;
typedef struct {
    uint16_t uvp; // mV
    uint16_t ovp; // mV
  } MP2762A_config_t;

class MP2762A {
    public:
        MP2762A();
        ~MP2762A();
        int MP2762_Init(void);
    private:
        MP2762A_config_t config;
        // Private members and methods
};


#endif // MP2762A_H
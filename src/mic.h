
#ifndef MIC_H
#define MIC_H


#include <stdint.h>

void mic_init(int input_pin);
uint32_t mic_read_pin(void);
int mic_detect_event(uint32_t *penergy, int32_t now_ms);
void mic_reset(void);


#endif


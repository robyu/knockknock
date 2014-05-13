
#ifndef SEQ_CORR_H
#define SEQ_CORR_H

#include <stdint.h>

#define SC_NUM_EVENTS_TWO_BITS 4
#define SC_DETECT_THRESHOLD 0.90
void seq_corr_init(void);
int seq_corr_detect(uint32_t *py, int len_y);

#endif


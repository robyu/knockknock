
#ifndef INPUTS_H
#define INPUTS_H

#include "queue.h"



int input_handle_events(event_queue_t *pinput_queue, uint32_t now_ms, int *ptimeout_flag);
int input_convert_abs_to_delta(event_queue_t *pinput_queue, uint32_t *pinput_seq_ms, int max_num_events);


#endif


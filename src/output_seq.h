
#ifndef OUTPUT_SEQ_H
#define OUTPUT_SEQ_H


#include "led.h"
#include "queue.h"

enum outseq_state_t {
    OUTSEQ_INIT,
    OUTSEQ_GET_NEXT_EVENT,
    OUTSEQ_DO_RAMP,
    OUTSEQ_WAIT,
    OUTSEQ_LAST
};

void outseq_init(led_t *ppwm_led, event_queue_t *pqueue);
void outseq_handle_event(uint32_t now_ms);
outseq_state_t outseq_get_state(void);

#endif


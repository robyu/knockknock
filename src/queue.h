
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H



#include <stdint.h>
// for arduino, 
// enums have to declared in their own .h file
// see http://arduino.cc/forum/index.php?topic=109584.0
//
// argh, screw the enum!
//
#define EVENT_NONE             0
//
// input events
#define EVENT_BUTTON_DOWN      1

//
// output events
#define EVENT_WAIT             2
#define EVENT_PWM_RAMP_DOWN    3

#define QUEUE_MAX_LEN 20
struct event_queue_t
{
    int num_events;
    int len_buffer;
    int32_t pbuffer[QUEUE_MAX_LEN];
    uint32_t parg[QUEUE_MAX_LEN];
};

void q_setup(event_queue_t *pqueue);

void q_write(event_queue_t *pqueue, int32_t event, uint32_t arg);

int q_dequeue(event_queue_t *pqueue, int32_t* pevent, uint32_t *parg);

void q_clear(event_queue_t *pqueue);

int q_has_event(event_queue_t *pqueue);

int q_is_full(event_queue_t *pqueue);

#endif


#include "Arduino.h"
#include <stdint.h>
#include "queue.h"
#include <assert.h>
#include "utils.h"

//
// set up input queue
void q_setup(event_queue_t *pqueue)
{
    memset(pqueue, 0, sizeof(event_queue_t));
    pqueue->len_buffer = QUEUE_MAX_LEN;
}

    
    
//
// add input event to queue
void q_write(event_queue_t *pqueue, int32_t event, uint32_t arg)
{
    if (pqueue->num_events < QUEUE_MAX_LEN)
    {
        // not full
        // shift queue by 1
        //
        // 
        // before:  4  3  2  1  x  x  x  x
        //                   ^
        //                   num_events = 4
        //
        // after:   5  4  3  2  1  x  x  x
        //                      ^
        //                      num_events = 5
        memmove(pqueue->pbuffer + 1,
                pqueue->pbuffer,
                pqueue->num_events * sizeof(int32_t));
        memmove(pqueue->parg + 1,
                pqueue->parg,
                pqueue->num_events * sizeof(uint32_t));
        pqueue->pbuffer[0] = event;
        pqueue->parg[0]    = arg;

        // utils_log("q_write:  wrote event=%d\n",pqueue->pbuffer[0]);
        // utils_log("q_write:  wrote arg=%d\n",pqueue->parg[0]);
        pqueue->num_events++;
    }
    // otherwise, queue is full
    
    UTILS_ASSERT((pqueue->num_events>=0) && (pqueue->num_events <= QUEUE_MAX_LEN));

}

//
// read input event
// returns number of events in queue
int q_dequeue(event_queue_t *pqueue, int32_t* pevent, uint32_t *parg)
{
    int has_event = 0;

    if (pqueue->num_events==0)
    {
        // empty queue
        has_event = 0;
        *pevent = EVENT_NONE;
        *parg = 0;
    }
    else
    {
        // return last item in queue
        has_event = 1;
        *pevent = pqueue->pbuffer[pqueue->num_events-1];
        *parg = pqueue->parg[pqueue->num_events-1];

        pqueue->num_events--;
    }
    UTILS_ASSERT((pqueue->num_events>=0) && (pqueue->num_events <= QUEUE_MAX_LEN));
    // utils_log("q_dequeue:  updated num_events=%d\n",pqueue->num_events);
    // utils_log("q_dequeue:  *pevent=%d\n",*pevent);
    // utils_log("q_dequeue:  *parg  =%d\n",*parg);
    return has_event;
}

void q_clear(event_queue_t *pqueue)
{
    pqueue->num_events = 0;
    memset(pqueue->pbuffer, 0, sizeof(int32_t) * QUEUE_MAX_LEN);
    memset(pqueue->parg, 0, sizeof(uint32_t) * QUEUE_MAX_LEN);
}

//
// any events in queue?
int q_has_event(event_queue_t *pqueue)
{
    int has_event = 0;
    has_event = (pqueue->num_events > 0);
    // utils_log("q_has_event:  num_events = %d\n",pqueue->num_events);
    return has_event;
}

int q_is_full(event_queue_t *pqueue)
{
    int is_full;
    is_full = (pqueue->num_events==QUEUE_MAX_LEN);
    return is_full;
}

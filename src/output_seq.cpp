#include "led.h"
#include "queue.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include "output_seq.h"

struct 
{
    outseq_state_t state;

    event_queue_t *pqueue;

    // for pwm
    led_t *ppwm_led;

    // for waiting
    uint32_t wait_thresh_ms;
    uint32_t counter;
} outseq_single;

void outseq_init(led_t *ppwm_led, event_queue_t *pqueue)
{
    memset(&outseq_single, 0, sizeof(outseq_single));
    outseq_single.state = OUTSEQ_INIT;
    outseq_single.pqueue = pqueue;
    outseq_single.ppwm_led = ppwm_led;
}

outseq_state_t outseq_get_state(void)
{
    return outseq_single.state;
}

//
// check queue for events, return next sequencer state and
// event argument
static outseq_state_t get_and_parse_event(event_queue_t *pqueue, uint32_t *parg)
{
    int has_event = 0;
    outseq_state_t next_state = OUTSEQ_GET_NEXT_EVENT;  // default next state

    //UTILS_ASSERT(pqueue != NULL);
    has_event = q_has_event(pqueue);
    //utils_log("get_and_parse:  has_event=%d\n",has_event);
    if (has_event==0)
    {
        // nothing t do
        next_state = OUTSEQ_GET_NEXT_EVENT;
    }
    else
    {
        int32_t event;
        //util_log("get_and_parse:  %d events in queue\n",num_events);
        q_dequeue(pqueue,&event,parg);
        //utils_log("get_and_parse:  dequeued event=%d\n",event);
        //utils_log("                         arg  =%d\n",*parg);
        switch(event)
        {
        case EVENT_WAIT:
            // 
            // set up wait period
            UTILS_ASSERT(*parg > 0);
            next_state = OUTSEQ_WAIT;
            break;
        case EVENT_PWM_RAMP_DOWN:
            // set up pwm ramp
            UTILS_ASSERT(*parg > 0);
            next_state = OUTSEQ_DO_RAMP;
            break;
        default:
            UTILS_ASSERT(0);
        }
    }
    return next_state;
}

//
// countdown wait state
// return next state based on current time
static outseq_state_t update_wait_state(uint32_t now_ms, uint32_t wait_thresh_ms)
{
    outseq_state_t next_state = OUTSEQ_WAIT;
    if (now_ms >= wait_thresh_ms)
    {
        // we're done waiting
        next_state = OUTSEQ_GET_NEXT_EVENT;
    }
    return next_state;
}



//
// output sequencer:  parse and execute each event in the output queue
void outseq_handle_event(uint32_t now_ms)
{
    uint32_t arg;

    outseq_state_t next_state = outseq_single.state;

    outseq_single.counter++;

    //
    // transition logic
    switch(outseq_single.state)
    {
    case OUTSEQ_INIT:
        next_state = OUTSEQ_GET_NEXT_EVENT;
        break;
    case OUTSEQ_GET_NEXT_EVENT:
        next_state = get_and_parse_event(outseq_single.pqueue, &arg);
        break;

    case OUTSEQ_DO_RAMP:
        // immediately transition back to get_next_event
        next_state = OUTSEQ_GET_NEXT_EVENT;
        break;

    case OUTSEQ_WAIT:
        next_state = update_wait_state(now_ms, outseq_single.wait_thresh_ms);
        break;
    default:
        UTILS_ASSERT(0);
    }

    //
    // transition actions
    if (next_state != outseq_single.state)
    {
        switch(outseq_single.state)
        {
        case OUTSEQ_INIT:
            break;

        case OUTSEQ_GET_NEXT_EVENT:
            if (next_state==OUTSEQ_DO_RAMP)
            {
                //util_log("get_next_event -> start_ramp\n");
                led_set_mode(outseq_single.ppwm_led, LED_RAMP_DOWN, (int)arg);
            }
            else if (next_state==OUTSEQ_WAIT)
            {
                //
                // wait time is expressed as delta time in ms
                //util_log("get_next_event -> wait, counter=%d\n",arg);
                outseq_single.wait_thresh_ms = arg + now_ms;
            }
            else
            {
                UTILS_ASSERT(0);
            }
            break;
            
        case OUTSEQ_DO_RAMP:

            UTILS_ASSERT(next_state==OUTSEQ_GET_NEXT_EVENT);
            //util_log("outseq_start_ramp -> get_next_event\n");
            break;

        case OUTSEQ_WAIT:
            UTILS_ASSERT(next_state==OUTSEQ_GET_NEXT_EVENT);
            //util_log("outseq_wait -> get_next_event\n");
            break;

        default:
            UTILS_ASSERT(0);
        }
        outseq_single.state = next_state;
    }
}



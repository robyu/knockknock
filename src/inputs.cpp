#include "knock.h"
#include "debounce.h"
#include "utils.h"
#include "queue.h"
#include "mic.h"
#include <Arduino.h>

// debounce button & detect button transition
// return 0 = no event
//        1 = button clicked
int detect_button_event(int32_t curr_time)
{
    static int prev_state = 1;
    int state = 1;
    int32_t event;  // return value

    event = 0;

    state = Debounce(DIGITAL_BUTTON);
    if (state != prev_state)
    {
        prev_state = state;

        //
        // transition from up to down
        if (state==0)
        {
            //utils_log("button up -> down\n");
            event = 1;
        }
        
        //debounce_print_button_state(state); 
        /* utils_log("\n"); */
    }

    return event;
}

//
// read input queue, convert queue events to delta time
// return number of delta time periods
int input_convert_abs_to_delta(event_queue_t *pinput_queue, uint32_t *pdelta_seq_ms, int max_delta_events)
{
    int32_t event;
    int32_t event2;
    uint32_t event_arg;
    uint32_t event_arg2;
    int n;
    int has_event;

    //
    // clear sequence array
    memset(pdelta_seq_ms, 0, sizeof(uint32_t) * max_delta_events);

    // read first event 
    has_event = q_dequeue(pinput_queue, &event, &event_arg);

    n=0;
    // read queue until it's empty or we've filled vector
    while(has_event && (n<max_delta_events))
    {
        has_event = q_dequeue(pinput_queue, &event2,&event_arg2);

        if (has_event)
        {
            pdelta_seq_ms[n] = (uint32_t)(event_arg2 - event_arg);
            event_arg = event_arg2;
            utils_log("delta_time[%d]=%lu\n",n,pdelta_seq_ms[n]);
            n++;
        }
    }
    return n;
}

//
// handle button press and timeout
int input_handle_events(event_queue_t *pinput_queue, uint32_t now_ms, int *ptimeout_flag)
{
    int button_event;
    int mic_event;
    int32_t delta_time;
    static int32_t time_last_event = 0;
    uint32_t energy;

    // handle inputs
    button_event = detect_button_event(now_ms);
    mic_event = mic_detect_event(&energy, now_ms);
    if (button_event)
    {
        utils_log("button down\n");
        q_write(pinput_queue, EVENT_BUTTON_DOWN, now_ms);
        time_last_event = now_ms;
        *ptimeout_flag = 0;
    }
    else if (mic_event)
    {
        utils_log("mic event | energy = %d\n",energy);
        q_write(pinput_queue, EVENT_BUTTON_DOWN, now_ms);
        time_last_event = now_ms;
        *ptimeout_flag = 0;
    }
    else 
    {
        //
        // timeout occurs after TIMEOUT_THRESH_MS 
        // of inactivity
        uint32_t delta_time;
        delta_time = (uint32_t)now_ms - (uint32_t)time_last_event;
        /* utils_log("delta_time=%lu |",delta_time); */
        /* utils_log("timeout_thresh = %lu\n",(uint32_t)TIMEOUT_THRESH_MS); */
        /* utils_log("delta_time > timeout_thresh = %d\n", delta_time > (uint32_t)TIMEOUT_THRESH_MS); */
        if (delta_time > (uint32_t)TIMEOUT_THRESH_MS)
        {
            utils_log("timeout | energy = %d\n",energy);
            *ptimeout_flag = 1;
            time_last_event = now_ms;
        }
    }

    return (int)(button_event+mic_event);
}

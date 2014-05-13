// -*- mode: c; -*-

#include "TimerOne.h"
#include "output_seq.h"
#include "utils.h"
#include "inputs.h"
#include "debounce.h"
#include "queue.h"
#include "knock.h"
#include "led.h"
#include "seq_corr.h"
#include "mic.h" 
#include "dc_filter.h"

struct
{
    led_t pwm_out;
    led_t pwm_input_confirm;
    event_queue_t input_queue;
    event_queue_t output_queue;
    int timeout_flag;   // no input event for a long time
    int interrupt_flag;  // timer interrupt
    uint32_t interrupt_counter;
    int handle_input_flag;
    int handle_output_flag;
} single;

//
// /Applications/Arduino.app/Contents/Resources/Java/hardware/arduino/cores/arduino
//
// /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/avr/include/stdint.hw
//

//   1  Hz: 1000000 usec
// 2000 Hz:     500 usec
#define TIMER1_PERIOD_USEC  500
//#define TIMER1_PERIOD_USEC    1000000

void setup() 
{
    // initialize serial communication:
    // make it slow to minimize stepping on interrupt
    Serial.begin(9600); 
    pinMode(DIGITAL_BUTTON, INPUT);
    pinMode(INT_LED_DIGITAL, OUTPUT);  
    //pinMode(INPUT_CONFIRM_LED, OUTPUT);  

    q_setup(&single.input_queue);
    q_setup(&single.output_queue);
    
    single.timeout_flag = 0;
    single.interrupt_counter = 0;
    outseq_init(&single.pwm_out, &single.output_queue);
    
    seq_corr_init();
    Timer1.initialize(TIMER1_PERIOD_USEC);  
    Timer1.attachInterrupt(timer1_isr);
    mic_init(ANALOG_IN);  // 2013-10-17

    // MUST call led_init AFTER Timer1.initialize
    led_init(&single.pwm_out, PWM_LED, 10);   // 10 is ramp increment
    led_init(&single.pwm_input_confirm, INPUT_CONFIRM_LED, -1);

    // IM ALIVE: tell led to ramp down
    q_write(&single.output_queue, EVENT_PWM_RAMP_DOWN, 1024);
    q_write(&single.output_queue, EVENT_WAIT, 2000);

    led_set_mode(&single.pwm_input_confirm, LED_RAMP_DOWN, 1024);

    utils_log("exit setup\n");
} 


//
// periodically do interrupt stuff
void timer1_isr(void)
{
    static int led_state = LOW;
    single.interrupt_counter++;

    // time for input processing?
    // 2013-10-22  use counter = 100 for input cycle
    if ((single.interrupt_counter % 50)==0)
    {
        single.handle_input_flag = 1;

        // flip LED state
        led_state = (led_state==HIGH) ? LOW : HIGH;
        digitalWrite(INT_LED_DIGITAL, led_state);
    }

    // time for output processing?
    if (((single.interrupt_counter+25) % 50)==0)
    {
        single.handle_output_flag = 1;
    }


    // update pwm leds
    led_update(&single.pwm_out);
    led_update(&single.pwm_input_confirm);

    mic_read_pin();
}

//
// do actions associated with timeout period
void do_timeout_actions(event_queue_t *pinput_queue, event_queue_t *poutput_queue)
{
    int has_event;
    uint32_t pdelta_seq_ms[QUEUE_MAX_LEN-1];  

    has_event = q_has_event(pinput_queue);
    if (has_event==0)
    {
        //
        // no events since timeout
    }
    else
    {
        int seq_flag;
        int num_delta_periods;
        num_delta_periods = input_convert_abs_to_delta(pinput_queue, pdelta_seq_ms, QUEUE_MAX_LEN-1);
        
        if (num_delta_periods==0)
        {
            // a single mic/button event -> dont do anything
        }
        else
        {
            seq_flag = seq_corr_detect(pdelta_seq_ms, SC_NUM_EVENTS_TWO_BITS);

            if (seq_flag==1)
            {
                // detected knock sequence, so response with 
                // "two bits"
                q_write(poutput_queue, EVENT_PWM_RAMP_DOWN, 1024);
                q_write(poutput_queue, EVENT_WAIT, 400);
                q_write(poutput_queue, EVENT_PWM_RAMP_DOWN, 1024);
                q_write(poutput_queue, EVENT_WAIT, 1000);   // final 1 sec
            }
            else 
            {
                // sequence not detected, so playback input sequence
                int m;
                for(m=0;m<num_delta_periods;m++)
                {
                    q_write(poutput_queue, EVENT_PWM_RAMP_DOWN, 1024);
                    q_write(poutput_queue, EVENT_WAIT, pdelta_seq_ms[m]);
                }
                q_write(poutput_queue, EVENT_PWM_RAMP_DOWN, 1024);
                q_write(poutput_queue, EVENT_WAIT, 1000);   // final 1 sec
            }
        }
    }

    // reset 
    q_clear(pinput_queue);
    mic_reset(); // clear mic energy sum
}

void test_dc_filter(void)
{
    int16_t px[50];
    int16_t py[50];
    dc_filter_t filter;
    int n;
    
    dc_filter_init(&filter, 
                   300.0f,   // fcut
                   2000.0f); // sample rate
    memset(py, 0, sizeof(py));
    for (n=0;n<25;n++)
    {
        px[n]    = 1024;
        px[n+24] = -1024;
    }
    dc_filter(&filter, px, 50, py);
    for (n=0;n<50;n++)
    {
        utils_log("y[%d]=",n);
        utils_log("%d\n",py[n]);
    }
}

//
// main loop
void loop() 
{
    int32_t event;
    uint32_t event_arg;
    static uint32_t loop_counter=0;
    //static int led_state = HIGH;
    uint32_t now_ms;

    /* test_dc_filter(); */
    /* assert(0); */

    loop_counter++;
    now_ms = millis();

    //utils_log("main_loop\n");
    //
    // handle input_flag from ISR
    if (single.handle_input_flag)
    {

        // accept input event IFF output queue is currently empty
        // this is to prevent solenoid-mic feedback loop
        if (q_has_event(&single.output_queue)==0)
        {
            int event_flag;
            //utils_log("handle input\n");
            event_flag = input_handle_events(&single.input_queue, now_ms, &single.timeout_flag);
            single.handle_input_flag = 0;

            if (event_flag)
            {
                // acknowledge input
                led_set_mode(&single.pwm_input_confirm, LED_RAMP_DOWN, 1024);
            }
        }

        // clear input flag and mic
        single.handle_input_flag = 0;
        mic_reset();
    }

    //
    // handle output_flag from ISR
    if (single.handle_output_flag)
    {
        //utils_log("handle output\n");
        outseq_handle_event(now_ms);
        single.handle_output_flag = 0;
    }

    //
    // timeout actions:  process input sequence
    // timeout flag is set by input processing
    if (single.timeout_flag)
    {
        //
        // timeout handling takes a while, so stop ISR to halt microphone reads
        Timer1.stop();
        utils_log("--------------- timeout\n");
        do_timeout_actions(&single.input_queue,
                           &single.output_queue);
        single.timeout_flag = 0;
        Timer1.resume();
    }
}

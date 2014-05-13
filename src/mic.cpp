// -*- mode: c; -*-
// Testing interrupt-based analog reading
// ATMega328p

// Note, many macro values are defined in <avr/io.h> and
// <avr/interrupts.h>, which are included automatically by
// the Arduino interface

#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "Arduino.h"
#include "dc_filter.h"
#include "mic.h"
#include "utils.h"

//#define EVENT_THRESH_MS  250
// reduced time threshold to improve knock resolution
#define EVENT_THRESH_MS  125

struct
{
    dc_filter_t dc_filter;
    uint32_t sum;
    uint32_t detect_threshold;
    int input_pin;
    int curr_state;             
    int32_t last_event_ms;
} mic_single;


void mic_init(int input_pin)
{
    memset(&mic_single, 0, sizeof(mic_single));
    dc_filter_init(&mic_single.dc_filter, 
                   300.0f, //fcut_hz
                   2000.0f); // sample_rate_hz
    mic_single.input_pin = input_pin;
    //mic_single.detect_threshold = 1500;
    mic_single.detect_threshold = 200;
}


//
// read microphone, do some signal processing
uint32_t mic_read_pin(void)
{
    int16_t samp;
    int16_t filtered_samp;

    samp = (int16_t)analogRead(mic_single.input_pin);
    dc_filter(&mic_single.dc_filter, &samp, 1, &filtered_samp);
    //utils_log("filtered_samp = %d\n",filtered_samp);

    // abs
    filtered_samp = (filtered_samp < 0) ? -filtered_samp : filtered_samp;
    mic_single.sum += filtered_samp;
    return mic_single.sum;
}

//
// reset energy sum and state
void mic_reset(void)
{
    mic_single.sum = 0;
    mic_single.curr_state = 0;
}

//
// knock detected?
// a clap or snap lasts 250 - 300 msec; 1/300 msec = 3.33 Hz
// so integration should occur at half this rate, e.g. 150 msec -> 6.66 Hz
// 
// assume that mic is being samples at Fs=2000 Hz
// then we should run mic event detector every k samples, where k is
//   Fs = k * 6.66 Hz
//   2000 Hz = k * 6.66 Hz
//   k = 300 samples  (2013-10-22:   k = 100 samples)
//
int mic_detect_event(uint32_t *penergy, int32_t now_ms)
{
    int reading;
    int curr_sum = mic_single.sum;
    
    //utils_log("mic_detect_event: curr_sum==%d\n",curr_sum);
    reading = (curr_sum >= mic_single.detect_threshold);
    // reset sum
    mic_single.sum = 0;    

    if ((reading==1) &&                       // sum exceeds threshold
        (now_ms > mic_single.last_event_ms + EVENT_THRESH_MS))  // and last event was a while ago
    {
        utils_log("mic_detect_event: positive event==%d\n",curr_sum);
        mic_single.curr_state = 1;
        mic_single.last_event_ms = now_ms;
    }
    else
    {
        mic_single.curr_state = 0;
    }

    if (penergy!=NULL) *penergy = curr_sum;


    return mic_single.curr_state;
}




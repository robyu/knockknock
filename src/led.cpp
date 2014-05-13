#include "Arduino.h"
#include "led.h"
#include "utils.h"
#include "TimerOne.h"
#include <assert.h>

// initialize led struc
// incr is increment for ramp
// specify negative number for default
void led_init(led_t *p, int analog_pin, int incr)
{
    memset(p, 0, sizeof(led_t));
    p->analog_pin = analog_pin;
    p->mode = LED_STEADY;
    p->pwm = 0;

    UTILS_ASSERT(incr < 100);  // sanity check for increment
    if (incr<=0)
    {
        p->incr = 2;
    }
    else
    {
        p->incr = incr;
    }

    UTILS_ASSERT((analog_pin==9) || (analog_pin==10));  // for timer1, must be pin 9 or 10
    //
    // initialize LED:  set to hi
    // use Timer1.pwm() at least once to configure the pin,
    // later use Timer1.setPwmDuty()
    // don't need to initialize timer1 with period
    // don't need timer1 ISR
    // output seq should only set led mode & initial pwm value
    Timer1.pwm(p->analog_pin, 1023); // set LED to hi
}

//
// set mode:  ramp up, ramp down, ramp up and down
void led_set_mode(led_t *p, led_mode_t mode, int pwm_val)
{
    UTILS_ASSERT(pwm_val >= 0);
    UTILS_ASSERT(pwm_val <= 1024);
    p->mode = mode;
    p->pwm = pwm_val;
}

// update led state, write to pin
// in order to achieve smooth ramp, you must call this function
// at a sufficiently SLOW rate
//
// e.g. 
// main loop / 10   : fade in 1 s
// or ISR @ 2000 Hz : fade in 0.5 s
int led_update(led_t *p)
{
    int pwm;

    switch(p->mode)
    {
    case LED_RAMP_DOWN:
        p->pwm -= p->incr;
        if (p->pwm <= 0) p->pwm = 0;
        pwm = p->pwm;
        break;

    case LED_RAMP_UP:
        p->pwm += p->incr;
        if (p->pwm >= 1023) p->pwm = 1023;
        pwm = p->pwm;
        break;

    case LED_STEADY:
        pwm = p->pwm;
        break;

    default:
        UTILS_ASSERT(0);
    }
    //utils_log("led_update pwm=%d\n",p->pwm);
    
    Timer1.setPwmDuty(p->analog_pin, pwm);
    return p->pwm;
}

int led_get_pwm_val(led_t *p)
{
    return p->pwm;
}

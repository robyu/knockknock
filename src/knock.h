
#ifndef KNOCK_H
#define KNOCK_H


#define DIGITAL_BUTTON   2    // digital input, connect button to D2
#define PWM_LED          9    // pwm output (led & solenoid)
#define INT_LED_DIGITAL  12   // interrupt indicator (iack)
#define INPUT_CONFIRM_LED 10  // confirm input event, also a pwm

#define ANALOG_IN        0    // mic input
//
// timeout period for "no recent events"
#define TIMEOUT_THRESH_MS 1500  // max seems to be 32000

#define MY_MIN(a,b) (a > b) ? b : a
#define INT_ABS(x) (x > 0) ? (x) : (-x)



#endif // knock_h



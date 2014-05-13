#ifndef LED_H_
#define LED_H_


enum led_mode_t
{
    LED_RAMP_UP,
    LED_RAMP_DOWN,
    LED_STEADY,
    LED_NOT_USED
};

struct led_t
{
    int analog_pin;
    int pwm;
    led_mode_t mode;
    int incr;
};

void led_init(led_t *p, int analog_pin, int incr);

void led_set_mode(led_t *p, led_mode_t mode, int pwm_val);

int led_update(led_t *p);

int led_get_pwm_val(led_t *p);
#endif



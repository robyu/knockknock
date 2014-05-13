#include "Arduino.h"
#include "utils.h"
// 
// gotta hold the button down to register a press
// returns button state:  LOW or HIGH
//
// test:  wire JC1 to input pin
// on protoshield, button goes LOW when pressed
int Debounce(int buttonPin)
{
    static int buttonState = HIGH;   // current reading
    static int lastButtonState = HIGH;
    static long lastDebounceTime = 0;
    long debounceDelay = 50;
    int reading = digitalRead(buttonPin);

    // check to see if you just pressed the button 
    // (i.e. the input went from LOW to HIGH),  and you've waited 
    // long enough since the last press to ignore any noise:  

    /* utils_log("entering debounce: lastButtonStat="); */
    /* print_button_state(lastButtonState); */
    /* utils_log("| buttonState="); */
    /* print_button_state(buttonState); */
    /* utils_log("\n"); */

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
        // reset the debouncing timer
        /*
        utils_log("bounce\n");
        */
        lastDebounceTime = millis();
    } 
  
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer
        // than the debounce delay, so take it as the actual current state:

        // if the button state has changed:
        if (reading != buttonState) {
            /*
            utils_log("state change\n");
            */
            buttonState = reading;
        }
    }
    lastButtonState = reading;
    return buttonState;
}

void debounce_print_button_state(int state)
{
    if (HIGH==state)
    {
        utils_log("high");
    }
    else
    {
        utils_log("low");
    }
}

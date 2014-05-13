

Knock*Knock
-----------
Knock Knock is an Arduino-powered, acoustic impulse sequence detector.  That's a fancy way of saying that it's a little embedded project which listens and responds to sequences of sharp sounds, like claps.  

The software is developed for an Arduino Uno with [Wiring][1].  It assumes additional hardware:  a microphone, current driver, and solenoid.  For a complete description of the software and hardware, see [Knock Knock Description][2].

Installation and Building
=========================
Download the files into your Arduino development directory.  Open the project file, `knock.ino`, in the Arduino IDE.

Knock Knock depends on the [TimerOne][3] library, which presents a simplified interface to the Arduino's periodic timer.  

On my system, the Arduino directory looks like this:

    ~/Arduino
        /knock
        /libraries
            /TimerOne

Files
=====
See the [Knock*Knock webpage][2] for a description of how everything hangs together.

**dc_filter.cpp** - fixed-point low pass IIR filter
**debounce.cpp** - debounce routine for test button
**inputs.cpp** - handle all inputs (button, microphone); write input events to event queue
**knock.ino** - main file; Arduino setup() and loop(); interrupt service routine
**led.cpp** - LED interface for smoothly ramping output via PWM
**output_seq.cpp** - output event queue and event handler
**queue.cpp** - event queue
**seq_corr.cpp** - compute correlation coefficient between known sequence and input sequence
**utils.cpp** - some utils for printing to the console

License
=======
MIT License


----------


  [1]: http://wiring.org.co/ Wiring
  [2]: http://robertyu.com/wikiperdido/Knock%20Knock Knock*Knock Webpage
  [3]: http://playground.arduino.cc/Code/Timer1 TimerOne
  

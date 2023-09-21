# Knock*Knock

Knock Knock is a little embedded project which listens for a sequence of claps or knocks and parrots the sequence by knocking against its enclosure.

### The Guts

The guts of Knock Knock consists of an Arduino Uno, a microphone preamp board from Sparkfun, a Sparkfun "protoshield," and a power supply.

![Knock Knock](/Users/ryu/Documents/p2023/knockknock/readme_images/small_arduino_uno_unpacked.jpg "KnockKnock Open Box")

### The Ins

Knock Knock listens to a microphone. The electret microphone board I purchased from Sparkfun included an amplifier, so I merely needed to supply a 5V bias voltage and ground, then read the voltage wiggles off the signal line.

To test the microphone, I powered it with a battery, then clipped a 3.5mm headphone jack to the signal line and plugged it into my laptop. I fretted that I my laptop would puke over the input signal's DC bias. Just stick it in the hole, my friend recommended; that's what I did, and it worked fine.

Here's where the Arduino shines: both 5V and ground are supplied by the Arduino, as well as analog-to-digital conversion. Wiring an amplified microphone to the Arduino required no extra components. The Arduino software framework includes commands for reading A-to-D values.

I also wired up a **Big Happy Button** as an alternate input for testing the software.

### Sampling

The Arduino includes an A-D (!), so sampling a signal is fairly easy. The Arduino expects the microphone input to have a reference voltage bias of 2.5 V. The A-to-D will measure voltage inputs of 0..5 V with 10-bit accuracy--it maps readings of 0..5 V to \[0..1024\], where 512 represents 2.5 V. If the bias voltage is a little off, then the samples are a little off as well, and you get a DC bias in your readings.

 If I were concerned about making accurate signal measurements, I'd have to add analog circuitry to adjust the bias voltage to exactly 2.5 V. Fortunately, Knock\*Knock is merely looking for impulses, so signal fidelity isn't important. I merely needed to re-map the A-to-D readings from 0..1024 to -512..-512 before applying my detection
algorithm. 

I eliminated the DC bias in software with a high-pass IIR:

 *y\[n\] = alpha \* (y\[n-1) + x\[n\] - x\[n-1\])*

Couldn't I have merely attached the microphone to a digital input pin and avoided all that sampling and filtering stuff? My friend mentioned that after I was done. Maybe next time.

### The Outs

The "knocks" are provided by a solenoid. I played around with a solenoid from a broken lawn irrigation valve, but eventually purchased a beefier solenoid. As I planned to modulate the knocks with the Arduino's PWM, I couldn't activate the solenoid with a simple on-off relay. Luckily, my friend at work just happened to have some [TI SN74410 Quad Half-H Drivers](https://www.ti.com/lit/ds/symlink/sn754410.pdf), which are specifically designed to provide current for motors and solenoids. 

Here's my solenoid schematic:

![](/Users/ryu/Documents/p2023/knockknock/readme_images/small_solenoid_driver.jpg)

What's with the diode in parallel with the solenoid? A solenoid is basically an inductor. When you suddenly cut the current to an inductor, the inductor's collapsing magnetic field draws additional current from whatever is connected. The diode is a [flyback diode](http://en.wikipedia.org/wiki/Flyback_diode), which protects the rest of the circuit from the resulting voltage spike.

Note: I never got around to modulating the clacks.

![](/Users/ryu/Documents/p2023/knockknock/readme_images/small_arduino_wiring.jpg)

### The Firmware ISR

The Arduino environment has a nice library ([Timer1]([Arduino Playground - Timer1](https://playground.arduino.cc/Code/Timer1/))) for configuring a timer-based ISR.

1. The on-chip timer invokes the ISR every 500 usec. 
2. Every M interrupts, the ISR sets a `handle_input` flag telling the main loop to process the inputs. 
3. Every M interrupts, the ISR sets a `handle_output` flag telling the main loop to process the outputs. 
4. Every interrupt, the ISR updates the PWM output pins, which are hooked up to an LED (input confirmation, or IACK) and the solenoid. Using the PWM output with the LEDs let me fade the lights smoothly. 
5. Finally, the ISR processes the microphone input. Processing the microphone involves reading the digitized value off the input pin, applying an IIR high-pass filter (to remove DC), then adding the absolute value of the filtered value to an accumulator. The ISR runs every 500 usec, so the microphone sample rate is 2KHz. 



### Main Loop

Outside the ISR, the main loop checks if the handle\_input flag was set by the ISR. If so, then the software applies heuristics to check the microphone sample accumulator (a simple threshold) or button (debounced) for an input event; if an event occurred, then the software adds the event to an input queue. If no event has occurred for a longish time--say one second--then it sets the timeout\_flag. 

When the timeout\_flag is set, the sequencer tries to match the contents of the input queue against a known sequence, namely the first five beats of "shave and a haircut...two bits."" It does this by converting the input queue into a sequence of delta time periods, i.e. milliseconds between input events, and computing the correlation coefficient against the known sequence. In order to speed things up, I precomputed the statistics (mean, variance) of the a-priori sequence during initialization. If the correlator recognizes the sequence, then the sequencer responds by writing the a command sequence to the output queue (e.g. "two bits"" = RAMP\_SOLENOID, WAIT, RAMP\_SOLENOID). Otherwise, the sequencer writes the input sequence to the output queue (so Knock Knock tries to mimic the timing of its input).

Finally, if the handle\_output flag was set by the ISR, then the software updates its output state machine.The state machine reads commands from an output queue, e.g. RAMP\_SOLENOID or WAIT. It performs the command, then processes the next command or goes quiet if the queue is empty.

![](/Users/ryu/Documents/p2023/knockknock/readme_images/knock_dfd.png)

### Installation and Building

Download the files into your Arduino development directory.  Open the project file, `knock.ino`, in the Arduino IDE.

Knock Knock depends on the [TimerOne](http://playground.arduino.cc/Code/Timer1) library, which presents a simplified interface to the Arduino's periodic timer.  

On my system, the Arduino directory looks like this:

    ~/Arduino
        /knock
        /libraries
            /TimerOne



### Putting It All Together

After getting the whole thing working on a breadboard, I soldered everything onto a protoboard, added a few panel LEDs, and shoved the whole thing into a cigar box. I freely admit that the wiring looks like I did it while drunk and wearing boxing gloves. I even used hot-pink duct tape for added effect.

![](/Users/ryu/Documents/p2023/knockknock/readme_images/small_open_box.jpg)



![](/Users/ryu/Documents/p2023/knockknock/readme_images/small_closed_box.jpg)



If you've read this far, you may as well check out the video:

[knock knock - YouTube](https://www.youtube.com/watch?v=kqF6x-jQpuQ)



License
=======

MIT License

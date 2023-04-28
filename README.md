# Granula - A granular synth aux cereales avec du chocolat dessus.

Granula is a synth based on Arduino Due.
This is still a very early WIP:

  - hardware not fully defined yet (knobs, buttons, midi in)
  - the main target is a granular but in order to start "small" in keep the fun, for now it's a monophonic synth
  - needs menus, etc.

## Hardware platform

The code is targeted ton run on Arduino Due, but it sould be able to run on many other platforms if you:

  - change the timers implementation (currently using DueTimer library)
  - change the buttons/knobs capture


## Frequency, buffer sizes and sample rate

We need to do some maths here.

The sample rate is the frequency at which we will update our 8bits analog output.
Thus we will use a timer and an interrupt in order to be called x times per second (ex. 22050 times per sec. when a sample rate of 22050 is set).

Now if we play a 100Hz sine wave, it means that this signal will be heard 100 times in 1s, thus our 22050 samples will contains this scheme 100 times.
At the end of the day, our 100Hz signal will be mapped onto 22050/100 values.

Thus, we will need a buffer of size (sample_rate / frequency) to store our signal and loop on it.

Now, we will use interrupts and timers in order to properly reach a given sample rate. Since we're using an 8bits DAC, trying to reach 9kHz for Hi res audio is pointless. Basically, anything around 20kHz would be a good start (we clearly won't have subtle high frequencies anyway).

The CPU is running at 16MHz and will increase a counter which can be 16bits (timer 0) or 8bits (timers 1 and 2). A "prescaler" can be used to skip increments (1,8,64,256,1024) and we can also define the value to reach before starting again at 0.
By using a prescaler set to 8 and a maximum value set to 90, or "sample rate" (ie interrupt frequency) will be : 16MHz/(8*(90+1)) = 21978,02

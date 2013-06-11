SNOOZER

![Alt text](https://raw.github.com/moetunes/images/master/others/snoozer.png)

An alarm with a snooze button.

Usage: snoozer _hour_ _minute_

       snoozer 4 30

	   snoozer 16 30

Green button turns red when the alarm sounds

Click the red button to kill the alarm

Click the blue button to sleep a bit more

Defines in the config.h file should be edited to suit.

-REPEATS number of times to chime

-SNOOZETIME how long to add before next chime

-CHIME path to the chime to play

-FONTNAME font to use

Dependencies:- _xlib_ _alsa(for aplay)_

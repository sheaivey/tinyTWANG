# tinyTWANG
1D Dungeon Crawler Game for the ATtiny10
Written by Shea Ivey

## About
[GIF of Game Play]

Similar to the hit game TWANG but using two buttons and rewritten to work within the constraints of the ATtiny10 microcontroller (1024 bytes of program space and 32 bytes of dynamic memory). The game is currently compiling around 838 bytes. 

The game is simple use the buttons to move the player (Green Pixel) to the end of the level (Blue Pixel) while avoiding lava (Orange Pixels) and enemies (Red Pixels). Pressing both buttons at the same time will attack and kill any enemy within range.

## Programming
* Requires USBasp2.0 with latest firmware.
* [How to program the ATtiny10](http://www.technoblogy.com/show?1YQY) by David Johnson-Davies

## Hardware
* 1x ATtiny10
* 2x 10k Ohm Pull Down Resistors
* 2x Momentary Push Buttons
* 1x WS2812B LED Strip (Up to 200 pixels in length)

Tested operating voltage range (3.0v to 5.5v). 
Current depends on LED Strip size (Calculate max current = 20 * 3 * PIXEL_COUNT). Without LED Strip 1mah/0mW (ATtiny10)

* [Add Schematic]

## Future plans
* Consolidate Buttons into one I/O pin and add speaker for sound effects.
* Design a simple coin cell powered dev kit to teach kids soldering and programming.
* Eye catching screen saver when no user interactions have happened.

## Links & Credits
* [TWANG Game](https://github.com/Critters/TWANG) - Critters
* [WS2812B LEDs on the ATtiny10](https://gist.github.com/MichMich/32ca143709ef9391f1f16c88a824188e) - MichMich
* [Overclock the ATtiny10 (from 4MHz(0x00) to 15 MHz(0xFF))](https://hackaday.io/project/10116-minimalist-a-go-go/log/37495-overclocking-attiny10) - kodera2t
* [Inline Assembly Tutorials](https://ucexperiment.wordpress.com/2016/03/07/arduino-inline-assembly-tutorial-2/) - Jim Eli

### License
[MIT License](/LICENSE)

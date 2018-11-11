/*
  1D Dungeon Crawler Game for the ATtiny10
  Written by Shea Ivey
  
  Similar to the hit game TWANG but using two buttons and rewritten to work 
  within the constraints of the ATtiny10 microcontroller (1024 bytes of 
  program space and 32 bytes of dynamic memory). The game is currently 
  compiling around 838 bytes.

  The game is simple use the buttons to move the player (Green Pixel) to the 
  end of the level (Blue Pixel) while avoiding lava (Orange Pixels) and 
  enemies (Red Pixels). Pressing both buttons at the same time will attack 
  and kill any enemy within range.
  
  MIT License
  
  Copyright (c) 2018 Shea Ivey
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

// BASIC HELPERS
#define F_CPU 8000000UL
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
//#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define delay(ms) _delay_ms(ms)
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
//#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
//#define round(x)     ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))
//#define radians(deg) ((deg)*DEG_TO_RAD)
//#define degrees(rad) ((rad)*RAD_TO_DEG)
//#define sq(x) ((x)*(x))

// PINS
#define PIN 0
#define PIN_BACKWARD 1
#define PIN_FORWARD 2

// BASIC GAME SETTINGS
#define NUM_LEDS 144
#define GAME_SPEED 50
#define ATTACK_WIDTH max(2, NUM_LEDS/32)
#define LAVA_WIDTH min(max(1, NUM_LEDS/18), 8)
#define LAVA_START (NUM_LEDS/2-LAVA_WIDTH)
#define LAVA_END (NUM_LEDS/2+LAVA_WIDTH-1)


// LED STRIP COLOR ORDER WS2812B
enum {
  GREEN,
  RED,
  BLUE
};

// GAME STATES
enum {
  WIN,
  END,
  PLAYING
};

int main() { // setup()
  // Set the clock speed of the ATtiny10 to 8mhz.
  CCP = 0xD8; // 8mhz 
  CLKPSR = 0;
  
  // Overclock (from 4MHz(0x00) to 15 MHz(0xFF))
  // From https://hackaday.io/project/10116-minimalist-a-go-go/log/37495-overclocking-attiny10
  OSCCAL = 0xFF; 

  // Setup I/O ports
  DDRB  = 0b0001;

  // globals
  uint8_t player = 0;
  uint8_t enemy = NUM_LEDS+20;
  uint8_t tick = 0;
  uint8_t state = PLAYING;

  while(1) { //loop()
    uint8_t color[3] = {0,0,0}; //rgb
    uint8_t attacking;
    uint8_t lavaActive;
    tick++; // simple loop counter for animations and timing.

    if((state == WIN || state == END) && tick > 60) {
      state = PLAYING; // reset the game
    }
   
    attacking = 0;
    lavaActive = tick%60 >30;

    // BUTTON LOGIC
    if(bitRead(PINB, PIN_BACKWARD) && bitRead(PINB, PIN_FORWARD)) {
      // attack when both buttons are pressed.
      attacking = player+ATTACK_WIDTH;
    }
    else if(bitRead(PINB, PIN_BACKWARD) && player > 0) {
      player--; // move backward
    }
    else if(bitRead(PINB, PIN_FORWARD) && player < NUM_LEDS-1) {
      player++; // move forward
    }

    // GAME LOGIC
    if( player == NUM_LEDS-1 && state == PLAYING) {
      // Player reached the end... winner!!!
      state = WIN;
      tick = 0;
    }
    
    if(lavaActive && player>=LAVA_START && player<=LAVA_END) {
      // Player died in lava
      state = END; // kill player
      tick = 0;
    }
    
    if(tick%2 == 0) {
      enemy--;
    }
    if(lavaActive && enemy>=LAVA_START && enemy<=LAVA_END) {
      // lava also kills enemy
      enemy = NUM_LEDS+10+tick%30; // kill enemy... really move it out of view
    }
    
    if(attacking>enemy) {
      // enemy is within attack range
      enemy = NUM_LEDS+10+tick%30; // kill enemy... really move it out of view
    }
    if(player > enemy) {
      // player was killed by enemy
      state = END; // kill player
      tick = 0;
    }

    if(state == WIN) {
      player=0; // reset      
      //color[RED] = 0;
      //color[GREEN] = 255-(tick*4);
      //color[BLUE] = 0;
      wheelColor(tick*8, color[RED], color[GREEN], color[BLUE]);
    }
    else if(state == END) {
      player=0; // reset
      enemy = NUM_LEDS+20;
      
      color[RED] = 255-(tick*4);
      color[GREEN] = 0;
      color[BLUE] = 0;
    }

    // THE WS2812B LOOP
    // This loop is extremly time sensitive.
    // Since the ATtiny10 only has 32 bytes for local variables we need to 
    // calculate each pixels color within the loop.
    for(uint8_t i=0; i<NUM_LEDS;i++) {
      if(state != PLAYING) goto _SEND_COLORS; // Jump to render WIN/END pixels.

      // GAME LOGIC FOR WHAT THE CURRENT PIXEL COLOR SHOULD BE.
      color[RED] = 0;
      color[GREEN] = 0;
      color[BLUE] = 0;
      if(lavaActive && i>=LAVA_START && i<=LAVA_END) {
        color[RED] = 150-((tick%3)*20); // gives lava some flicker.
        color[GREEN] = 40;
        /*  
        // todo figure out why this makes the timing so bad that some LED strips glitch.
        if(lavaActive) {
          color[RED] = 200;
          color[GREEN] = 40;
        }
        else {
          color[RED] = 20;
          color[GREEN] = 5;
        }*/
      }
      if(i == player) {
        color[GREEN] = 255;
      }
      else if(i == NUM_LEDS-1 || (i >= player && i<=attacking)) { // exit
        color[BLUE] = 255;
      }
      else if(enemy == i) {
        color[RED] = 255;
      }
      // END GAME LOGIC

      _SEND_COLORS: // GOTO
      for(uint8_t c = 0; c<3;c++) {
        uint8_t byte = color[c];
        for( uint8_t bit = 0 ; bit < 8 ; bit++ ) {
          if (bitRead( byte , 7 )) {
            asm volatile (
              "sbi %[port], %[bit] \n"        
              "nop \n nop \n nop \n nop \n nop \n nop \n nop \n"
              "cbi %[port], %[bit] \n"      
              "nop \n"
              ::
              [port]    "I" (_SFR_IO_ADDR(PORTB)),
              [bit]     "I" (PIN)
            );        
          } else {      
            asm volatile (
              "sbi %[port], %[bit] \n"    
              "nop \n"
              "cbi %[port], %[bit] \n"
              "nop \n nop \n nop \n nop \n nop \n nop \n nop \n"
              ::
              [port]    "I" (_SFR_IO_ADDR(PORTB)),
              [bit]     "I" (PIN)
            );
          }
          byte <<= 1;                                  
        }
      }
    }
    delay(GAME_SPEED);
  }
}


// Quick color wheel byte value from 0-255 represents 0-360
void wheelColor(uint8_t wheelPosition, uint8_t &r, uint8_t &g, uint8_t &b) {
  wheelPosition = 255 - wheelPosition;
  if(wheelPosition < 85) {
    r = 255 - wheelPosition * 3;
    g = 0;
    b = wheelPosition * 3;
    return;
  }
  
  if(wheelPosition < 170) {
    wheelPosition -= 85;
    r = 0;
    g = wheelPosition * 3;
    b = 255 - wheelPosition * 3;
    return;
  }

  wheelPosition -= 170;
  r = wheelPosition * 3;
  g = 255 - wheelPosition * 3;
  b = 0;
}

/*
MIDI controller

8 footswitch, 2 expression pedal with 8 neopixel foot controller for Eventide H9.
License MPL v2.0, originally written by Jack Ha.

button numbering:
5 6 7 8
1 2 3 4

8: switch from preset mode to looper mode and vice versa
7: tuner

preset mode
1-4 select 1-4
5,6 page up/down

current bank lights up
press button = all leds light up
expression pedal change = all leds get intensity

looper mode:
1 rec
2 play
3 stop
4 empty

5 direction
6 octave switch

of course, in the code the btn index start at 0.

based on:

 http://www.arduino.cc/en/Tutorial/Midi

from 

https://ccrma.stanford.edu/~craig/articles/linuxmidi/misc/essenmidi.html

   0x80     Note Off
   0x90     Note On
   0xA0     Aftertouch
   0xB0     Continuous controller
   0xC0     Patch change
   0xD0     Channel Pressure
   0xE0     Pitch bend
   0xF0     (non-musical commands)
   
 */
#include <Adafruit_NeoPixel.h>

#define DELAYTIME 20
#define NUM_BUTTONS 8
#define NUM_ANALOGS 2
#define LED 13

#define MIDI_NOTE 0x90
#define MIDI_CC 0xB0
#define MIDI_PC 0xC0
#define MIDI_CC_HIGH_VALUE 0x45

#define MODE_PRESET 1  // fast selection of presets
#define MODE_LOOPER 2  // looper functions
#define MODE_EASY 3  // not implemented

// program Eventide H9 (or any other device) accordingly.
#define MIDI_CC_TUNER 0
#define MIDI_CC_PRESET_START 40  // PRESET_START + index (0 to 3) + 4 * current_page will be sent.
#define MIDI_CC_LOOPER_RECORD 109
#define MIDI_CC_LOOPER_PLAY 110
#define MIDI_CC_LOOPER_STOP 111
#define MIDI_CC_LOOPER_EMPTY 112
#define MIDI_CC_LOOPER_DIRECTION 113
#define MIDI_CC_LOOPER_OCTAVE 114

// buttons!
// looper mode
#define BTN_REC 0
#define BTN_PLAY 1
#define BTN_STOP 2
#define BTN_EMPTY 3
#define BTN_DIRECTION 4
#define BTN_OCTAVE 5

// prset mode
#define BTN_PRESET1 0
#define BTN_PRESET2 1
#define BTN_PRESET3 2
#define BTN_PRESET4 3

#define BTN_PAGE_UP 5
#define BTN_PAGE_DOWN 4

// easy mode
#define BTN_ACTIVE 0
#define BTN_PRESET_DOWN 1
#define BTN_PRESET_UP 2
#define BTN_PERFORMANCE 3

#define BTN_PRESET_DOWN_LOAD 4
#define BTN_PRESET_UP_LOAD 5


#define BTN_TUNER 6
#define BTN_MODE 7

// yes we use an 8 led neopixel! 
#define NEOPIXEL_PIN            6
// How many NeoPixels are attached to the Arduino?
#define NUM_PIXELS      8

// indices for color
#define RED 0
#define GREEN 1
#define BLUE 2

const int button_pin[] = {2,3,4,5, 8,10,9,11};

// looper mode
int button_cmd[] = {MIDI_CC, MIDI_CC, MIDI_CC, MIDI_CC, MIDI_CC, MIDI_CC, MIDI_CC, MIDI_CC};
int button_num[] = {21, 20, 2, 2, 21, 20, 2, 2};

// preset mode

const int analog_pin[] = {A0, A1}; 
int analog_cc[] = {10, 32, 33};  // which cc to send for each analog input

// self-calibrating
int analog_min[] = {1023, 1023};
int analog_max[] = {0, 0};

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
const int color_button[] = {0,0,30};
const int color_analog[][3] = {{30,0,0},{0,20,0}, {0,10,20}};
int current_colors[NUM_PIXELS][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}; 
int last_colors[NUM_PIXELS][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
boolean colors_changed = false;

/* modes etc */

int button_state[] = {0,0,0,0,0,0,0,0};
int button_press[] = {0,0,0,0,0,0,0,0};  // is set for an instant
int button_depress[] = {0,0,0,0,0,0,0,0};  // is set for an instant

long new_state = 0;
long new_value = 0;

int analog_state[] = {0, 0};

int current_mode = MODE_PRESET;  // current bank lights up
int current_page = 0;  // 8 pages because we have 8 leds :-)

boolean have_action = false;


void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
    
  for (int i=0; i<NUM_BUTTONS; i++) {
    pinMode(button_pin[i], INPUT_PULLUP);
  }
  pinMode(LED, OUTPUT);

  //  Set MIDI baud rate:
  Serial.begin(31250);
  //Serial.begin(9600);
}

void loop() {
  // reset colors, we set them at the end of the loop function
  for (int led=0; led<NUM_PIXELS; led++) {
    for (int x=0; x<3; x++) {
      current_colors[led][x] = 0;
    }
  }
  
  // reset changes
  for (int i=0; i<NUM_BUTTONS; i++) {
    button_press[i] = 0;
    button_depress[i] = 0;
  }
  
  // analog inputs
  for (int i=0; i<NUM_ANALOGS; i++) {
    new_state = analogRead(analog_pin[i]);
    if (new_state < analog_min[i]) {
      analog_min[i] = new_state;
//      Serial.println("min=");
//      Serial.println(new_state);
    }
    if (new_state > analog_max[i]) {
      analog_max[i] = new_state;
//      Serial.println("max=");
//      Serial.println(new_state);
    }
    new_value = 128 * (new_state - analog_min[i]) / (analog_max[i] - analog_min[i]);
    if ((abs(analog_state[i] - new_value) > 2) && (analog_max[i] - analog_min[i] > 100)) {  // value changed + check for calibration
//      Serial.print("new value: ");
//      Serial.println(new_value);
//      Serial.println(new_state);
//      Serial.println(analog_max[i]);
//      Serial.println(analog_min[i]);
      // send new cc
      midi_cc(MIDI_CC, analog_cc[i], new_value);
      
//      pixels.show();
      digitalWrite(LED, HIGH);
      //delay(DELAYTIME);
      analog_state[i] = new_value;
    } else {
      digitalWrite(LED, LOW);
    }

    // update led
//    for (int led=0; led<NUM_PIXELS; led++) {
//        if (new_value > led * 16) {
//          current_colors[led][0] += color_analog[i][0];
//          current_colors[led][1] += color_analog[i][1];
//          current_colors[led][2] += color_analog[i][2];
//        } 
//      }
  }
  
  for (int i=0; i<NUM_BUTTONS; i++) {
    new_state = digitalRead(button_pin[i]);
    if ((new_state == LOW) && (button_state[i] != new_state)) {
      button_press[i] = 1;
    }
    if ((new_state == HIGH) && (button_state[i] != new_state)) {
      button_depress[i] = 1;
    }
    button_state[i] = new_state;
  }
    
  // momentary buttons
  have_action = false;
  if (button_press[BTN_MODE]) {
    have_action = true;
    switch (current_mode) {
//      case MODE_EASY: 
//        current_mode = MODE_LOOPER; 
//        break;
      case MODE_PRESET: 
        current_mode = MODE_LOOPER; 
        break;
      case MODE_LOOPER: 
        current_mode = MODE_PRESET; 
        break;
    }
  }
  
  if (button_press[BTN_TUNER]) {
    have_action = true;
    midi_cc(MIDI_CC, MIDI_CC_TUNER, MIDI_CC_HIGH_VALUE);
  }
  if (button_depress[BTN_TUNER]) {
    have_action = true;
    midi_cc(MIDI_CC, MIDI_CC_TUNER, 0x0);
  }
  
  if (current_mode == MODE_PRESET) {
    if (button_press[BTN_PRESET1]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+0+current_page*4, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_PRESET1]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+0+current_page*4, 0);
    }

    if (button_press[BTN_PRESET2]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+1+current_page*4, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_PRESET2]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+1+current_page*4, 0);
    }

    if (button_press[BTN_PRESET3]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+2+current_page*4, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_PRESET3]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+2+current_page*4, 0);
    }

    if (button_press[BTN_PRESET4]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+3+current_page*4, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_PRESET4]) {
      have_action = true;
      midi_cc(MIDI_PC, MIDI_CC_PRESET_START+3+current_page*4, 0);
    }

    if (button_press[BTN_PAGE_UP]) {
      have_action = true;
      current_page = (current_page + 1) % 8;
    }
    if (button_press[BTN_PAGE_DOWN]) {
      have_action = true;
      current_page = (current_page + 7) % 8;
    }
  }

  if (current_mode == MODE_LOOPER) {
    if (button_press[BTN_REC]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_RECORD, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_REC]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_RECORD, 0);
    }

    if (button_press[BTN_PLAY]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_PLAY, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_PLAY]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_PLAY, 0);
    }

    if (button_press[BTN_STOP]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_STOP, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_REC]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_STOP, 0);
    }

    if (button_press[BTN_DIRECTION]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_DIRECTION, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_DIRECTION]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_DIRECTION, 0);
    }

    if (button_press[BTN_OCTAVE]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_OCTAVE, MIDI_CC_HIGH_VALUE);
    }
    if (button_depress[BTN_OCTAVE]) {
      have_action = true;
      midi_cc(MIDI_CC, MIDI_CC_LOOPER_OCTAVE, 0);
    }
  }
  
  
  /* now the 8 neopixels! */
  
  // analog input status
  /*
  current_colors[4][RED] = analog_state[0]; 
  current_colors[5][RED] = analog_state[0]; 
  current_colors[6][RED] = analog_state[0]; 
  current_colors[7][RED] = analog_state[0]; 
  
  current_colors[0][RED] = analog_state[1]; 
  current_colors[1][RED] = analog_state[1]; 
  current_colors[2][RED] = analog_state[1]; 
  current_colors[3][RED] = analog_state[1]; 
   */
   
  // status
  if (current_mode == MODE_PRESET) {
    // blue glow
    for (int i=0; i<NUM_BUTTONS; i++) {
      current_colors[i][RED] += 2;
      if (button_state[i] == LOW) {
        for (int j=0; j<NUM_BUTTONS; j++) {
          current_colors[j][RED] += 10;
        }
      }
    }
    // current page
    current_colors[current_page][BLUE] += 20; 
  }

  if (current_mode == MODE_LOOPER) {
    // green glow
    for (int i=0; i<NUM_BUTTONS; i++) {
      current_colors[i][GREEN] += 2;
      if (button_state[i] == LOW) {
        for (int j=0; j<NUM_BUTTONS; j++) {
          current_colors[j][GREEN] += 10;
        }
      }    
    }
  }

  /* lazy debouncing */
  if (have_action) {
    delay(DELAYTIME);
  }
  
  /*
  for (int i=0; i<NUM_BUTTONS; i++) {
    new_state = digitalRead(button_pin[i]);

    if ((new_state == LOW) && (button_state[i] != new_state)) {
      midi_cc(button_cmd[i], button_num[i], 0x45);
      digitalWrite(LED, HIGH);
      delay(DELAYTIME);
    }
    if ((new_state == HIGH) && (button_state[i] != new_state)) {
      midi_cc(button_cmd[i], button_num[i], 0x00);
      delay(DELAYTIME);
      digitalWrite(LED, LOW);
    }
    button_state[i] = new_state;

    // check if any leds must be on for button press
    if (new_state == LOW) {
      current_colors[i][0] += color_button[0];
      current_colors[i][1] += color_button[1];
      current_colors[i][2] += color_button[2];
    }
  }
  */
      
         
  // store current pixel settings
  // check if setting pixels is needed 
  colors_changed = false;
  for (int led=0; led<NUM_PIXELS; led++) {
    for (int x=0; x<3; x++) {
      if (last_colors[led][x] != current_colors[led][x]) {
        colors_changed = true;
      }
      last_colors[led][x] = current_colors[led][x];
    }
  }
 
  // set colors if needed
  if (colors_changed) {
    for (int led=0; led<NUM_PIXELS; led++) {
      pixels.setPixelColor(led, pixels.Color(current_colors[led][0],current_colors[led][1],current_colors[led][2])); 
    }
    pixels.show();
  }
  
  delay(1);
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void midi_cc(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

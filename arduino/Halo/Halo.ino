#include <Wire.h>
#include <FastLED.h>
#include <SparkFun_APDS9960.h>

FASTLED_USING_NAMESPACE
// Intended target: Arduino Nano with ATMEGA 328

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
#define LED_TYPE    WS2812
//#define DATA_PIN    20
//#define CLK_PIN   19
//#define LED_TYPE    APA102

#define COLOR_ORDER GRB
#define NUM_LEDS     64
#define BUTTON_PIN    4
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

uint32_t last_press = 0;
#define DEBOUNCE  200
#define LONG_PRESS_DURATION 1000
#define TIMEOUT (10L * 60L * 1000L)

int8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
int8_t gLastPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

const int    APDS_UP = 1;
const int  APDS_DOWN = 2;
const int  APDS_LEFT = 3;
const int APDS_RIGHT = 4;
const int  APDS_NEAR = 5;
const int   APDS_FAR = 6;
const int  APDS_NONE = 7;

SparkFun_APDS9960 g_apds = SparkFun_APDS9960();

typedef void (*SimplePatternList[])();
void blue();
void green();
void red();
void white();
void rainbow();
void rainbowWithGlitter();
void sinelon();
void juggle();
void bpm();
void pattern_cycle();
void nextPattern();
void off();
void setOff();
void addGlitter(fract8 chanceOfGlitter);
void apds_setup();

SimplePatternList gPatterns = {blue, green, red, white,
			       rainbow, rainbowWithGlitter, sinelon, juggle, bpm,
			       pattern_cycle};


uint32_t last_interaction = 0;

void interact(){
  int gesture = handleGesture();
  if(gesture > 0){
    last_interaction = millis();
  }
  if(gesture == APDS_LEFT){
    nextPattern();
    Serial.println("Next");
  }
  if(gesture == APDS_RIGHT){
    prevPattern();
    Serial.println("Prev");
  }
  if(millis() - last_press > DEBOUNCE && digitalRead(BUTTON_PIN) == LOW){
    nextPattern();
    gPatterns[gCurrentPatternNumber]();
    my_show();
    uint32_t press_start = millis();
    uint32_t press_dur = millis() - press_start;
    uint32_t attempt = 0;
    while(digitalRead(BUTTON_PIN) == LOW && press_dur < LONG_PRESS_DURATION){
      if(attempt == 0){
	Serial.println("Wait for relase...");
      }
      attempt++;
      press_dur = millis() - press_start;
    }
    if(press_dur >= LONG_PRESS_DURATION){
      setOff();
      off();
      my_show();
      Serial.println("Long Press");
      while(digitalRead(BUTTON_PIN) == LOW){
	// insure release
      }
    }
    else{
      Serial.println("normal press");
    }
    last_press = millis();
    Serial.println("released");
  }
  if(millis() - last_interaction > TIMEOUT){
    gCurrentPatternNumber++; // pre-increment (like button long press does)
    setOff();
    off();
    my_show();
  }
  //Serial.print(millis() - last_interaction);
  //Serial.print(" ");
  //Serial.println(TIMEOUT);
}

void setup() {
  Serial.begin(115200);
  delay(300); // 3 second delay for recovery
  Serial.println("WyoLum.com:: Halo");
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).
    setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(BUTTON_PIN, INPUT);

  apds_setup();  
}


void amp_profile(){
  return;
  int i;
  CHSV hsv;
  for(i = 0; i < NUM_LEDS; i++){ // bright in the middle fade to sides
    hsv = rgb2hsv_approximate(leds[i]);
    leds[i] = CHSV(hsv.hue, hsv.saturation, 127 + 128 * cos((i - NUM_LEDS/2) * 2 * PI / (.7 * NUM_LEDS)));
  }
}

void my_show(){
  // consolidate all show activities
  amp_profile();
  FastLED.show();
}
// List of patterns to cycle through.  Each is defined as a separate function below.
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  if(gCurrentPatternNumber >= 0){
    gPatterns[gCurrentPatternNumber]();
  }

  amp_profile();
  
  // send the 'leds' array out to the actual LED strip
  my_show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  interact();
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setOff(){
  gLastPatternNumber = (gCurrentPatternNumber - 1) % (ARRAY_SIZE(gPatterns));
  gCurrentPatternNumber = -1;
}
void nextPattern(){
  if(gCurrentPatternNumber < 0){
    gCurrentPatternNumber = gLastPatternNumber;
  }
  else{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  }
}

void prevPattern(){
  if(gCurrentPatternNumber < 0){
    gCurrentPatternNumber = gLastPatternNumber;
  }
  else{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber - 1) % ARRAY_SIZE( gPatterns);
  }
}

int pattern_cycle_num = 0;


/// patterns
int hue = 0;
int divisor = 30;
#define MIN_BRIGHTNESS 8
#define MAX_BRIGHTNESS 255
void breath () {
 float breath = (exp(sin(millis()/5000.0*PI)) - 0.36787944)*108.0;
 breath = map(breath, 0, 255, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
 FastLED.setBrightness(breath);
 fill_rainbow(leds, NUM_LEDS, (hue++/divisor));
 if(hue == (255 * divisor)) {
 	hue = 0;
 }
 delay(5);
}

void pattern_cycle(){
  gPatterns[pattern_cycle_num]();
  EVERY_N_SECONDS( 10 ) {
    pattern_cycle_num += 1; // last two are cycle and off
    pattern_cycle_num %= ARRAY_SIZE(gPatterns) - 2;
  } // change patterns periodically
}

void off(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
    FastLED.show();
  }
}

void mona(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  for(int i = 8; i < 16; i++){
    leds[i] = CRGB::Red;
  }
  for(int i = 16; i < 24; i++){
    leds[i] = CRGB::Green/8;
  }
}
void blue(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Blue;
  }
}
void red(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Red;
  }
}
void white(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::White;
  }
}
void green(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Green;
  }
}
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

///////////////// gesture sensor
void apds_setup(){
  // Initialize APDS-9960 (configure I2C and initial values)
  if(g_apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 gesture sensor engine
  if (g_apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}
int handleGesture() {
  int out = 0;
  if ( g_apds.isGestureAvailable() ) {
    switch ( g_apds.readGesture() ) {
    case DIR_UP:
      out = APDS_UP;
      break;
    case DIR_DOWN:
      out = APDS_DOWN;
      break;
    case DIR_LEFT:
      out = APDS_LEFT;
      break;
    case DIR_RIGHT:
      out = APDS_RIGHT;
      break;
    case DIR_NEAR:
      out = APDS_NEAR;
      Serial.println("Near");
      break;
    case DIR_FAR:
      out = APDS_FAR;
      Serial.println("Far");
      break;
    default:
      out = APDS_NONE;
    }
  }
  return out;
}

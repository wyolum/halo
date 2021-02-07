#include <DS3231.h>
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

int8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
int8_t gLastPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
CRGB leds[NUM_LEDS];
CRGB ledsFront[NUM_LEDS/2];
CRGB ledsBack[NUM_LEDS/2];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

uint32_t last_press = 0;
#define DEBOUNCE  200
#define LONG_PRESS_DURATION 3000
#define TIMEOUT (10L * 60L * 1000L)

const int    APDS_UP = 1;
const int  APDS_DOWN = 2;
const int  APDS_LEFT = 3;
const int APDS_RIGHT = 4;
const int  APDS_NEAR = 5;
const int   APDS_FAR = 6;
const int  APDS_NONE = 7;

DS3231 Clock;
bool Century=false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

SparkFun_APDS9960 g_apds = SparkFun_APDS9960();

typedef void (*SimplePatternList[])(CRGB *leds);
void blue(CRGB *leds);
void green(CRGB *leds);
void red(CRGB *leds);
void white(CRGB *leds);
void rainbow(CRGB *leds);
void rainbowWithGlitter(CRGB *leds);
void sinelon(CRGB *leds);
void juggle(CRGB *leds);
void bpm(CRGB *leds);
void nextPattern();
void off();
void setOff();
void addGlitter(CRGB *leds,fract8 chanceOfGlitter);
void apds_setup();

SimplePatternList gPatterns = {blue, green, red, white,
			       rainbow, rainbowWithGlitter, juggle, sinelon, bpm
			       };

int handleGesture();
void prevPattern();
void my_show();
void black(CRGB *leds);
void hour(int hh);
void minute(int mm);
void fill_black(){
  fill_solid(ledsFront, NUM_LEDS/2, CRGB::Black);
  fill_solid(ledsBack, NUM_LEDS/2, CRGB::Black);
}
void fill_white(){
  fill_solid(ledsFront, NUM_LEDS/2, CRGB::White);
  fill_solid(ledsBack, NUM_LEDS/2, CRGB::White);
}

void splash(){
  int hh = Clock.getHour(h12, PM);
  int mm = Clock.getMinute();
  int steps_to_go = hh + (120 - mm)/5;
  int _delay = 75;
  int final_delay = 300;
  int delta_delay = (final_delay - _delay) / steps_to_go;

  int i;
  int mm_counter = 0;
  
  Serial.println("splash()");
  for(i = hh; i < hh + 12; i++){
    fill_black();
    hour(i);
    minute((60 + mm - 5 * mm_counter) % 60);
    my_show();
    delay(_delay);
    mm_counter++;
  }
  Serial.println(mm_counter);
  Serial.println(mm);
  Serial.println();
  for(i = mm_counter; 120 + mm - i * 5 >= mm; i++){
    fill_black();
    minute(120 + mm - 5 * i);
    hour(hh);
    my_show();
    delay(_delay);
    _delay += delta_delay;
  }
  fill_black();
  hour(hh);
  minute(mm);
  my_show();
}

uint32_t last_interaction = 0;
int maphalo2(int i){
  if (i < 32)
    return(i*2 + i%2);
  return (i-32)*2+!(i%2);
}
void interact(){
  if(Serial.available()){
    splash();
    while(Serial.available()){
      Serial.read();
    }
  }
  return;
  int gesture = handleGesture();
  if(gesture > 0){
    last_interaction = millis();
  }
  if(gesture == APDS_UP){
    nextPattern();
    Serial.println("Next");
  }
  if(gesture == APDS_DOWN){
    prevPattern();
    Serial.println("Prev");
  }
  if(millis() - last_press > DEBOUNCE && digitalRead(BUTTON_PIN) == LOW){
    Serial.println("thinks button pressed");
    nextPattern();
    gPatterns[gCurrentPatternNumber](ledsFront);
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

void hour(int hh){
  if(hh % 12 == 0){
    ledsFront[14] = CRGB::Blue;    
    ledsFront[15] = CRGB::Blue;
  }
  if(hh % 12 == 1){
    ledsFront[18] = CRGB::Blue;    
    ledsFront[19] = CRGB::Blue;
  }
  else if(hh % 12 == 2){
    ledsFront[22] = CRGB::Blue;    
    ledsFront[23] = CRGB::Blue;
  }
  else if(hh % 12 == 3){
    ledsFront[26] = CRGB::Blue;    
    ledsFront[27] = CRGB::Blue;
  }
  else if(hh % 12 == 4){
    ledsFront[24] = CRGB::Blue;    
    ledsFront[25] = CRGB::Blue;
  }
  else if(hh % 12 == 5){
    ledsFront[20] = CRGB::Blue;    
    ledsFront[21] = CRGB::Blue;
  }
  else if(hh % 12 == 6){
    ledsFront[16] = CRGB::Blue;    
    ledsFront[17] = CRGB::Blue;
  }
  else if(hh % 12 == 7){
    ledsFront[12] = CRGB::Blue;    
    ledsFront[13] = CRGB::Blue;
  }
  else if(hh % 12 == 8){
    ledsFront[ 8] = CRGB::Blue;    
    ledsFront[ 9] = CRGB::Blue;
  }
  else if(hh % 12 == 9){
    ledsFront[ 2] = CRGB::Blue;    
    ledsFront[ 3] = CRGB::Blue;
  }
  else if(hh % 12 == 10){
    ledsFront[ 6] = CRGB::Blue;    
    ledsFront[ 7] = CRGB::Blue;
  }
  else if(hh % 12 == 11){
    ledsFront[10] = CRGB::Blue;    
    ledsFront[11] = CRGB::Blue;
  }
}
void minute(int mm){
  mm = (mm / 5) * 5;
  if(mm % 60 == 30){
    ledsBack[14] = CRGB::Green;    
    ledsBack[15] = CRGB::Green;
  }
  if(mm % 60 == 25){
    ledsBack[18] = CRGB::Green;    
    ledsBack[19] = CRGB::Green;
  }
  else if(mm % 60 == 20){
    ledsBack[22] = CRGB::Green;    
    ledsBack[23] = CRGB::Green;
  }
  else if(mm % 60 == 15){
    ledsBack[26] = CRGB::Green;    
    ledsBack[27] = CRGB::Green;
  }
  else if(mm % 60 == 10){
    ledsBack[24] = CRGB::Green;    
    ledsBack[25] = CRGB::Green;
  }
  else if(mm % 60 ==  5){
    ledsBack[20] = CRGB::Green;    
    ledsBack[21] = CRGB::Green;
  }
  else if(mm % 60 ==  0){
    ledsBack[16] = CRGB::Green;    
    ledsBack[17] = CRGB::Green;
  }
  else if(mm % 60 == 55){
    ledsBack[12] = CRGB::Green;    
    ledsBack[13] = CRGB::Green;
  }
  else if(mm % 60 == 50){
    ledsBack[ 8] = CRGB::Green;    
    ledsBack[ 9] = CRGB::Green;
  }
  else if(mm % 60 == 45){
    ledsBack[ 2] = CRGB::Green;    
    ledsBack[ 3] = CRGB::Green;
  }
  else if(mm % 60 == 40){
    ledsBack[ 6] = CRGB::Green;    
    ledsBack[ 7] = CRGB::Green;
  }
  else if(mm % 60 == 35){
    ledsBack[10] = CRGB::Green;    
    ledsBack[11] = CRGB::Green;
  }
}
void setup() {
  Serial.begin(115200);
  delay(300); // .3 second delay for recovery
  Serial.println("WyoLum.com:: Halo");
  
  Wire.begin();
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).
    setCorrection(TypicalLEDStrip);
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(BUTTON_PIN, INPUT);
  //fill_white();
  //my_show();
  //while(1)delay(100);
  //splash();
  apds_setup();
  
}

// this routine hasn't been modified for dual strip **kgo
void amp_profile(){
  return;
  int i;
  CHSV hsv;
  for(i = 0; i < NUM_LEDS; i++){ // bright in the middle fade to sides
    hsv = rgb2hsv_approximate(leds[maphalo2(i)]);
    leds[i] = CHSV(hsv.hue, hsv.saturation, 127 + 128 * cos((i - NUM_LEDS/2) * 2 * PI / (.7 * NUM_LEDS)));
  }
}

void my_show(){
  // consolidate all show activities
  //amp_profile();
  // copy front and back arrays into main array.
  for (int i = 0; i < NUM_LEDS/2; i++){
    leds[maphalo2(i)] = ledsBack[i];
    leds[maphalo2(i+32)]= ledsFront[i];
  }
  FastLED.show();
}
// List of patterns to cycle through.  Each is defined as a separate function below.
uint8_t last_mm = 255;
void loop()
{
  uint8_t mm = Clock.getMinute();
  if(mm / 5 != last_mm / 5){
    last_mm = mm;
    splash();
    //black(ledsFront);
    //black(ledsBack);
    //hour(Clock.getHour(h12, PM));
    //minute(Clock.getMinute());
  }
  /*
  Serial.print(Clock.getHour(h12, PM));
  Serial.print(":");
  Serial.print(Clock.getMinute());
  Serial.print(":");
  Serial.println(Clock.getSecond());
  */
  interact();
  my_show();
  delay(10L);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setOff(){
  Serial.println("setOff");
  gLastPatternNumber = (gCurrentPatternNumber - 1) % (ARRAY_SIZE(gPatterns));
  gCurrentPatternNumber = -1;
}
void nextPattern(){
  Serial.print("pattern: ");Serial.println(gCurrentPatternNumber);
  Serial.print("lastpattern: ");Serial.println(gLastPatternNumber);
  if(gCurrentPatternNumber < 0){
    gCurrentPatternNumber = gLastPatternNumber;
    Serial.println("next set to last");
  }
  else{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
    Serial.print("incrementing pattern:");Serial.println(gCurrentPatternNumber);
  }
}

void prevPattern(){
  Serial.print("prevPattern");
  if(gCurrentPatternNumber < 0){
    gCurrentPatternNumber = gLastPatternNumber;
    Serial.println("prevPattern: Next set to last");
  }
  else{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber - 1) % ARRAY_SIZE( gPatterns);
    Serial.println("prevPattern: pattern decremented");
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


void off(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
    FastLED.show();
  }
}

void mona(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::Black;
  }
  for(int i = 8; i < 16; i++){
    leds[i] = CRGB::Blue;
  }
  for(int i = 16; i < 24; i++){
    leds[i] = CRGB::Green/8;
  }
}
void blue(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::Blue;
  }
}
void red(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::Blue;
  }
}
void white(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::White;
  }
}
void black(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::Black;
  }
}
void green(CRGB *leds){
  for(int i = 0; i < NUM_LEDS/2; i++){
    leds[i] = CRGB::Green;
  }
}
void rainbow(CRGB *leds) 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS/2, gHue, 7);
}

void rainbowWithGlitter(CRGB *leds) 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow(leds);
  addGlitter(leds,80);
}

void addGlitter( CRGB *leds, fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS/2) ] += CRGB::White;
  }
}

void sinelon(CRGB *leds)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS/2, 20);
  int pos = beatsin16( 13, 0, (NUM_LEDS/2)-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm(CRGB *leds)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS/2; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle(CRGB *leds) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS/2, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, (NUM_LEDS/2)-1 )] |= CHSV(dothue, 200, 255);
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

/*
 * Designed by Timothy Ramsey McReynolds on 2/5/19
 */

#include <FastLED.h>
#include <Adafruit_NeoPixel.h>

#define BRIGHTNESS 200

#define ELEVATOR_PIN 3
#define INTAKE_PIN 4
#define STATE_PIN_1 7
#define STATE_PIN_2 8
#define ALLIANCE_PIN 9

#define ELEVATOR_NUM_LEDS 54
#define ELEVATOR_FIRE_COOLING  75
#define ELEVATOR_FIRE_SPARKING 175

#define INTAKE_NUM_LEDS 26
#define INTAKE_FIRE_COOLING  75
#define INTAKE_FIRE_SPARKING 175

#define IDLE_CASE 0
#define HATCH_CASE 1
#define CARGO_CASE 2
#define CLIMBING_CASE 3

#define FIRE_MILLIS 0
#define FLASH_MILLIS 1

#define FIRE_SPEED 10
#define FLASH_SPEED 75

Adafruit_NeoPixel elevator = Adafruit_NeoPixel(ELEVATOR_NUM_LEDS, ELEVATOR_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel intake = Adafruit_NeoPixel(INTAKE_NUM_LEDS, INTAKE_PIN, NEO_GRBW + NEO_KHZ800);

const CRGBPalette16 redFirePalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
const CRGBPalette16 blueFirePalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
const CRGB white = CRGB::White;
const CRGB cargoFlash = CRGB::OrangeRed;
const CRGB hatchFlash = CRGB::Yellow;
const CRGB climbFlash = white;

CRGBPalette16 firePalette = redFirePalette;
CRGB flashColor = CRGB::White;

unsigned long previousMillis[] = {0, 0, 0, 0};

byte binaryState = 0;

boolean redAlliance = true, 
   flashed = false;

int inputs[] = 
{
  STATE_PIN_1, 
  STATE_PIN_2
};

void setup()
{
  //Initializes elevator LEDs
  elevator.setBrightness(BRIGHTNESS);
  elevator.begin();
  elevator.show();

  //Initializes intake LEDs
  intake.setBrightness(BRIGHTNESS);
  intake.begin();
  intake.show();
  
  //Initializes the state and alliance input pins
  pinMode(STATE_PIN_1, INPUT);
  pinMode(STATE_PIN_2, INPUT);
  pinMode(ALLIANCE_PIN ,INPUT);
}

void loop()
{
  
  //Determines the binary state based off of state input pins
  binaryState = getState();
  //Determines which alliance to set default fire color and matrix text color to
  redAlliance = digitalRead(ALLIANCE_PIN)==LOW;
  //Run light programs based off of binary state
  switch(binaryState)
  {
    case HATCH_CASE:
    case CARGO_CASE:
    case CLIMBING_CASE:
      runFlash();
      break;
      
    case IDLE_CASE:
    default:
      runFire();
      break;
  }
  
  
  //Actually displays everything changed during loop()
  intake.show();
  elevator.show();

  //Momentarily delays program 
  delay(10);
}

void runFire()
{
  //Adds entropy to random number generator for the fire pattern
  random16_add_entropy( random());
  
  //Sets the color of fire to red or blue depending on alliance color
  if(redAlliance)
    firePalette = redFirePalette;
  else
    firePalette = blueFirePalette;

  //Runs fire programs without delaying the whole program
  if(myDelay(FIRE_MILLIS, FIRE_SPEED))
  {
    elevatorFire();
    intakeFire();
  }
}

void runFlash()
{
  switch(binaryState)
  {
    case HATCH_CASE:
      flashColor = hatchFlash;
      break;
      
    case CARGO_CASE:
      flashColor = cargoFlash;
      break;
      
    case CLIMBING_CASE:
    default:
      flashColor = climbFlash;
      break;
  }
  
  //Runs flashing programs without delaying the whole program
  if(myDelay(FLASH_MILLIS, FLASH_SPEED))
  {
    elevatorFlash();
    intakeFlash();
    flashed = !flashed;
  } 
}

void elevatorFire()
{
  //Design from Fire2012WithPalette modified to work with Neopixel strip instead of FastLED
  static byte heat[ELEVATOR_NUM_LEDS];
  for( int i = 0; i < ELEVATOR_NUM_LEDS; i++) 
  {
    heat[i] = qsub8( heat[i],  random8(0, ((ELEVATOR_FIRE_COOLING * 10) / ELEVATOR_NUM_LEDS) + 2));
  }

  for( int k = ELEVATOR_NUM_LEDS - 1; k >= 2; k--) 
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  
  if( random8() < ELEVATOR_FIRE_SPARKING ) 
  {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  for( int j = 0; j < ELEVATOR_NUM_LEDS; j++) 
  {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( firePalette, colorindex);
    setElevatorPixel(j, color);
  }
}

void intakeFire()
{
  //Mirrors fire on the intake LEDs
  static byte heat[INTAKE_NUM_LEDS / 2];
  for( int i = 0; i < INTAKE_NUM_LEDS / 2; i++)
  {
    heat[i] = qsub8( heat[i],  random8(0, ((INTAKE_FIRE_COOLING * 10) / (INTAKE_NUM_LEDS / 2) + 2)));
  }

  for( int k= (INTAKE_NUM_LEDS / 2) - 1; k >= 2; k--) 
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  
  if( random8() < INTAKE_FIRE_SPARKING ) 
  {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  for( int j = 0; j < (INTAKE_NUM_LEDS / 2); j++) {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette(firePalette, colorindex);
    setIntakeFirstHalf((INTAKE_NUM_LEDS / 2) - 1 - j, color);
    setIntakeSecondHalf(j, color);
  }
}

void elevatorFlash()
{
  //Custom code to flash a color on the elevator
  for(int i = 0; i < ELEVATOR_NUM_LEDS; i++)
  {
    if(!flashed)
      setElevatorPixel(i, flashColor);
    else
      setElevatorPixel(i, CRGB::Black);
  }
}

void intakeFlash()
{
  //Custom code to flash a color on the intake
  for(int i = 0; i < INTAKE_NUM_LEDS; i++)
  {
    if(!flashed)
      setIntakePixel(i, flashColor);
    else
      setIntakePixel(i, CRGB::Black);
  }
}

void setElevatorPixel(int pixel, CRGB color)
{
  //Sets the color of an elevator pixel
  if(color == white)
    setElevatorPixelWhite(pixel);
  else
    elevator.setPixelColor(pixel, elevator.Color(color.r, color.g, color.b, 0));
}

void setIntakePixel(int pixel, CRGB color)
{
  //Sets the color of an intake pixel
  if(color == white)
    setIntakePixelWhite(pixel);
  else
    intake.setPixelColor(pixel, intake.Color(color.r, color.g, color.b, 0));
}

void setElevatorPixelWhite(int pixel)
{
  elevator.setPixelColor(pixel, elevator.Color(255, 255, 255, 255));
}

void setIntakePixelWhite(int pixel)
{
  intake.setPixelColor(pixel, intake.Color(255, 255, 255, 255));
}

void setIntakeFirstHalf(int pixel, CRGB color)
{
  //Sets the color of the first half of the intake LEDs
  if(pixel < INTAKE_NUM_LEDS / 2)
    setIntakePixel(pixel, color);
}

void setIntakeSecondHalf(int pixel, CRGB color)
{
  //Sets the color of the second half of the intake LEDs
  pixel += (INTAKE_NUM_LEDS / 2);
  if(pixel < INTAKE_NUM_LEDS)
    setIntakePixel(pixel, color);
}

byte getState()
{
  //Returns a value from zero to two to the power of the length of inputs array
  byte b = 0;
  for(int i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++)
  {
    bitWrite(b, i, digitalRead(inputs[i]));
  }
  return b;
}

boolean myDelay(int prev, long interval)
{
  //Returns true and resets time of millis() delay if a specified time has past
  //Functions like delay() without fully delaying the program
  unsigned long current = millis();
  if(current - previousMillis[prev] >= interval)
  {
    previousMillis[prev] = current;
    return true;
  }
  return false;
}
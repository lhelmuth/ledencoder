#include <FastLED.h>

//led constants
#define LED_PIN     7
#define NUM_LEDS    8
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

//encoder constants
#define PIN_S1 9
#define PIN_S2 8
#define PIN_KEY 10

//values for encoder
int totalPos;           //relative position of the encoder dial
bool s1Last;
bool s1Val;
bool keyDown;           //encoder button
int keyTime;            //time the button is held
int keyTop;             //total time held when button is released
int startIndex;

//LED/palette objects
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType currentBlending;
int frozenLED;

void setup() 
{   
    //power-up delay to avoid possibility of surging the LED array
    delay(3000); 
    
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    
    currentBlending = NOBLEND;
    pinMode(PIN_S1, INPUT);
    pinMode(PIN_S2, INPUT);
    s1Last = digitalRead(PIN_S1);
    totalPos = 0;
    startIndex = 0;
    keyTime = 0;
    frozenLED = 0;
    
    //startup sequence
    whitePalette();
    FillLEDsFromPaletteColors(startIndex);
    FastLED.show();
    delay(500);
    randomizePalette();
    FillLEDsFromPaletteColors(startIndex);
    FastLED.show();

    //*********************************DEBUG
    Serial.begin(9600);
    Serial.println("Begin.");
}

void loop()
{
    s1Val = digitalRead(PIN_S1);
    
    //dial is rotating
    if (s1Val != s1Last)
    {   
        // rotating clockwise if S1 = S2
        if (digitalRead(PIN_S2) == s1Val) 
        { 
            totalPos++;
        } 
        
        //otherwise rotating counterclockwise
        else
        {
            totalPos--;
        }

        //*********************************DEBUG
        Serial.print("p ");Serial.println(totalPos);
        Serial.print("i ");Serial.println(startIndex);
    } 
    
    s1Last = s1Val;
    
    //move array with encoder position
    startIndex = totalPos*8;
    FillLEDsFromPaletteColors(startIndex);
    
    //take the inverse of the digital value to see if it's pressed
    // - that is, check whether the pin is grounded
    keyDown = !digitalRead(PIN_KEY);
    
    //button is pressed
    if (keyDown)
    {
        keyTime += 1;
    }
    else
    {
        keyTop = keyTime;
        keyTime = 0;
    }
    
    //on long press, reset
    if (keyTop > 200)
    {
        software_Reset();
    }
    //on short press, freeze an LED at the end of the row
    else if (keyTop > 5)
    {
        frozenLED++;
    }
    
    //if all LEDs are frozen, "win"
    if (frozenLED >=8)
    {
        CRGB black = CRGB::Black;
        CRGB white = CRGB::White;
        
        currentPalette = CRGBPalette16( white, black, white, black,
                                        white, black, white, black,
                                        white, black, white, black,
                                        white, black, white, black);
        FillLEDsFromPaletteColors(0);
        FastLED.show();
        delay(1000);
        software_Reset();
    }
    
    //ensure that no leds are hidden, delay to reduce jitter
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    //redraw LEDs, ignoring the frozen ones
    for(int i = 0; i < NUM_LEDS - frozenLED; i++) 
    {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, BRIGHTNESS, currentBlending);
        colorIndex += 16;
    }
}

void whitePalette()
{
    for( int i = 0; i < 16; i++) 
    {
        currentPalette[i] = CHSV(0, 0, 255);
    }
}

void randomizePalette()
{
    randomSeed(analogRead(11));
    for( int i = 0; i < 16; i++) 
    {
        currentPalette[i] = CHSV(random(255), 255, 255);
    }
}

void software_Reset()
{       
    asm volatile ("  jmp 0");  
}
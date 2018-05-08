#include <ThingSpeak.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE;

// -------------------------------------
// Simple CheerLights Pulsing NeoPixel
// -------------------------------------

const byte led7 = D7;       // built-in LED
const byte dataPin = D0;    // data pin for NeoPixel
bool toggle = 1;            // on/off toggle for Particle function ledToggle

const byte numLEDs = 1;     // Number of LEDs on NeoPixel strip
CRGB leds[numLEDs];         // Create FastLED object for NeoPixel strip

const short cheerLightsChannelNumber = 1417;
unsigned long milliseconds;

TCPClient client;

String cheerColor, cheerHSV, testColorHSV;
byte testHSV[3] = {0, 0, 0};

void setup()
{
    Serial.begin(9600);

    FastLED.addLeds<NEOPIXEL, dataPin>(leds, numLEDs);
    FastLED.clear();

    ThingSpeak.begin(client);

    pinMode(led7, OUTPUT);

    // Initialize Particle functions and variables
    Particle.function("ledToggle", ledToggle);      // Turns LED on/off
    Particle.function("testColor", testColor);      // Sends test color to setColor function
    Particle.function("setTestHue", setTestHue);    // Sets test color hue
    Particle.function("setTestSat", setTestSat);    // Sets test color saturation
    Particle.function("setTestVal", setTestVal);    // Sets test color brightness value

    Particle.variable("cheerColor", cheerColor);
    Particle.variable("cheerHSV", cheerHSV);
    Particle.variable("testColorHSV", testColorHSV);
}

void loop()
{
    if (toggle == 1) {
        Serial.print(F("Requesting API..."));
        milliseconds = millis();

        cheerColor = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);

        Serial.print(F(" Received response in "));
        Serial.print(F(millis() - milliseconds));
        Serial.println(F(" ms"));

        setColor(cheerColor);

    } else if (toggle == 0) {
        setColor("none");
    }
}

String colorName[] = {"none","red","pink","green","blue","cyan","white","warmwhite","oldlace","purple","magenta","yellow","orange","test"};

// Map of HSV values for each of the CheerLight color names
byte colorsHSV[][3] = {
                          0,   0,   0,  // "none"
                          0, 255, 255,  // "red"
                        245, 192, 255,  // "pink"
                         96, 255, 255,  // "green"
                        160, 255, 255,  // "blue"
                        121, 255, 255,  // "cyan"
                          0,   0, 255,  // "white"
                         16,  90, 255,  // "warmwhite"
                         29, 160, 255,  // "oldlace"
                        200, 255, 255,  // "purple"
                        215, 223, 255,  // "magenta"
                         40, 255, 255,  // "yellow"
                         20, 255, 255,  // "orange"
                          0,   0, 255   // "test"
};

int8_t setColor(String color)
{
    for (int i = 0; i <= 13; i++) {
        if (color == colorName[i]) {
            // Create HSV color object for FastLED
            CHSV hsv(colorsHSV[i][0], colorsHSV[i][1], colorsHSV[i][2]);
            milliseconds = millis();            // start time for LED pulse
            // LED pulsing sin wave function
            short deg = 270;                    // Starting point in degrees (90 = led high, 270 = led low)
            byte numCycles = 5;                 // Change this value for number of pulse cycles
            uint end = numCycles * 360 + deg;   // Ending point in degrees
            for (deg; deg < end; deg++) {
                int8_t wave = sin(deg * 0.0174533) * 100;
                short val = map(wave, -100, 100, -1, colorsHSV[i][2]);
                hsv.val = constrain(val, 0, 255);
                leds[0] = hsv;
                FastLED.show();
                delay(30);
            }
            // Print cycle time
            Serial.print(F("LED cycle time: "));
            Serial.print(F(millis() - milliseconds));
            Serial.println(F(" ms"));
            // Print total uptime
            Serial.print(F("Total uptime: "));
            Serial.print(F(millis()));
            Serial.println(F(" ms"));

            // Set string for Particle variable cheerHSV
            if (color != "test") {
                char col[20];
                sprintf(col, "h: %d|s: %d|v: %d", colorsHSV[i][0], colorsHSV[i][1], colorsHSV[i][2]);
                cheerHSV = String(col);
            }
            return i;
        }
    }
    return -1;
}

// PARTICLE FUNCTIONS //

int8_t ledToggle(String command)
{
    if (command == "on") {
        digitalWrite(led7, LOW);
        setColor(cheerColor);
        toggle = 1;
        return toggle;
    }
    else if (command == "off") {
        digitalWrite(led7, HIGH);
        toggle = 0;
        return toggle;
    }
    else {
        return -1;
    }
}

byte testColor(String color)
{
    // Set string for Particle variable testColorHSV
    char col[20];
    sprintf(col, "h: %d|s: %d|v: %d", testHSV[0], testHSV[1], testHSV[2]);
    testColorHSV = String(col);

    // Set values for test color in colors array
    colorsHSV[13][0] = testHSV[0];
    colorsHSV[13][1] = testHSV[1];
    colorsHSV[13][2] = testHSV[2];
    return setColor(color);
}

// Individually set hue, sat, and val for test color
byte setTestHue(String hue)
{
    testHSV[0] = hue.toInt();
    return testHSV[0];
}

byte setTestSat(String sat)
{
    testHSV[1] = sat.toInt();
    return testHSV[1];
}

byte setTestVal(String val)
{
    testHSV[2] = val.toInt();
    return testHSV[2];
}

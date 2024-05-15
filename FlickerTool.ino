/* by Matt Gaidica, PhD | Neurotech Hub */
// analogOut() operates at 1.831kHz

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const String VERSION = "v1.0";
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define BUTTON_TOP 9
#define BUTTON_MIDDLE 6
#define BUTTON_BOTTOM 5
#define KNOB_HZ A5
#define KNOB_LUM A4
#define LED 10
#define PWM_FREQ 1000

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int range_hz[2] = { 5, 30 };
const int range_lum[2] = { 0, 100 };

void setup() {
  pinMode(BUTTON_TOP, INPUT_PULLUP);
  pinMode(BUTTON_MIDDLE, INPUT_PULLUP);
  pinMode(BUTTON_BOTTOM, INPUT_PULLUP);
  pinMode(KNOB_HZ, INPUT);
  pinMode(KNOB_LUM, INPUT);
  pinMode(LED, OUTPUT);
  analogWrite(LED, 0);       // off
  analogReadResolution(12);  // Set ADC resolution to 10 bits (0-1023)

  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println("Flicker Toolbox " + VERSION);
  display.println("");

  // display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println("Neurotech Hub");
  display.println("gaidica@wustl.edu");
  display.display();
  delay(500);
  display.clearDisplay();
}

int last_scaled_hz = -1;
int last_scaled_lum = -1;
bool isTiming = false;

int scaled_hz;
int scaled_lum;
int timerSec = 0;
long timerStart = 0;
int countdown;
bool ledState = LOW;
unsigned long ledToggleTime;

void loop() {
  if (!isTiming) {  // change settings
    int hz = analogRead(KNOB_HZ);
    int lum = analogRead(KNOB_LUM);

    scaled_hz = map(hz, 0, 4095, range_hz[0], range_hz[1]);
    scaled_lum = map(lum, 0, 4095, range_lum[0], range_lum[1]);

    if (scaled_hz != last_scaled_hz || scaled_lum != last_scaled_lum) {
      last_scaled_hz = scaled_hz;
      last_scaled_lum = scaled_lum;
      updateMain(0, scaled_hz, scaled_lum);
    }

    if (!digitalRead(BUTTON_TOP) || !digitalRead(BUTTON_BOTTOM)) {
      if (!digitalRead(BUTTON_BOTTOM)) {
        timerSec = 30;
      } else {
        timerSec = 60;
      }
      isTiming = true;
      timerStart = millis();
      ledToggleTime = millis();
    }
  } else {  // countdown
    // toggle LED (Hz)
    unsigned long toggleInterval = 1000 / scaled_hz / 2;
    // Check if the toggle interval has passed for the LED
    if (millis() - ledToggleTime >= toggleInterval) {
      ledToggleTime += toggleInterval;  // Update the LED toggle time
      ledState = !ledState;             // Toggle state
      Serial.println("toggling");
      if (ledState) {
        analogWrite(LED, map(scaled_lum, 0, 100, 0, 255)); // on
      } else {
        analogWrite(LED, 0); // off
      }
    }

    // second timer
    if (millis() - timerStart >= 1000) {
      timerStart += 1000;  // Update timerStart to the next second interval

      // Decrement the countdown value
      if (timerSec > 0) {
        updateMain(timerSec, scaled_hz, scaled_lum);
        timerSec--;
      } else {
        updateMain(0, scaled_hz, scaled_lum);
        isTiming = false;
        analogWrite(LED, 0);  // off
      }
    }
  }
}

void updateMain(int srem, int hz, int lum) {
  const int row0 = 4;
  const int row1 = 22;
  const int col0 = 0;
  const int col1 = 44;
  const int col2 = 92;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(col0, row0);
  display.println(srem);
  display.setTextSize(1);
  display.setCursor(col0, row1);
  display.print("s.rem");

  display.setTextSize(2);
  display.setCursor(col1, row0);
  display.println(hz);
  display.setCursor(col1, row1);
  display.setTextSize(1);
  display.print("Hz");

  display.setTextSize(2);
  display.setCursor(col2, row0);
  display.println(lum);
  display.setTextSize(1);
  display.setCursor(col2, row1);
  display.print("%lum");

  display.display();
}
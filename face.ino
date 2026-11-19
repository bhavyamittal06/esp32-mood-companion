// face.ino - desk companion for ESP32 + SSD1306 OLED (128x64, I2C) + buzzer
//
// Wiring (same in Wokwi and on real hardware):
//   OLED  GND -> GND      OLED  VCC -> 3V3
//   OLED  SCL -> GPIO 22  OLED  SDA -> GPIO 21
//   Buzzer (-) -> GND     Buzzer (+) -> GPIO 18
//
// Libraries (Wokwi: Library Manager "+" tab):
//   "Adafruit SSD1306", "Adafruit GFX Library"
//
// Control: send a word over Serial (115200 baud), ending with newline:
//   happy | sad | angry | sleepy | excited | neutral

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_ADDR 0x3C
#define BUZZER_PIN 18

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

enum Mood { HAPPY, SAD, ANGRY, SLEEPY, EXCITED, NEUTRAL };
Mood mood = NEUTRAL;

unsigned long lastBlink = 0;
unsigned long nextBlinkAt = 3000;

// ---------- sounds ----------
void beep(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms + 20);
}

void playMoodSound(Mood m) {
  switch (m) {
    case HAPPY:   beep(660, 90); beep(880, 120); break;
    case SAD:     beep(440, 150); beep(330, 250); break;
    case ANGRY:   beep(200, 120); beep(200, 120); beep(150, 250); break;
    case SLEEPY:  beep(300, 200); beep(250, 300); break;
    case EXCITED: beep(700, 60); beep(900, 60); beep(1100, 60); beep(1400, 100); break;
    case NEUTRAL: beep(500, 80); break;
  }
}

// ---------- drawing ----------
const int EYE_Y = 24;
const int LEFT_X = 38;
const int RIGHT_X = 90;

void eyesOpen(int r) {
  display.fillCircle(LEFT_X, EYE_Y, r, SSD1306_WHITE);
  display.fillCircle(RIGHT_X, EYE_Y, r, SSD1306_WHITE);
}

void eyesClosed() {
  display.fillRect(LEFT_X - 10, EYE_Y, 20, 3, SSD1306_WHITE);
  display.fillRect(RIGHT_X - 10, EYE_Y, 20, 3, SSD1306_WHITE);
}

void drawFace(Mood m, bool blink) {
  display.clearDisplay();

  switch (m) {
    case HAPPY:
      if (blink) eyesClosed(); else eyesOpen(8);
      // smile = bottom half of a circle
      display.drawCircleHelper(64, 40, 14, 0x0C, SSD1306_WHITE);
      display.drawCircleHelper(64, 41, 14, 0x0C, SSD1306_WHITE);
      break;

    case SAD:
      if (blink) eyesClosed(); else eyesOpen(7);
      // frown = top half of a circle, placed lower
      display.drawCircleHelper(64, 56, 12, 0x03, SSD1306_WHITE);
      display.drawCircleHelper(64, 57, 12, 0x03, SSD1306_WHITE);
      // tear
      display.fillCircle(LEFT_X + 8, EYE_Y + 14, 2, SSD1306_WHITE);
      break;

    case ANGRY:
      eyesOpen(6);
      // slanted eyebrows
      display.drawLine(LEFT_X - 12, EYE_Y - 14, LEFT_X + 10, EYE_Y - 7, SSD1306_WHITE);
      display.drawLine(LEFT_X - 12, EYE_Y - 13, LEFT_X + 10, EYE_Y - 6, SSD1306_WHITE);
      display.drawLine(RIGHT_X + 12, EYE_Y - 14, RIGHT_X - 10, EYE_Y - 7, SSD1306_WHITE);
      display.drawLine(RIGHT_X + 12, EYE_Y - 13, RIGHT_X - 10, EYE_Y - 6, SSD1306_WHITE);
      // flat mouth
      display.fillRect(48, 50, 32, 3, SSD1306_WHITE);
      break;

    case SLEEPY:
      eyesClosed();
      display.drawCircle(64, 50, 4, SSD1306_WHITE);   // small "o" mouth
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(108, 4);  display.print("z");
      display.setCursor(114, 12); display.print("Z");
      break;

    case EXCITED:
      if (blink) eyesClosed(); else eyesOpen(11);
      // pupils (black dots inside the white eyes)
      if (!blink) {
        display.fillCircle(LEFT_X, EYE_Y, 4, SSD1306_BLACK);
        display.fillCircle(RIGHT_X, EYE_Y, 4, SSD1306_BLACK);
      }
      // open mouth
      display.fillRoundRect(52, 44, 24, 16, 6, SSD1306_WHITE);
      break;

    case NEUTRAL:
      if (blink) eyesClosed(); else eyesOpen(7);
      display.fillRect(52, 50, 24, 3, SSD1306_WHITE);
      break;
  }

  display.display();
}

// ---------- serial input ----------
bool parseMood(String s, Mood &out) {
  s.trim();
  s.toLowerCase();
  if (s == "happy")   { out = HAPPY;   return true; }
  if (s == "sad")     { out = SAD;     return true; }
  if (s == "angry")   { out = ANGRY;   return true; }
  if (s == "sleepy")  { out = SLEEPY;  return true; }
  if (s == "excited") { out = EXCITED; return true; }
  if (s == "neutral") { out = NEUTRAL; return true; }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin(21, 22);  // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found - check wiring / address");
    while (true) delay(1000);
  }
  Serial.println("Companion ready. Type: happy, sad, angry, sleepy, excited, neutral");
  drawFace(mood, false);
}

void loop() {
  // 1) listen for a new mood
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    Mood m;
    if (parseMood(line, m)) {
      mood = m;
      Serial.print("mood -> "); Serial.println(line);
      drawFace(mood, false);
      playMoodSound(mood);
    } else if (line.length() > 0) {
      Serial.println("unknown mood");
    }
  }

  // 2) blink now and then
  unsigned long now = millis();
  if (now - lastBlink > nextBlinkAt) {
    drawFace(mood, true);
    delay(120);
    drawFace(mood, false);
    lastBlink = millis();
    nextBlinkAt = 2500 + random(0, 3000);
  }
}

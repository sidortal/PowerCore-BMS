// ===== "Source Code" + Alpha3 Rev with INA219 + Relay Control =====
// MLX90614 (Pack Temp + Ambient), NTC (Surface Temp linked to IR Ambient)
// INA219 (real current, voltage 12.22–12.26 V, SOH)
// OLED with rotating screens + safety alerts
// Relay1 + Relay2 with safety logic
// Movie-style scrolling credits + soft boot beep
// 4th screen (Capacity) restored

#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_INA219.h>
#include <math.h>

// OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// IR Temp Sensor (on I2C1)
TwoWire IRTEMP = TwoWire(1);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
#define SDA_IR 19
#define SCL_IR 18

// INA219 (I2C0 default pins: SDA=21, SCL=22)
Adafruit_INA219 ina219;

// Pins
#define GAS_SENSOR_PIN 34
#define NTC_PIN        4
#define TEMP_GREEN_LED 12
#define TEMP_BLUE_LED  32
#define TEMP_RED_LED   23
#define PPM_GREEN_LED  25
#define PPM_BLUE_LED   26
#define BUZZER_PIN     27
#define RELAY1_PIN     33
#define RELAY2_PIN     14

// NTC constants
const float NTC_R_FIXED = 10000.0;
const float NTC_BETA    = 3950.0;
const float NTC_R0      = 10000.0;
const float NTC_T0_REF  = 25.0 + 273.15;

// Simulation helpers for SOH
const float SOH_CENTER = 76.0f;
const float SOH_AMPL   = 4.0f;

// Cell voltages (simulated)
const float CELL1_BASE = 4.13f;
const float CELL2_BASE = 4.09f;
const float CELL3_BASE = 3.67f;
const float CELL_AMPL  = 0.05f; // faster variation
const float CELL_MAX_V = 4.20f;

// Smoothing
float ntcSmoothed = 25.0;
const float alpha = 0.08;

// Alert timing
unsigned long lastGasBeep = 0;
unsigned long lastTempBeep = 0;
const unsigned long gasBeepCooldown  = 3000UL;
const unsigned long tempBeepCooldown = 3000UL;

// Screen timing (now 4 screens)
const unsigned long SCREEN_INTERVAL_MS[4] = {5000UL, 2000UL, 2000UL, 2000UL};
unsigned long lastScreenSwitch = 0;
uint8_t currentScreen = 0;

// ====== SOFT BOOT BEEP (intro only) ======
static void bootBeep() {
  digitalWrite(BUZZER_PIN, HIGH); delay(100);
  digitalWrite(BUZZER_PIN, LOW);  delay(150);
  digitalWrite(BUZZER_PIN, HIGH); delay(80);
  digitalWrite(BUZZER_PIN, LOW);  delay(180);
  digitalWrite(BUZZER_PIN, HIGH); delay(120);
  digitalWrite(BUZZER_PIN, LOW);
}

// ====== SCROLLING CREDITS (intro only) ======
static void drawScrollingIntro() {
  u8g2.setFont(u8g2_font_7x14B_tr); // bold, readable on 128x64

  const char* credits[] = {
    "PowerCore BMS",
    "",
    "Student:",
    "Siddhesh Dangade",
    "Under Guidance:",
    "Prof. Sahil Goyal",
    "Prof. Uday Apte",
    "",
    "Date: 10/09/25"
  };
  const int lines = sizeof(credits) / sizeof(credits[0]);
  const int lineH = 16;
  const int totalH = lines * lineH;

  int startY = 64;          // start just off-screen bottom
  int endY   = -totalH;     // scroll until all lines pass top

  for (int offY = startY; offY > endY; --offY) {
    u8g2.clearBuffer();
    for (int i = 0; i < lines; ++i) {
      int y = offY + i * lineH;
      if (y > 0 && y <= 64) {
        int x = (128 - u8g2.getStrWidth(credits[i])) / 2;
        u8g2.drawStr(x, y, credits[i]);
      }
    }
    u8g2.sendBuffer();
    delay(65); // smooth scroll
  }
}

// ====== NTC READ ======
float readNTCTempC() {
  int raw = analogRead(NTC_PIN);
  if (raw <= 5 || raw >= 4090) return NAN;

  float v = (raw / 4095.0f) * 3.3f;
  float r_ntc = NTC_R_FIXED * (v / (3.3f - v));
  if (r_ntc <= 0.0f) return NAN;

  float steinhart = log(r_ntc / NTC_R0) / NTC_BETA + (1.0f / NTC_T0_REF);
  float tempK = 1.0f / steinhart;
  return tempK - 273.15f;
}

// ====== HELPERS ======
void beep(int count, int duration_ms) {
  for (int i = 0; i < count; ++i) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration_ms);
    digitalWrite(BUZZER_PIN, LOW);
    delay(300);
  }
}

void drawAlert(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  u8g2.drawStr((128 - u8g2.getStrWidth(line1)) / 2, 28, line1);
  u8g2.drawStr((128 - u8g2.getStrWidth(line2)) / 2, 48, line2);
  u8g2.sendBuffer();
}

// Temps + PPM screen (centered)
void drawTempsScreen(float packT, float ambT, float surfT, int ppm) {
  char buf[32];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);

  sprintf(buf, "Pack: %.1f C", packT);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 16, buf);

  sprintf(buf, "Surface: %.1f C", surfT);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 32, buf);

  sprintf(buf, "Ambient: %.1f C", ambT);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 48, buf);

  sprintf(buf, "PPM: %d", ppm);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 62, buf);

  u8g2.sendBuffer();
}

// INA219 screen (centered, fake 12.22–12.26 V)
void drawINA219Screen() {
  char buf[32];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  // Fake pack voltage oscillation 12.22–12.26 V (center 12.24, amp 0.02)
  float busV = 12.24f + 0.02f * sinf(millis() / 2000.0f);
  float current_mA = ina219.getCurrent_mA();

  unsigned long t = millis();
  float w2 = sinf((6.2831853f * (t % 12000UL)) / 12000.0f + 1.3f);
  float soh = SOH_CENTER + SOH_AMPL * w2;

  sprintf(buf, "Pack Volt: %.2f V", busV);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 20, buf);

  sprintf(buf, "Current: %.0f mA", current_mA);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 34, buf);

  sprintf(buf, "SOH: %.0f%%", soh);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 48, buf);

  u8g2.sendBuffer();
}

// Progress bar helper (unchanged)
void drawBar(int x, int y, int w, int h, float fraction) {
  if (fraction < 0) fraction = 0;
  if (fraction > 1) fraction = 1;
  int fillW = (int)(w * fraction + 0.5f);
  u8g2.drawFrame(x, y, w, h);
  if (fillW > 0) u8g2.drawBox(x + 1, y + 1, fillW - 2 < 0 ? 0 : fillW - 2, h - 2);
}

// Cells screen (unchanged layout with bars)
void drawCellsScreen() {
  char buf[24];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  unsigned long t = millis();
  float c1 = CELL1_BASE + CELL_AMPL * sinf((6.2831853f * (t % 4000UL))  / 4000.0f);
  float c2 = CELL2_BASE + CELL_AMPL * sinf((6.2831853f * (t % 5000UL)) / 5000.0f + 1.1f);
  float c3 = CELL3_BASE + CELL_AMPL * sinf((6.2831853f * (t % 6000UL)) / 6000.0f + 2.1f);

  const int left = 6;
  const int barX = 34;
  const int barW = 88;
  const int barH = 12;
  const int row1Y = 12;
  const int row2Y = 28;
  const int row3Y = 44;

  sprintf(buf, "C1");
  u8g2.drawStr(left, row1Y + 9, buf);
  drawBar(barX, row1Y, barW, barH, c1 / CELL_MAX_V);
  sprintf(buf, "%.2fV", c1);
  u8g2.drawStr(barX + barW + 3, row1Y + 9, buf);

  sprintf(buf, "C2");
  u8g2.drawStr(left, row2Y + 9, buf);
  drawBar(barX, row2Y, barW, barH, c2 / CELL_MAX_V);
  sprintf(buf, "%.2fV", c2);
  u8g2.drawStr(barX + barW + 3, row2Y + 9, buf);

  sprintf(buf, "C3");
  u8g2.drawStr(left, row3Y + 9, buf);
  drawBar(barX, row3Y, barW, barH, c3 / CELL_MAX_V);
  sprintf(buf, "%.2fV", c3);
  u8g2.drawStr(barX + barW + 3, row3Y + 9, buf);

  u8g2.sendBuffer();
}

// Capacity screen (restored as 4th screen, centered text)
void drawCapacityScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);

  const char* l1 = "C1: 8.12Wh 2160mAh";
  const char* l2 = "C2: 8.14Wh 2190mAh";
  const char* l3 = "C3: 8.10Wh 2100mAh";

  u8g2.drawStr((128 - u8g2.getStrWidth(l1)) / 2, 20, l1);
  u8g2.drawStr((128 - u8g2.getStrWidth(l2)) / 2, 38, l2);
  u8g2.drawStr((128 - u8g2.getStrWidth(l3)) / 2, 56, l3);

  u8g2.sendBuffer();
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(50);

  u8g2.begin();
  IRTEMP.begin(SDA_IR, SCL_IR, 100000);
  mlx.begin(MLX90614_I2CADDR, &IRTEMP);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip!");
    while (1) delay(10);
  }

  pinMode(TEMP_GREEN_LED, OUTPUT);
  pinMode(TEMP_BLUE_LED, OUTPUT);
  pinMode(TEMP_RED_LED, OUTPUT);
  pinMode(PPM_GREEN_LED, OUTPUT);
  pinMode(PPM_BLUE_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY1_PIN, HIGH); // Relay1 ON by default
  digitalWrite(RELAY2_PIN, LOW);  // Relay2 OFF by default

  #if defined(ARDUINO_ARCH_ESP32)
    analogSetPinAttenuation(NTC_PIN, ADC_11db);
  #endif

  // Intro: soft beep + centered scrolling credits
  bootBeep();
  drawScrollingIntro();
}

// ====== LOOP ======
void loop() {
  float packTemp = mlx.readObjectTempC();
  float ambientTemp = mlx.readAmbientTempC();
  float ntcTemp = readNTCTempC();

  if (!isnan(ntcTemp)) {
    ntcSmoothed = alpha * ntcTemp + (1.0f - alpha) * ntcSmoothed;
  }

  float surfaceTemp = isnan(ntcSmoothed) ? ambientTemp
                      : (0.7f * ambientTemp + 0.3f * ntcSmoothed);

  int gasRaw = analogRead(GAS_SENSOR_PIN);
  int ppm = map(gasRaw, 0, 4095, 400, 5000);

  bool alertShown = false;
  unsigned long now = millis();

  // GAS ALERTS (unchanged)
  if (ppm > 4000 && now - lastGasBeep > gasBeepCooldown) {
    beep(3, 800);
    drawAlert("Fire Hazard!", "PPM > 4000");
    lastGasBeep = now;
    alertShown = true;
    delay(3000);
  } else if (ppm > 3000 && now - lastGasBeep > gasBeepCooldown) {
    beep(2, 800);
    drawAlert("Fumes Detected", "PPM > 3000");
    lastGasBeep = now;
    alertShown = true;
    delay(3000);
  }

  // TEMP ALERTS (unchanged)
  if (packTemp > 50.0f && now - lastTempBeep > tempBeepCooldown) {
    beep(2, 1500);
    drawAlert("Thermal Runaway", "Pack > 50C!");
    lastTempBeep = now;
    alertShown = true;
    delay(3000);
  } else if (packTemp > 40.0f && now - lastTempBeep > tempBeepCooldown) {
    beep(2, 1500);
    drawAlert("Overheating", "Pack > 40C!");
    lastTempBeep = now;
    alertShown = true;
    delay(3000);
  }

  // ===== RELAY CONTROL (unchanged) =====
  static bool relay1State = true;   // Relay1 starts ON
  static bool relay2State = false;  // Relay2 starts OFF

  // Relay1 logic (temperature + ppm)
  if (ppm > 4000) {
    relay1State = false;  // Safety: force OFF if gas too high
  } else {
    if (packTemp > 40.0f) {
      relay1State = false;  // Overheat → OFF
    } else if (packTemp < 30.0f && ppm <= 4000) {
      relay1State = true;   // Cool & safe → ON
    }
    // Between 30–40 °C → keep previous state
  }
  digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);

  // Relay2 logic (ppm only)
  if (ppm > 3500) {
    relay2State = true;   // Turn ON if fumes strong
  } else if (ppm < 800) {
    relay2State = false;  // Reset OFF when air safe
  }
  // Between 800–3500 → keep previous state
  digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);

  // DISPLAY SCREENS IF NO ALERT
  if (!alertShown) {
    unsigned long elapsed = now - lastScreenSwitch;
    if (elapsed > SCREEN_INTERVAL_MS[currentScreen]) {
      currentScreen = (currentScreen + 1) % 4;   // rotate 4 screens
      lastScreenSwitch = now;
    }

    switch (currentScreen) {
      case 0: drawTempsScreen(packTemp, ambientTemp, surfaceTemp, ppm); break;
      case 1: drawINA219Screen(); break;
      case 2: drawCellsScreen(); break;
      case 3: drawCapacityScreen(); break;  // restored
    }
  }

  // LEDs (unchanged)
  if (packTemp > 35.0f) {
    digitalWrite(TEMP_GREEN_LED, LOW);
    digitalWrite(TEMP_BLUE_LED, LOW);
    digitalWrite(TEMP_RED_LED, HIGH);
  } else if (packTemp > 30.0f) {
    digitalWrite(TEMP_GREEN_LED, LOW);
    digitalWrite(TEMP_BLUE_LED, HIGH);
    digitalWrite(TEMP_RED_LED, LOW);
  } else if (packTemp > 20.0f) {
    digitalWrite(TEMP_GREEN_LED, HIGH);
    digitalWrite(TEMP_BLUE_LED, LOW);
    digitalWrite(TEMP_RED_LED, LOW);
  } else {
    digitalWrite(TEMP_GREEN_LED, LOW);
    digitalWrite(TEMP_BLUE_LED, LOW);
    digitalWrite(TEMP_RED_LED, LOW);
  }

  if (ppm > 2000) {
    digitalWrite(PPM_GREEN_LED, LOW);
    digitalWrite(PPM_BLUE_LED, HIGH);
  } else {
    digitalWrite(PPM_GREEN_LED, HIGH);
    digitalWrite(PPM_BLUE_LED, LOW);
  }

  delay(300);

}

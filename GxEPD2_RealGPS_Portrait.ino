#include <GxEPD2_BW.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeMono9pt7b.h> // Serifenlose Schriftart für Datum und Uhrzeit
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// GPS und Display Konfigurationen
static const uint32_t GPSBaud = 9600;
static const int UTC_OFFSET = 2; // MESZ ist UTC+2

TinyGPSPlus gps; // TinyGPSPlus Objekt
HardwareSerial ss(1); // Serielle Verbindung zum GPS Gerät

// Definition für das Waveshare 2.9" V2 Display
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> display(GxEPD2_290_T94_V2(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

// Startwerte für SOG und HDG
float sog = 2.0;
double hdg = 20.0;

void setup() {
  initEPaperDisplay();
  Serial.begin(115200);
  ss.begin(GPSBaud, SERIAL_8N1, 33, 32);
  Serial.println(F("Hallo. Serial initialisiert."));
}

void loop() {
  updateGPSValues();
  GPSSerialDump();
  smartDelay(1000); // Wartet 1 Sekunde
}

void GPSSerialDump() {
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

void initEPaperDisplay() {
  display.init(115200, true, 2, false);
  display.setRotation(0); // Setzt die Rotation des Displays auf Hochformat
  display.setFont(&FreeSans12pt7b); // Setzt die Schriftart für den statischen Text
  display.setTextColor(GxEPD_BLACK); // Setzt die Textfarbe auf Schwarz
  display.setFullWindow();
  display.firstPage();
  do {
    drawStaticElements();
  } while (display.nextPage());
}

void drawStaticElements() {
  display.fillScreen(GxEPD_WHITE); // Füllt den Bildschirm mit Weiß
  display.setFont(&FreeSans12pt7b); // Kleinere Schriftart für statische Bezeichnungen
  display.setCursor(10, 40); // Position für die erste Zeile (SOG)
  display.print("SOG (kn)");
  display.setCursor(10, 160); // Position für die dritte Zeile (HDG)
  display.print("HDG (dg)");
}

void updateGPSValues() {
  if (gps.speed.isValid()) {
    sog = gps.speed.knots();
    hdg = gps.course.deg();
  } else {
    sog = 0;
    hdg = 0;
  }

  updateDisplay();
}

void updateDisplay() {
  updateSOG();
  updateHDG();
  updateDateTime();
}

void updateSOG() {
  char sogBuffer[10];
  snprintf(sogBuffer, sizeof(sogBuffer), "%04.1f", sog); // 5 Stellen insgesamt: 4 vor dem Dezimalpunkt, 1 Dezimalstelle

  int16_t tbx, tby;
  uint16_t tbw, tbh;

  display.setFont(&FreeSansBold24pt7b); // Größere Schriftart für SOG-Zahl
  display.getTextBounds(sogBuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = 10; // X-Position für SOG-Zahl
  uint16_t y = 100; // Y-Position für SOG-Zahl
  uint16_t w = tbw + 10; // Breite des aktualisierten Bereichs
  uint16_t h = tbh + 5; // Höhe des aktualisierten Bereichs
  display.setPartialWindow(x, y - tbh, w, h); // Setzt das Fenster auf den Bereich, der aktualisiert werden soll

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // Füllt den festgelegten Bereich mit Weiß
    display.setCursor(x, y);
    display.print(sogBuffer); // Gibt die SOG-Zahl aus
  } while (display.nextPage());
}

void updateHDG() {
  char hdgBuffer[10];
  snprintf(hdgBuffer, sizeof(hdgBuffer), "%03d", (int)hdg); // 3 Stellen insgesamt: führende Nullen, falls nötig

  int16_t tbx, tby;
  uint16_t tbw, tbh;

  display.setFont(&FreeSansBold24pt7b); // Größere Schriftart für HDG-Zahl
  display.getTextBounds(hdgBuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = 10; // X-Position für HDG-Zahl
  uint16_t y = 220; // Y-Position für HDG-Zahl
  uint16_t w = tbw + 10; // Breite des aktualisierten Bereichs
  uint16_t h = tbh + 5; // Höhe des aktualisierten Bereichs
  display.setPartialWindow(x, y - tbh, w, h); // Setzt das Fenster auf den Bereich, der aktualisiert werden soll

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // Füllt den festgelegten Bereich mit Weiß
    display.setCursor(x, y);
    display.print(hdgBuffer); // Gibt die HDG-Zahl aus
  } while (display.nextPage());
}

void updateDateTime() {
  char dateBuffer[20];
  char timeBuffer[20];

  if (gps.date.isValid() && gps.time.isValid()) {
    int hour = gps.time.hour() + UTC_OFFSET;
    int dayOffset = 0;

    // Überprüfen, ob die Stunde nach Mitternacht geht
    if (hour >= 24) {
      hour -= 24;
      dayOffset = 1;
    }

    snprintf(dateBuffer, sizeof(dateBuffer), "%02d.%02d.%04d", gps.date.day() + dayOffset, gps.date.month(), gps.date.year());
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, gps.time.minute());
  } else {
    snprintf(dateBuffer, sizeof(dateBuffer), "DD.MM.JJJJ");
    snprintf(timeBuffer, sizeof(timeBuffer), "HH:MM");
  }

  int16_t tbx, tby;
  uint16_t tbw, tbh;

  // Datum
  display.setFont(&FreeMono9pt7b); // Kleinere, serifenlose Schriftart
  display.getTextBounds(dateBuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = 10; // X-Position für das Datum
  uint16_t y = 260; // Y-Position für das Datum
  uint16_t w = tbw + 10; // Breite des aktualisierten Bereichs
  uint16_t h = tbh+3; // Höhe des aktualisierten Bereichs
  display.setPartialWindow(x, y - tbh, w, h); // Setzt das Fenster auf den Bereich, der aktualisiert werden soll

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // Füllt den festgelegten Bereich mit Weiß
    display.setCursor(x, y);
    display.print(dateBuffer); // Gibt das Datum aus
  } while (display.nextPage());

  // Uhrzeit
  display.getTextBounds(timeBuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = 10; // X-Position für die Uhrzeit
  y = 280; // Y-Position für die Uhrzeit
  w = tbw + 10; // Breite des aktualisierten Bereichs
  h = tbh+3; // Höhe des aktualisierten Bereichs
  display.setPartialWindow(x, y - tbh, w, h); // Setzt das Fenster auf den Bereich, der aktualisiert werden soll

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // Füllt den festgelegten Bereich mit Weiß
    display.setCursor(x, y);
    display.print(timeBuffer); // Gibt die Uhrzeit aus
  } while (display.nextPage());
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec) {
  if (!valid) {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len) {
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t) {
  if (!d.isValid()) {
    Serial.print(F("********** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid()) {
    Serial.print(F("******** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len) {
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}

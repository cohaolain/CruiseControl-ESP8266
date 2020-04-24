#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

#include <TinyGPS++.h>
#include <U8g2lib.h>

#define DEBUG

TinyGPSPlus gps;
SoftwareSerial ss(14, -1);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/16, SCL, SDA);

const int buzzerPin = D6;
const int greenPin = D7;
const int redPin = D8;

const int numPrevReadings = 10;
const int numSpeeds = 12;
const int speeds[] = {1, 5, 10, 15, 20, 30, 45, 50, 60, 80, 100, 120};
const uint8_t *fonts[] = {u8g2_font_profont12_tf, u8g2_font_profont17_mf, u8g2_font_profont12_mf};

bool wasSpeeding;
bool deviceDisabled;

int currentSpeedLimIndex = 7;
double prevReadings[numPrevReadings];
int lastUpdate = 0;

bool pendingButtonAction;
bool greenPushed;
bool redPushed;
int lastUpdatePeriod = 0;
int lastButtonPress = 0;

bool serialConnectionStale;

void preinit()
{
    // Don't need WiFi for this project, turn it off to save power
    WiFi.preinitWiFiOff();
}

void ackBeep(bool doTwice)
{
    tone(buzzerPin, 880, doTwice ? 75 : 100);
    if (doTwice)
    {
        delay(125);
        ackBeep(false);
    }
}

void ackBeep()
{
    ackBeep(false);
}

void actionBeep(bool isNice)
{
    if (isNice)
    {
        tone(buzzerPin, 650, 75);
        delay(75);
        tone(buzzerPin, 880, 75);
    }
    else
    {
        tone(buzzerPin, 880, 75);
        delay(75);
        tone(buzzerPin, 650, 75);
    }
}

double currentSpeed()
{
    double acc = 0;
    for (double val : prevReadings)
    {
        acc += val;
    }
    return acc / numPrevReadings;
}

bool updateSpeedLim(int indexDelta)
{
    noTone(buzzerPin);
    if (currentSpeedLimIndex + indexDelta >= 0 && currentSpeedLimIndex + indexDelta < numSpeeds)
    {
        currentSpeedLimIndex += indexDelta;
        return true;
    }
    return false;
}

// Interrupt for button presses
ICACHE_RAM_ATTR void onButtonPressed()
{
    if (lastButtonPress + 150 <= millis())
    {
        greenPushed = digitalRead(greenPin);
        redPushed = digitalRead(redPin);
        pendingButtonAction = true;
    }
    lastButtonPress = millis();
}

void performPendingButtonActions()
{
    if (pendingButtonAction)
    {
        if (greenPushed && redPushed)
        {
            deviceDisabled ^= 1;
            u8g2.setPowerSave(deviceDisabled);
            actionBeep(!deviceDisabled);
        }
        else if ((greenPushed || redPushed))
        {
            ackBeep(!updateSpeedLim(greenPushed ? 1 : -1));
        }
        pendingButtonAction = false;
    }
}

bool waitForSerial()
{
    performPendingButtonActions();
    String s;
    uint waitSince = millis();
    while (!ss.available())
    {
        yield();
        if (millis() - waitSince > 1000)
        {
            serialConnectionStale = true;
            return !serialConnectionStale;
        }
    }

    do
    {
        gps.encode(ss.read());
        delay(1);
    } while (ss.available());
    serialConnectionStale = false;
    performPendingButtonActions();
    return !serialConnectionStale;
}

static char *formattedTime()
{
    char buf[16];
    TinyGPSTime t = gps.time;
    if (t.isValid())
    {
        sprintf(buf, "%02d:%02d", t.hour(), t.minute());
    }
    else
    {
        sprintf(buf, "??:??");
    }
    return buf;
}

static char *formattedSpeed()
{
    char buf[32];
    char padBuf[32];
    dtostrf(currentSpeed(), 3, 1, buf);
    sprintf(padBuf, "%5s", buf);
    return padBuf;
}

bool allPrevReadingsZero()
{
    for (double val : prevReadings)
    {
        if (val != 0)
            return false;
    }
    return true;
}

void addReading(double reading)
{
    double temp1 = prevReadings[0];
    for (int i = 1; i < numPrevReadings; i++)
    {
        double temp2 = prevReadings[i];
        prevReadings[i] = temp1;
        temp1 = temp2;
    }
    prevReadings[0] = reading;
}

void updateGPS()
{
    if (gps.speed.isUpdated())
    {
        double reading = gps.speed.kmph();
        if (reading > 3.5 && !allPrevReadingsZero())
        {
            addReading(reading);
        }
        else
        {
            addReading(0);
        }
        lastUpdatePeriod = millis() - lastUpdate;
        lastUpdate = millis();
    }
}

void printAccuracy(int x, int y)
{
    u8g2.setFont(fonts[0]);
    u8g2.setCursor(x, y);
    u8g2.print("Á");
    if (gps.hdop.isValid() && !serialConnectionStale)
    {
        u8g2.print(gps.hdop.hdop());
    }
    else
    {
        Serial.println(gps.hdop.isValid() ? "valid" : "invalid");
        Serial.println(!serialConnectionStale ? "stale" : "not stale");
        u8g2.print("...");
    }
}

void printTime()
{
    u8g2.setFont(fonts[2]);
    u8g2.drawStr(128 - 30, 8, formattedTime());
}

void printSpeed()
{
    u8g2.setFont(fonts[1]);
    u8g2.drawStr(49, 15, formattedSpeed());
    u8g2.setFont(fonts[0]);
    u8g2.setCursor(70, 28);
    u8g2.print("km/h");
}

void printLimit(int x, int y)
{
    char buf[64];
    sprintf(buf, "%d", speeds[currentSpeedLimIndex]);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(x, y - 1, 10, U8G2_DRAW_ALL);
    u8g2.setDrawColor(0);
    u8g2.drawStr(x + 1 - (6 * ((int)log10(speeds[currentSpeedLimIndex]) + 1)) / 2, y + 3, buf);
    u8g2.setDrawColor(1);
}

void tooFast()
{
    double speedingAmount = ((currentSpeed() / speeds[currentSpeedLimIndex]) - 1) * 100;
    double freq = map(speedingAmount, 0, 50, 220, 800);
    tone(buzzerPin, freq);
    wasSpeeding = true;
}

void checkSpeed()
{
    if (currentSpeed() <= speeds[currentSpeedLimIndex] && wasSpeeding)
    {
        wasSpeeding = !wasSpeeding;
        noTone(buzzerPin);
    }
    else if (currentSpeed() > speeds[currentSpeedLimIndex])
        tooFast();
}

void showNoSerial()
{
    u8g2.clearBuffer();
    u8g2.setFont(fonts[1]);
    u8g2.setCursor(6, 12);
    u8g2.print("No GPS Module");
    u8g2.setCursor(12, 28);
    u8g2.print("Attached :(");
    u8g2.sendBuffer();
}

void showInvalid()
{
    u8g2.clearBuffer();
    u8g2.setFont(fonts[1]);
    u8g2.setCursor(22, 12);
    u8g2.print("Searching");
    u8g2.setCursor(28, 28);
    u8g2.print("for GPS...");
    u8g2.sendBuffer();
}

void setup()
{
    // Serial for debugging
    Serial.begin(115200);
    Serial.println("Begin!");

    // Software serial for GPS
    ss.begin(19200);

    // Setup OLED display
    u8g2.begin();
    u8g2.setFlipMode(1);

    // Setup buttons
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(redPin), onButtonPressed, RISING);
    attachInterrupt(digitalPinToInterrupt(greenPin), onButtonPressed, RISING);
}

void loop()
{
    if (waitForSerial())
    {
        if (gps.hdop.hdop() >= 50)
        {
            showInvalid();
        }
        else
        {
            updateGPS();
            checkSpeed();

            // Render UI
            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 32);

            printAccuracy(2, 10);
            printSpeed();
            printLimit(112, 17);

            // Write UI into display buffer
            noInterrupts();
            u8g2.sendBuffer();
            interrupts();
        }
    }
    else
    {
        showNoSerial();
    }
}

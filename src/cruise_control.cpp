// Bare essentials
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

// GPS & display libraries
#include <TinyGPS++.h>
#include <U8g2lib.h>

// My libraries
#include <Speedometer.h>
#include <StartLogo.h>

#define DEBUG

const int buzzerPin = D8;
const int greenPin = D7;
const int redPin = D6;

const int numPrevReadings = 3;
const int numSpeeds = 11;
const int speeds[numSpeeds] = {5, 10, 15, 20, 25, 30, 50, 60, 80, 100, 120};
const uint8_t *fonts[] = {u8g2_font_profont12_tf, u8g2_font_profont17_mf, u8g2_font_profont12_mf};

const double safetyMultiplier = 1.05;

bool wasSpeeding;
bool deviceDisabled;

int currentSpeedLimIndex = 6;
double prevReadings[numPrevReadings];

bool pendingButtonAction;
bool greenPushed;
bool redPushed;

int lastUpdateGPS = 0;
int lastButtonPress = 0;
int lastSerialRead = 0;

TinyGPSPlus gps;
SoftwareSerial ss(14, -1);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/16, SCL, SDA);
U8G2 globalInstanceU8G2 = u8g2;

Speedometer speedometer(24, 32, 16, speeds[currentSpeedLimIndex]);
StartLogo Logo(0, 0);

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
    return (acc / numPrevReadings) * safetyMultiplier;
}

bool updateSpeedLim(int indexDelta)
{
    noTone(buzzerPin);
    if (currentSpeedLimIndex + indexDelta >= 0 && currentSpeedLimIndex + indexDelta < numSpeeds)
    {
        currentSpeedLimIndex += indexDelta;
        speedometer.updateRange(speeds[currentSpeedLimIndex]);
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
        if (reading >= 2)
            addReading(reading);
        else
            addReading(0);
        lastUpdateGPS = millis();
    }
}

bool waitForSerial()
{
    performPendingButtonActions();

    if (ss.available())
    {
        while (ss.available())
        {
            String nmeaString = ss.readStringUntil('\n');
            for (char c : nmeaString)
            {
                gps.encode(c);
            }
            Serial.println(nmeaString);
        }

        updateGPS();

        Serial.printf("$GPTXT,CC says,hdop (%s): %.2f\n", gps.hdop.isValid() ? "valid" : "invalid", gps.hdop.hdop());
        Serial.printf("$GPTXT,CC says,location (%s): %f, %f\n", gps.location.isValid() ? "valid" : "invalid", gps.location.lat(), gps.location.lng());
        Serial.printf("$GPTXT,CC says,speed (%s): %.2f km/h\n", gps.speed.isValid() ? "valid" : "invalid", gps.speed.kmph());
        Serial.printf("$GPTXT,CC says,stated speed (%s): %.2f km/h\n", gps.speed.isValid() ? "valid" : "invalid", currentSpeed());
        Serial.printf("$GPTXT,CC says,passed: %d, failed: %d, success: %.2f\%\n", gps.passedChecksum(), gps.failedChecksum(), ((double)gps.passedChecksum()) / max((uint32_t)1, (gps.passedChecksum() + gps.failedChecksum())));
        Serial.print("$GPTXT,CC says,prevReadings: ");
        for (auto val : prevReadings)
            Serial.printf("%.2f ", val);
        Serial.println("\n");

        lastSerialRead = millis();
    }

    return millis() - lastSerialRead <= 1000;
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

void printAccuracy(int x, int y)
{
    u8g2.setFont(fonts[0]);
    u8g2.setCursor(x, y);
    if (gps.hdop.isValid())
    {
        u8g2.print(gps.hdop.hdop());
    }
    else
    {
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
    int freq = min(map(speedingAmount, 0, 50, 220, 800), 800l);
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
    u8g2.setCursor(18, 28);
    u8g2.print("for GPS...");
    u8g2.sendBuffer();
}

void setup()
{
    // Serial for debugging
    Serial.begin(1843200);
    Serial.println("Begin!");

    // Software serial for GPS
    ss.begin(9600, SWSERIAL_8N1, 14, -1, false, 256);
    ss.setTimeout(10);

    // Setup OLED display
    u8g2.begin();

    // Setup buttons
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(redPin), onButtonPressed, RISING);
    attachInterrupt(digitalPinToInterrupt(greenPin), onButtonPressed, RISING);

    // Setup UI
    speedometer.updateRange(speeds[currentSpeedLimIndex]);

    // Show logo
    Logo.displayLogo(0);
    u8g2.sendBuffer();
    delay(10000);
}

void loop()
{
    if (waitForSerial())
    {
        if (gps.hdop.hdop() >= 4 || !gps.speed.isValid())
        {
            if (wasSpeeding)
                noTone(buzzerPin);
            showInvalid();
        }
        else
        {
            checkSpeed();

            // Render UI
            u8g2.clearBuffer();

            printAccuracy(2, 10);
            printSpeed();
            printLimit(112, 17);
            speedometer.draw(currentSpeed());

            // Write UI into display buffer
            u8g2.sendBuffer();
        }
    }
    else
    {
        if (wasSpeeding)
            noTone(buzzerPin);
        showNoSerial();
    }
}

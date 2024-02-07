#include <Arduino.h>

#include "paddle_config.h"

void sendStillAlive();
void sendElCmd(double speed);
void sendAzCmd(double speed);
// Create an IntervalTimer object
IntervalTimer myTimer;

double speedpotValue = 0.0;

struct pinControl
{
  pinControl(int _pinNo, uint32_t _intervalMillis = 0) : pinNo(_pinNo), updateIntervalMillis(_intervalMillis)
  {
    pinMode(pinNo, INPUT_PULLUP);
    lastEdgeTime = millis();
    // SERIAL_CH.printf("Initial Time: %d\n", lastEdgeTime);
    isUpdated = false;
  }
  int pinNo;
  uint32_t updateIntervalMillis;
  uint32_t lastEdgeTime;
  uint8_t state;
  uint8_t prevState;
  bool isUpdated;
  bool update()
  {
    uint32_t currTime = millis();
    if ((currTime - lastEdgeTime) > updateIntervalMillis)
    {
      prevState = state;
      state = digitalRead(pinNo);
      if(state != prevState)
      {
        lastEdgeTime = currTime;
        isUpdated = true;
      }
    }
    return isUpdated;
  }
  bool fallingEdge()
  {
    isUpdated = false;
    return (state == LOW && prevState == HIGH);
  }
  bool risingEdge()
  {
    isUpdated = false;
    return (state == HIGH && prevState == LOW);
  }
};

pinControl *posElBtn, *negElBtn, *posAzBtn, *negAzBtn;

void setup()
{
  // put your setup code here, to run once:
  SERIAL_CH.begin(SERIAL_BAUD);

  pinMode(LED_BUILTIN, OUTPUT);
  for (int16_t ii = 0; ii < 5; ii++)
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(100);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(300);                      // wait for a second
  }
  analogReadAveraging(16);

  posElBtn = new pinControl(POS_EL_BTN, BOUNCE_INTERVAL_MS);
  negElBtn = new pinControl(NEG_EL_BTN, BOUNCE_INTERVAL_MS);
  posAzBtn = new pinControl(POS_AZ_BTN, BOUNCE_INTERVAL_MS);
  negAzBtn = new pinControl(NEG_AZ_BTN, BOUNCE_INTERVAL_MS);

  // myTimer.begin(sendStillAlive, STILL_ALIVE_INTERVAL);
}

void loop()
{
  speedpotValue = analogRead(SPEED_POT_PIN);
  double speedVal = speedpotValue * SPEED_POT_SCALE;

  if (posElBtn->update())
  {
    if (posElBtn->fallingEdge())
    {
      sendElCmd(speedVal);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (posElBtn->risingEdge())
    {
      sendElCmd(0.0);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  if (negElBtn->update())
  {
    if (negElBtn->fallingEdge())
    {
      sendElCmd(-1 * speedVal);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (negElBtn->risingEdge())
    {
      sendElCmd(0.0);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  if (posAzBtn->update())
  {
    if (posAzBtn->fallingEdge())
    {
      sendAzCmd(speedVal);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (posAzBtn->risingEdge())
    {
      sendAzCmd(0.0);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  if (negAzBtn->update())
  {
    if (negAzBtn->fallingEdge())
    {
      sendAzCmd(-1 * speedVal);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (negAzBtn->risingEdge())
    {
      sendAzCmd(0.0);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void sendStillAlive()
{
  // send the still alive message
  SERIAL_CH.print(0xDEADBEEF);
}

void sendElCmd(double speed)
{
  // send the command
  SERIAL_CH.printf("{AZ:%6.5f}\n", speed);
}

void sendAzCmd(double speed)
{
  // send the command
  SERIAL_CH.printf("{EL:%6.5f}\n", speed);
}

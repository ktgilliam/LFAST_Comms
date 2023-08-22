/*

Software to control a tec on the controller board.

Should:
accept power commands
Manage direction control.  Set current to zero before switching direction.
Report Thermistor ADC values to serial or over Ethernet port  (this would be a good place for a web server!)

*/

#ifndef __ThermoElectricController_H
#define __ThermoElectricController_H

#include "ThermoElectricGlobal.h"

bool i2cHardwareID_init();
int get_hardware_id();
bool initialize_SPI();

const int TEC_PWM_FREQ = 50000;

class ThermoElectricController
{
public:
  ThermoElectricController();
  int begin(const int dirPin, const int pwmPin, const int minVal);

  int setDutyCycle(const float percent);
  // void setDirection( const bool direction );
  float getDutyCycle();
  bool getDirection();
  unsigned int getSeebeck(int channel);

protected:
  void setPwm(float duty);
  float pwmPct;
  bool dir;
  int dirPin;
  int pwmPin;
  int minPercent;
  int raw_data;
};

#endif

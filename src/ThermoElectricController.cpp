/*
  Implementation of the TEC control software
*/
#include "ThermoElectricController.h"
#include "ThermoElectricGlobal.h"
#include "teensy41_device.h"


// temperature for nominal resistance (almost always 25 C = 298.15 K)
#define TEMPERATURENOMINAL 298.15

static int hardware_id = -1;

ThermoElectricController::ThermoElectricController() {}

  /*! @brief     Initializes the contents of the class
    @details   Sets pin definitions, and initializes the variables of the class.
    @param[in] dirPin Defines which pin controls direction
    @param[in] pwmPin Defines which pin provides PWM pulses to the TEC
    @return    void
  */
int ThermoElectricController::begin(const int dirP, const int pwmP, const int minVal)
{
  dirPin = dirP;
  pwmPin = pwmP;
  // thermistorPin = thermistorP;
  raw_data = 0.0;
  pwmPct = 0;
  dir = 0;
  minPercent = minVal;

  // set the pins properly for this TEC
  // pinMode(dirPin, INPUT_PULLUP);   // Is the DIR pin an Input?  Or output to switch the H-Bridge?
  pinMode(dirPin, OUTPUT); // Changed the DIR pin to output so we can control the H-Bridge
  pinMode(pwmPin, OUTPUT);
  // PRINT_GOT_HERE();
  // pinMode(thermistorPin,INPUT_DISABLE);
  // set up the PWM parameters for the PWM pin.
  analogWriteFrequency(pwmPin, TEC_PWM_FREQ);
  // analogReadResolution(12);  Don't need this anymore since we're using an external ADC

  return 0;
}

void ThermoElectricController::setPwm(float duty)
{
  float scaled_power = fabs(duty) / 100 * (100 - minPercent) + minPercent;
  float tmp = (float)((scaled_power * 255.0) / 100.0 + 0.5); // convert to 0-255 )
  analogWrite(pwmPin, tmp);                                  //
}

int ThermoElectricController::setDutyCycle(const float power)
{
  //  int board_id = get_hardware_id();
  if (power > 100 || power < -100)
    return -1;
  //  //TEST_SERIAL.print("Setting Power to ");//TEST_SERIAL.println( power );
  // set direction
  if (((power < 0) && (pwmPct >= 0)) ||
      ((power > 0) && (pwmPct <= 0)))
  { // change needed
    setPwm(0);
    dir = (power < 0);
    digitalWrite(dirPin, dir);
  }
  // Convert to duty cycle
  // Set duty cycle
  setPwm(power);
  pwmPct = power; // save the power setting
  return 0;
}

// The Seebeck is now using the on-board ADC (ADS7953)
// This routine gets data for the specific channel
// The channel might be needed or it might not - stand by...
unsigned int ThermoElectricController::getSeebeck(int channel)
{
  // read the analog voltage
  unsigned int recvBits = 0;
  unsigned int adcCounts = 0;
  float save_power = pwmPct;

  setPwm(0);
  // wait a few milliseconds
  // Problem is i2c starts to fail when delay goes up to 50 msec
  delay(500); // Delay for 0.5 second.  This looks good for current going to 0

  // Get the ADC covnersion data in manual mode
  SPI.beginTransaction(SPISettings(ADC_SPI_BPS, MSBFIRST, SPI_MODE0));
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  uint16_t addr = (channel << 7);
  recvBits = SPI.transfer16((ADC_CMD_MODE_MANUAL | addr));
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH); // This address will be converted in two chipSelects

  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  recvBits = SPI.transfer16((ADC_CMD_MODE_MANUAL | addr)); // leave in manual mode, but don't worry about addr
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);

  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  recvBits = SPI.transfer16((ADC_CMD_MODE_MANUAL | addr)); // leave in manual mode, but don't worry about addr
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);
  // We now have the correct channel conversion, I think
  SPI.endTransaction();
  // Set the Pwm back to its original setting
  setPwm(save_power);

  if ((uint16_t)recvBits >> 12 == channel)
  {
    adcCounts = recvBits & 0x0FFF; //
  }
  // convert to voltage
  // float voltage = adcCounts; // * 2.5/4096.0; // Begin with Vref range (we can and may go to 2xVref range which is ADC selectable)
  return adcCounts; // voltage;  // Just the voltage, please
}

float ThermoElectricController::getDutyCycle(void)
{
  return pwmPct;
}

bool ThermoElectricController::getDirection(void)
{
  return dir;
}

// Initialized Module ID hardware
bool i2cHardwareID_init()
{
  // Wait for pin inputs to settle
  delay(100);

  // Read the jumpers.  This must only be done once, even if they produce an
    // VVV ##### What jumpers? -K
  // invalid ID, in order to ensure that they are correctly read.
  // The pins have inverted sense, so the raw values have been reversed.
  // These pins are no used on the 16 channel version of the TEC board
  int pin0 = digitalRead(I2C_ID_PIN_0) ? 0 : 1;
  int pin1 = digitalRead(I2C_ID_PIN_1) ? 0 : 1;

  hardware_id = (pin1 << 1) +
                (pin0 << 0);
  delay(100);
  DebugPrintNoEOL("Hardware ID = ");
  DebugPrint(hardware_id);

  // Make sure ID is valid
  if (hardware_id < 0 || hardware_id > MAX_BOARD_ID)
  {
    DebugPrint("invalid board ID detected, check jumpers");
    return false;
  }
  return true; // Success
}

int get_hardware_id()
{
  return hardware_id;
}

bool initialize_SPI()
{
  // uint16_t SDOchans; // Testing variable to print the channels

  SPI.begin();
  delay(1000); // wait for SPI to begin
  // Now go through the power up register programming sequence
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 10 MHz speed maximum
  // First low CS device in manual mode, channel 0, SDO invalid
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  SPI.transfer16(ADC_CMD_MODE_AUTO1);      // Device enters auto-1 program sequence
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH); // Raise the chip select
  // Second low CS device programs Auto 1 register
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  // SDOchans = SPI.transfer16(0xFFFF); // Choose channels to convert with positional 16 bit word
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);     // Raise the chip select
  //TEST_SERIAL.println(SDOchans >> 12);
  // Third low CS device programs Auto 2 register
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  // SDOchans = SPI.transfer16(0xFFFF); // Won't be using Auto-2, this is don't cares - can use the same word as Auto-1 programming
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);     // Raise the chip select
  //TEST_SERIAL.println(SDOchans >> 12);
  // Fourth low CS device programs Alram registers
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  // SDOchans = SPI.transfer16(0x0000); // We won't be using Alarms, so this is don't cares
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);    // Raise the chip select
  //TEST_SERIAL.println(SDOchans >> 12);
  // Fifth low CS device programs GPIO registers
  digitalWrite(ADC_CHIP_SELECT_PIN, LOW);
  // SDOchans = SPI.transfer16(0x0000); // We won't be using GPIO, so this is don't cares
  digitalWrite(ADC_CHIP_SELECT_PIN, HIGH);    // Raise the chip select
  //TEST_SERIAL.println(SDOchans >> 12);
  SPI.endTransaction(); // The ADC is now in manual mode and we can select the channel we want to convert
  return true;          // No return codes for SPI routine so not a true error check
}
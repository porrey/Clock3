// ***
// *** Copyright(C) 2020, Daniel M. Porrey. All rights reserved.
// ***
// *** This program is free software: you can redistribute it and/or modify
// *** it under the terms of the GNU Lesser General Public License as published
// *** by the Free Software Foundation, either version 3 of the License, or
// *** (at your option) any later version.
// ***
// *** This program is distributed in the hope that it will be useful,
// *** but WITHOUT ANY WARRANTY; without even the implied warranty of
// *** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *** GNU Lesser General Public License for more details.
// ***
// *** You should have received a copy of the GNU Lesser General Public License
// *** along with this program. If not, see http://www.gnu.org/licenses/.
// ***
#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

class BatteryMonitor
{
  public:
    // ***
    // *** Default constructor.
    // ***
    BatteryMonitor(uint8_t bits, float referenceVoltage)
    {
      this->_voltageSteps = referenceVoltage / pow(2, bits);
    };

    // ***
    // *** Initializes this instance on the
    // *** specified analog pin.
    // ***
    void begin(uint16_t pin)
    {
      this->_pin = pin;
    }

    // ***
    // *** Gets the current voltage on
    // *** the analog pin.
    // ***
    const float voltage()
    {
      // ***
      // *** Get the GPS battery voltage.
      // ***
      uint16_t value = analogRead(this->_pin);
      return this->_voltageSteps * value;
    }

    // ***
    // *** Gets the voltage per ADC increment.
    // ***
    const float voltageIncrements()
    {
      return this->_voltageSteps;
    }

  protected:
    // ***
    // *** Stores the analog pin.
    // ***
    uint16_t _pin;
    
    // ***
    // *** Holds the calculated voltage
    // *** increment applied to the value read
    // *** from the analog port.
    // ***
    float _voltageSteps;
};
#endif

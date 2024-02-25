/**
 * @file TempSensorDallas.h
 * 
 * @brief Handler for Dallas DS18B20 temperature sensor
 */

#pragma once

#include "TempSensor.h"


class TempSensorDallas : public TempSensor {
    public:
        TempSensorDallas(int GPIOPin);

        float getTempinCelsius();

    private:

};

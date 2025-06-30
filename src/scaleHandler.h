/**
 * @file scaleHandler.h
 *
 * @brief Implementation of scale initialization and weight measurement
 */

#pragma once

#include "brewStates.h"
#include "hardware/HX711Scale.h"

void displayScaleFailed();

inline bool scaleCalibrationOn = false;
inline bool scaleTareOn = false;
inline int shottimerCounter = 10;
inline float currReadingWeight = 0; // value from HX711
inline float prewBrewWeight = 0;    // value of scale before brew started
inline float currBrewWeight = 0;    // weight value of current brew
inline float scaleDelayValue = 2.5; // value in gramm that takes still flows onto the scale after brew is stopped
inline bool scaleFailure = false;

inline HX711Scale* scale = nullptr;

extern BrewState currBrewState;

inline void scaleCalibrate(int cellNumber, const int pin) {
    const int scaleSamples = config.get<int>("hardware.sensors.scale.samples");
    const auto scaleKnownWeight = ParameterRegistry::getInstance().getParameterById("hardware.sensors.scale.known_weight")->getValueAs<float>();

    HX711_ADC* loadCell = scale->getLoadCell(cellNumber);

    if (!loadCell) {
        return;
    }

    loadCell->setCalFactor(1.0);

    u8g2->clearBuffer();
    u8g2->drawStr(0, 22, "Calibration coming up");
    u8g2->drawStr(0, 32, "Empty scale ");
    u8g2->print(pin, 0);
    u8g2->sendBuffer();

    LOGF(INFO, "Taking scale %d to zero point", pin);

    loadCell->update();
    loadCell->tare();

    LOGF(INFO, "Put load on scale %d within the next 10 seconds", pin);

    u8g2->clearBuffer();
    u8g2->drawStr(2, 2, "Calibration in progress.");
    u8g2->drawStr(2, 12, "Place known weight");
    u8g2->drawStr(2, 22, "on scale in next");
    u8g2->drawStr(2, 32, "10 seconds");
    u8g2->drawStr(2, 42, number2string(scaleKnownWeight));
    u8g2->sendBuffer();
    delay(10000);

    LOG(INFO, "Taking scale load point");

    // increase scale samples temporarily to ensure a stable reading
    loadCell->setSamplesInUse(128);
    loadCell->refreshDataSet();
    float calibration = loadCell->getNewCalibration(scaleKnownWeight);
    loadCell->setSamplesInUse(scaleSamples);

    LOGF(INFO, "New calibration: %f", calibration);

    u8g2->sendBuffer();

    scale->setCalibrationFactor(calibration, cellNumber);

    // Save calibration to parameter registry
    if (cellNumber == 2) {
        ParameterRegistry::getInstance().setParameterValue("hardware.sensors.scale.calibration2", calibration);
    }
    else {
        ParameterRegistry::getInstance().setParameterValue("hardware.sensors.scale.calibration", calibration);
    }

    u8g2->clearBuffer();
    u8g2->drawStr(2, 2, "Calibration done!");
    u8g2->drawStr(2, 12, "New calibration:");
    u8g2->drawStr(2, 22, number2string(calibration));
    u8g2->sendBuffer();

    delay(2000);
}

inline float w1 = 0.0;
inline float w2 = 0.0;

inline void checkWeight() {
    if (scaleFailure || !scale) { // abort if scale is not working
        return;
    }

    // Update the scale and get current weight
    if (scale->update()) {
        currReadingWeight = scale->getWeight();
    }

    if (scaleCalibrationOn) {
        const int scaleType = config.get<int>("hardware.sensors.scale.type");

        // Calibrate first cell
        scaleCalibrate(1, PIN_HXDAT);

        // Calibrate second cell if dual scale
        if (scaleType == 0) {
            scaleCalibrate(2, PIN_HXDAT2);
        }

        scaleCalibrationOn = false;
    }

    if (scaleTareOn) {
        scaleTareOn = false;
        u8g2->clearBuffer();
        u8g2->drawStr(0, 2, "Taring scale,");
        u8g2->drawStr(0, 12, "remove any load!");
        u8g2->drawStr(0, 22, "....");
        u8g2->sendBuffer();
        delay(2000);

        scale->tare();

        u8g2->drawStr(0, 32, "done");
        u8g2->sendBuffer();
        delay(2000);
    }
}

inline void initScale() {
    const int scaleType = config.get<int>("hardware.sensors.scale.type");
    const int scaleSamples = config.get<int>("hardware.sensors.scale.samples");

    // Get calibration factors from config
    float cal1 = scaleCalibration; // These should be defined somewhere in main.cpp
    float cal2 = scale2Calibration;

    if (scaleType == 0) {          // Dual load cell
        scale = new HX711Scale(PIN_HXDAT, PIN_HXDAT2, PIN_HXCLK, cal1, cal2);
    }
    else {                         // Single load cell
        scale = new HX711Scale(PIN_HXDAT, PIN_HXCLK, cal1);
    }

    // Initialize the scale
    if (!scale->init()) {
        LOG(ERROR, "Scale initialization failed");
        displayScaleFailed();
        delay(5000);
        scaleFailure = true;
        delete scale;
        scale = nullptr;
        return;
    }

    scale->setSamples(scaleSamples);

    scaleCalibrationOn = false;
    scaleFailure = false;
}

/**
 * @brief Scale with shot timer
 */
inline void shotTimerScale() {
    switch (shottimerCounter) {
        case 10: // waiting step for brew switch turning on
            if (currBrewState != kBrewIdle) {
                prewBrewWeight = currReadingWeight;
                shottimerCounter = 20;
            }
            break;

        case 20:
            currBrewWeight = currReadingWeight - prewBrewWeight;

            if (currBrewState == kBrewIdle) {
                shottimerCounter = 10;
            }
            break;

        default:;
    }
}

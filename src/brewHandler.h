/**
 * @file brewHandler.h
 *
 * @brief Handler for brewing
 *
 */

#pragma once

#include <hardware/pinmapping.h>

enum BrewSwitchState {
    kBrewSwitchIdle = 10,
    kBrewSwitchBrew = 20,
    kBrewSwitchBrewAbort = 30,
    kBrewSwitchFlushOff = 31,
    kBrewSwitchReset = 40
};

enum BrewState {
    kBrewIdle = 10,
    kPreinfusion = 20,
    kWaitPreinfusion = 21,
    kPreinfusionPause = 30,
    kWaitPreinfusionPause = 31,
    kBrewRunning = 40,
    kWaitBrew = 41,
    kBrewFinished = 42,
    kWaitBrewOff = 43
};

enum BackflushState {
    kBackflushWaitBrewswitchOn = 10,
    kBackflushFillingStart = 20,
    kBackflushFilling = 21,
    kBackflushFlushingStart = 30,
    kBackflushFlushing = 31,
    kBackflushWaitBrewswitchOff = 43
};

// Normal Brew
BrewState currBrewState = kBrewIdle;

uint8_t currStateBrewSwitch = LOW;
uint8_t currBrewSwitchStateMomentary = LOW;
int brewSwitchState = kBrewSwitchIdle;
boolean brewSwitchWasOff = false;

double totalBrewTime = 0;                           // total brewtime set in software
double timeBrewed = 0;                              // total brewed time
double lastBrewTimeMillis = 0;                      // for shottimer delay after disarmed button
double lastBrewTime = 0 ;
unsigned long startingTime = 0;                     // start time of brew
boolean brewPIDDisabled = false;                    // is PID disabled for delay after brew has started?

// Shot timer with or without scale
#if (ONLYPIDSCALE == 1 || BREWMODE == 2)
    boolean scaleCalibrationOn = 0;
    boolean scaleTareOn = 0;
    int shottimerCounter = 10 ;
    float calibrationValue = SCALE_CALIBRATION_FACTOR;  // use calibration example to get value
    float weight = 0;                                   // value from HX711
    float weightPreBrew = 0;                            // value of scale before wrew started
    float weightBrew = 0;                               // weight value of brew
    float scaleDelayValue = 2.5;                        // value in gramm that takes still flows onto the scale after brew is stopped
    bool scaleFailure = false;
    const unsigned long intervalWeight = 200;           // weight scale
    unsigned long previousMillisScale;                  // initialisation at the end of init()
    HX711_ADC LoadCell(PIN_HXDAT, PIN_HXCLK);

    #if SCALE_TYPE == 0 
        HX711_ADC LoadCell2(PIN_HXDAT2, PIN_HXCLK);
    #endif 
#endif

/**
 * @brief Toggle or momentary input for Brew Switch
 */
void checkbrewswitch() {
    uint8_t brewSwitchReading = brewSwitch->isPressed();

    if (BREWSWITCH_TYPE == Switch::TOGGLE) {
        currStateBrewSwitch = brewSwitchReading;
    }
    else if (BREWSWITCH_TYPE == Switch::MOMENTARY) {
        if (currBrewSwitchStateMomentary != brewSwitchReading) {
            currBrewSwitchStateMomentary = brewSwitchReading;
        }
        
        // Convert momentary brew switch input to brew switch state
        switch (brewSwitchState) {
            case kBrewSwitchIdle:
                if (currBrewSwitchStateMomentary == HIGH && machineState != kWaterEmpty ) {
                    brewSwitchState = kBrewSwitchBrew;
                    LOG(DEBUG, "brewSwitchState = kBrewSwitchIdle; waiting for brew switch input");
                }
                break;

            case kBrewSwitchBrew:
                // Brew switch short pressed - start brew
                if (currBrewSwitchStateMomentary == LOW) {
                    // Brew trigger
                    currStateBrewSwitch = HIGH;
                    brewSwitchState = kBrewSwitchBrewAbort;
                    LOG(DEBUG, "brewSwitchState = kBrewSwitchBrew; brew switch short pressed - start Brew");
                }

                // Brew switch more than brewSwitchMomentaryLongPress pressed - start flushing
                if (currBrewSwitchStateMomentary == HIGH && brewSwitch->longPressDetected() && machineState != kWaterEmpty) {
                    brewSwitchState = kBrewSwitchFlushOff;
                    valveRelay.on();
                    pumpRelay.on();
                    startingTime = millis();
                    LOG(DEBUG, "brewSwitchState = kBrewSwitchBrew: brew switch long pressed - start flushing");
                }
                break;

            case kBrewSwitchBrewAbort:
                // Brew switch got short pressed while brew is running - abort brew 
                if ((currBrewSwitchStateMomentary == HIGH && currStateBrewSwitch == HIGH) || (machineState == kShotTimerAfterBrew) || (backflushState == kBackflushWaitBrewswitchOff)) {
                    currStateBrewSwitch = LOW;
                    brewSwitchState = kBrewSwitchReset;
                    LOG(DEBUG, "brewSwitchState = kBrewSwitchBrewAbort: brew switch short pressed - stop brew");
                }
                break;

            case kBrewSwitchFlushOff:
                // Brew switch got released - stop flushing
                if (currBrewSwitchStateMomentary == LOW && currStateBrewSwitch == LOW) {
                    brewSwitchState = kBrewSwitchReset;
                    LOG(DEBUG, "brewswitchTriggerCase = kBrewSwitchFlushOff: brew switch long press released - stop flushing");
                    valveRelay.off();
                    pumpRelay.off();
                }
            break;

            case kBrewSwitchReset:
                // Brew switch is released - go back to start and wait for next brew switch input
                if (currBrewSwitchStateMomentary == LOW) {
                    brewSwitchState = kBrewSwitchIdle;
                    LOG(DEBUG, "brewSwitchState = kBrewSwitchReset: brew switch released - go to kBrewSwitchIdle ");
                }
                break;
        }
    }
}

/**
 * @brief Backflush
 */
void backflush() {
    if (backflushState != kBackflushWaitBrewswitchOn && backflushOn == 0) {
        backflushState = kBackflushWaitBrewswitchOff;  // Force reset in case backflushOn is reset during backflush!
        LOG(INFO, "Backflush: Disabled via Webinterface");
    }
    else if (offlineMode == 1 || currBrewState > kBrewIdle || maxflushCycles <= 0 || backflushOn == 0) {
        return;
    }

    if (pidMode == 1) {  // Deactivate PID
        pidMode = 0;
        bPID.SetMode(pidMode);
        pidOutput = 0;
    }

    heaterRelay.off();  // Stop heating

    checkbrewswitch();

    if (currStateBrewSwitch == LOW && backflushState != kBackflushWaitBrewswitchOn) {  // Abort function for state machine from every state
        backflushState = kBackflushWaitBrewswitchOff;
    }

    // State machine for backflush
    switch (backflushState) {
        case kBackflushWaitBrewswitchOn:
            if (currStateBrewSwitch == HIGH && backflushOn) {
                startingTime = millis();
                backflushState = kBackflushFillingStart;
            }

            break;

        case kBackflushFillingStart:
            LOG(INFO, "Backflush: Portafilter filling...");
            valveRelay.on();
            pumpRelay.on();
            backflushState = kBackflushFilling;

            break;

        case kBackflushFilling:
            if (millis() - startingTime > FILLTIME) {
                startingTime = millis();
                backflushState = kBackflushFlushingStart;
            }

            break;

        case kBackflushFlushingStart:
            LOG(INFO, "Backflush: Flushing to drip tray...");
            valveRelay.off();
            pumpRelay.off();
            flushCycles++;
            backflushState = kBackflushFlushing;

            break;

        case kBackflushFlushing:
            if (millis() - startingTime > flushTime && flushCycles < maxflushCycles) {
                startingTime = millis();
                backflushState = kBackflushFillingStart;
            }
            else if (flushCycles >= maxflushCycles) {
                backflushState = kBackflushWaitBrewswitchOff;
            }

            break;

        case kBackflushWaitBrewswitchOff:
            if (currStateBrewSwitch == LOW) {
                LOG(INFO, "Backflush: Finished!");
                valveRelay.off();
                pumpRelay.off();
                flushCycles = 0;
                backflushState = kBackflushWaitBrewswitchOn;
            }

            break;
    }
}

#if (BREWMODE == 1)  // normal brew mode (based on time)
/**
 * @brief PreInfusion, Brew Normal
 */
void brew() {
    if (OnlyPID == 0) {
        unsigned long currentMillisTemp = millis();
        checkbrewswitch();

        if (currStateBrewSwitch == LOW && currBrewState > kBrewIdle) {
            // abort function for state machine from every state
            LOG(INFO, "Brew stopped manually");
            currBrewState = kWaitBrewOff;
        }

        if (currBrewState > kBrewIdle && currBrewState < kWaitBrewOff || brewSwitchState == kBrewSwitchFlushOff) {
            timeBrewed = currentMillisTemp - startingTime;
        }

        if (currStateBrewSwitch == LOW && movingAverageInitialized) {
            // check if brewswitch was turned off at least once, last time,
            brewSwitchWasOff = true;
        }

        totalBrewTime = (preinfusion * 1000) + (preinfusionPause * 1000) +
            (brewTime * 1000);  // running every cycle, in case changes are done during brew

        // state machine for brew
        switch (currBrewState) {
            case kBrewIdle: // waiting step for brew switch turning on
                if (currStateBrewSwitch == HIGH && backflushState == 10 && backflushOn == 0 && brewSwitchWasOff && machineState != kWaterEmpty) {
                    startingTime = millis();

                    if (preinfusionPause == 0 || preinfusion == 0) {
                        currBrewState = kBrewRunning;
                    }
                    else {
                        currBrewState = kPreinfusion;
                    }

                    coldstart = false;  // force reset coldstart if shot is pulled
                }
                else {
                    backflush();
                }

                break;

            case kPreinfusion:  // preinfusioon
                LOG(INFO, "Preinfusion");
                valveRelay.on();
                pumpRelay.on();
                currBrewState = kWaitPreinfusion;

                break;

            case kWaitPreinfusion:  // waiting time preinfusion
                if (timeBrewed > (preinfusion * 1000)) {
                    currBrewState = kPreinfusionPause;
                }

                break;

            case kPreinfusionPause:  // preinfusion pause
                LOG(INFO, "Preinfusion pause");
                valveRelay.on();
                pumpRelay.off();
                currBrewState = kWaitPreinfusionPause;

                break;

            case kWaitPreinfusionPause:  // waiting time preinfusion pause
                if (timeBrewed > ((preinfusion * 1000) + (preinfusionPause * 1000))) {
                    currBrewState = kBrewRunning;
                }

                break;

            case kBrewRunning:  // brew running
                LOG(INFO, "Brew started");
                valveRelay.on();
                pumpRelay.on();
                currBrewState = kWaitBrew;

                break;

            case kWaitBrew:  // waiting time brew
                lastBrewTime = timeBrewed;

                if (timeBrewed > totalBrewTime) {
                    currBrewState = kBrewFinished;
                }

                break;

            case kBrewFinished:  // brew finished
                LOG(INFO, "Brew stopped");
                valveRelay.off();
                pumpRelay.off();
                currBrewState = kWaitBrewOff;
                timeBrewed = 0;

                break;

            case kWaitBrewOff:  // waiting for brewswitch off position
                if (currStateBrewSwitch == LOW) {
                    valveRelay.off();
                    pumpRelay.off();

                    // disarmed button
                    currentMillisTemp = 0;
                    brewDetected = 0;  // rearm brewDetection
                    currBrewState = kBrewIdle;
                    timeBrewed = 0;
                }

                break;
        }
    }
}
#endif

#if (BREWMODE == 2)
/**
 * @brief Weight based brew mode
 */
void brew() {
    if (OnlyPID == 0) {
        checkbrewswitch();
        unsigned long currentMillisTemp = millis();

        if (currStateBrewSwitch == LOW && currBrewState > kBrewIdle) {
            // abort function for state machine from every state
            currBrewState = kWaitBrewOff;
        }

        if (currBrewState > kBrewIdle && currBrewState < kWaitBrewOff) {
            timeBrewed = currentMillisTemp - startingTime;
            weightBrew = weight - weightPreBrew;
        }

        if (currStateBrewSwitch == LOW && movingAverageInitialized) {
            // check if brewswitch was turned off at least once, last time,
            brewSwitchWasOff = true;
        }

        totalBrewTime = ((preinfusion * 1000) + (preinfusionPause * 1000) +
            (brewTime * 1000));  // running every cycle, in case changes are done during brew

        // state machine for brew
        switch (currBrewState) {
            case 10:  // waiting step for brew switch turning on
                if (currStateBrewSwitch == HIGH && backflushState == 10 && backflushOn == 0 && brewSwitchWasOff) {
                    startingTime = millis();
                    currBrewState = kPreinfusion;

                    if (preinfusionPause == 0 || preinfusion == 0) {
                        currBrewState = kBrewRunning;
                    }

                    coldstart = false;  // force reset coldstart if shot is pulled
                    weightPreBrew = weight;
                } else {
                    backflush();
                }

                break;

            case 20:  // preinfusioon
                LOG(INFO, "Preinfusion");
                valveRelay.on();
                pumpRelay.on();
                currBrewState = kWaitPreinfusion;

                break;

            case 21:  // waiting time preinfusion
                if (timeBrewed > (preinfusion * 1000)) {
                    currBrewState = kPreinfusionPause;
                }

                break;

            case 30:  // preinfusion pause
                LOG(INFO, "preinfusion pause");
                valveRelay.on();
                pumpRelay.off();
                currBrewState = kWaitPreinfusionPause;

                break;

            case 31:  // waiting time preinfusion pause
                if (timeBrewed > ((preinfusion * 1000) + (preinfusionPause * 1000))) {
                    currBrewState = kBrewRunning;
                }

                break;

            case 40:  // brew running
                LOG(INFO, "Brew started");
                valveRelay.on();
                pumpRelay.on();
                currBrewState = kWaitBrew;

                break;

            case 41:  // waiting time brew
                if (weightBrew > (weightSetpoint - scaleDelayValue)) {
                    currBrewState = kBrewFinished;
                }

                break;

            case 42:  // brew finished
                LOG(INFO, "Brew stopped");
                valveRelay.off();
                pumpRelay.off();
                currBrewState = kWaitBrewOff;

                break;

            case 43:  // waiting for brewswitch off position
                if (brewSwitch == LOW) {
                    valveRelay.off();
                    pumpRelay.off();

                    // disarmed button
                    currentMillisTemp = 0;
                    timeBrewed = 0;
                    brewDetected = 0;  // rearm brewDetection
                    currBrewState = kBrewIdle;
                }

                weightBrew = weight - weightPreBrew;  // always calculate weight to show on display

                break;
        }
    }
}
#endif

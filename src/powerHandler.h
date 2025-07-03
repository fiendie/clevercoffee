
/**
 * @file powerHandler.h
 *
 * @brief Handler for digital power switch
 */
#pragma once

inline bool currStatePowerSwitchPressed = false;
inline bool lastPowerSwitchPressed = false;

void performSafeShutdown();

inline void checkPowerSwitch() {
    if (!config.get<bool>("hardware.switches.power.enabled") || powerSwitch == nullptr) {
        return;
    }

    const bool powerSwitchPressed = powerSwitch->isPressed();

    if (const int powerSwitchType = config.get<int>("hardware.switches.power.type"); powerSwitchType == Switch::TOGGLE) {
        if (powerSwitchPressed != lastPowerSwitchPressed) {
            lastPowerSwitchPressed = powerSwitchPressed;

            if (powerSwitchPressed) {
                if (machineState == kStandby) {
                    // Exit standby and turn on
                    machineState = kPidNormal;
                    standbyModeOn = false;
                    resetStandbyTimer(kPidNormal);
                    setRuntimePidState(true);
                    u8g2->setPowerSave(0);
                }
            }
            else {
                if (machineState != kStandby) {
                    performSafeShutdown();
                    machineState = kStandby;
                    standbyModeOn = true;

                    standbyModeRemainingTimeMillis = 0;
                    standbyModeRemainingTimeDisplayOffMillis = 0;
                }
            }
        }
    }
    else if (powerSwitchType == Switch::MOMENTARY) {
        if (powerSwitchPressed != currStatePowerSwitchPressed) {
            currStatePowerSwitchPressed = powerSwitchPressed;

            if (currStatePowerSwitchPressed) {
                if (machineState == kStandby) {
                    machineState = kPidNormal;
                    standbyModeOn = false;
                    resetStandbyTimer(kPidNormal);
                    setRuntimePidState(true);
                    u8g2->setPowerSave(0);
                }
                else {
                    performSafeShutdown();
                    machineState = kStandby;
                    standbyModeOn = true;

                    standbyModeRemainingTimeMillis = 0;
                    standbyModeRemainingTimeDisplayOffMillis = 0;
                }
            }
        }
    }
}
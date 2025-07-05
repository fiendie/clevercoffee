/**
 * @file powerHandler.h
 *
 * @brief Handler for digital power switch
 */
#pragma once

inline bool currStatePowerSwitchPressed = false;
inline bool lastPowerSwitchPressed = false;

extern bool systemInitialized;

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
                    machineState = kPidNormal;
                    resetStandbyTimer(kPidNormal);
                    setRuntimePidState(true);
                    u8g2->setPowerSave(0);
                }
            }
            else {
                if (machineState != kStandby) {
                    performSafeShutdown();
                    machineState = kStandby;
                    standbyModeRemainingTimeMillis = 0;
                    standbyModeRemainingTimeDisplayOffMillis = 0;
                }
            }
        }
    }
    else if (powerSwitchType == Switch::MOMENTARY) {
        if (powerSwitchPressed != currStatePowerSwitchPressed) {
            currStatePowerSwitchPressed = powerSwitchPressed;

            if (currStatePowerSwitchPressed && systemInitialized) {
                if (machineState == kStandby) {
                    machineState = kPidNormal;
                    resetStandbyTimer(kPidNormal);
                    setRuntimePidState(true);
                    u8g2->setPowerSave(0);
                }
                else {
                    performSafeShutdown();
                    machineState = kStandby;
                    standbyModeRemainingTimeMillis = 0;
                    standbyModeRemainingTimeDisplayOffMillis = 0;
                }
            }
        }

        // Check for long press to trigger reboot (only for momentary switches)
        if (powerSwitchPressed && systemInitialized && powerSwitch->longPressDetected()) {
            LOG(INFO, "Power switch long press detected - initiating system reboot");
            u8g2->setPowerSave(0);

            // Display reboot message
            displayMessage("REBOOTING", "Please wait...", "", "", "", "");
            delay(1000);

            performSafeShutdown();

            LOG(INFO, "System reboot initiated");
            delay(500);

            ESP.restart();
        }
    }
}

/**
 * @brief Check if power switch allows operation (for brew/steam/hot water handlers)
 * @return true if operation is allowed, false otherwise
 */
inline bool isPowerSwitchOperationAllowed() {
    if (!config.get<bool>("hardware.switches.power.enabled") || powerSwitch == nullptr) {
        return true; // No power switch configured, allow operation
    }

    if (const int powerSwitchType = config.get<int>("hardware.switches.power.type"); powerSwitchType == Switch::TOGGLE) {
        return powerSwitch->isPressed();
    }
    else if (powerSwitchType == Switch::MOMENTARY) {
        // For momentary switches, check machine state instead of switch state
        return machineState != kStandby;
    }

    return true;
}
/**
 * @file displayTemplateScale.h
 *
 * @brief Display template with brew scale
 *
 */

#pragma once

/**
 * @brief Send data to display
 */
inline void printScreen() {

    // Show fullscreen brew timer:
    if (displayFullscreenBrewTimer()) {
        // Display was updated, end here
        return;
    }

    // Show fullscreen manual flush timer:
    if (displayFullscreenManualFlushTimer()) {
        // Display was updated, end here
        return;
    }

    // Show fullscreen hot water on timer:
    if (displayFullscreenHotWaterOnTimer()) {
        // Display was updated, end here
        return;
    }

    // Print the machine state
    if (displayMachineState()) {
        // Display was updated, end here
        return;
    }

    // If no specific machine state was printed, print default:
    u8g2->clearBuffer();

    displayStatusbar();

    displayThermometerOutline(4, 62);

    // Draw current temp in thermometer
    if (fabs(temperature - setpoint) < 0.3) {
        if (isrCounter < 500) {
            drawTemperaturebar(8, 30);
        }
    }
    else {
        drawTemperaturebar(8, 30);
    }

    // Draw current temp and temp setpoint
    u8g2->setFont(u8g2_font_profont11_tf);

    u8g2->setCursor(32, 16);
    u8g2->print("T: ");
    u8g2->print(temperature, 1);
    u8g2->print("/");
    u8g2->print(setpoint, 1);
    u8g2->print(static_cast<char>(176));
    u8g2->print("C");

    const bool scaleEnabled = config.get<bool>("hardware.sensors.scale.enabled");

    if (scaleEnabled) {
        // Show current weight if scale has no error
        displayBrewWeight(32, 26, currReadingWeight, -1, scaleFailure);
    }
    /**
     * @brief Shot timer for scale
     *
     * If scale has an error show fault on the display otherwise show current reading of the scale
     * if brew is running show current brew time and current brew weight
     * if brewControl is enabled and time or weight target is set show targets
     * if brewControl is enabled show flush time during manualFlush
     * if pressure sensor is enabled show current pressure during brew
     * if brew is finished show brew values for postBrewTimerDuration
     */

    if (config.get<bool>("hardware.switches.brew.enabled")) {
        if (config.get<bool>("brew.by_time") || config.get<bool>("brew.by_weight")) {
            if (shouldDisplayBrewTimer()) {
                // Time
                displayBrewTime(32, 36, langstring_brew, currBrewTime, totalTargetBrewTime);

                // Weight
                u8g2->setDrawColor(0);
                u8g2->drawBox(32, 27, 100, 10);
                u8g2->setDrawColor(1);

                if (scaleEnabled) {
                    displayBrewWeight(32, 26, currBrewWeight, targetBrewWeight, scaleFailure);
                }
            }

            // Show flush time
            if (machineState == kManualFlush) {
                u8g2->setDrawColor(0);
                u8g2->drawBox(32, 37, 100, 10);
                u8g2->setDrawColor(1);
                displayBrewTime(32, 36, langstring_manual_flush, currBrewTime);
            }
        }
        else {
            // Brew Timer with optocoupler
            if (shouldDisplayBrewTimer()) {
                // Time
                displayBrewTime(32, 36, langstring_brew, currBrewTime);

                // Weight
                u8g2->setDrawColor(0);
                u8g2->drawBox(32, 27, 100, 10);
                u8g2->setDrawColor(1);

                if (scaleEnabled) {
                    displayBrewWeight(32, 26, currBrewWeight, -1, scaleFailure);
                }
            }
        }
    }

    if (config.get<bool>("hardware.sensors.pressure.enabled")) {
        u8g2->setCursor(32, 46);
        u8g2->print(langstring_pressure);
        u8g2->print(inputPressure, 1);
    }

    // Show heater output in %
    displayProgressbar(pidOutput / 10, 30, 60, 98);

    displayBufferReady = true;
}

#pragma once

ESP32Encoder encoder;

unsigned long startMillisEncoderSwitch = 0;
unsigned long EncoderSwitchBackflushInterval = 2000;
unsigned long EncoderSwitchControlInterval = 800;
bool encoderSwitchPressed = false;
bool displayProfileDescription = false;
bool blockScroll = false;
int descriptionScrollY = 0;

void initEncoder() {
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    // encoder.attachHalfQuad(PIN_ROTARY_DT, PIN_ROTARY_CLK);
    encoder.attachFullQuad(PIN_ROTARY_DT, PIN_ROTARY_CLK);
    encoder.setCount(0);
}

int getEncoderDelta(void) {
    static long lastValueDelta = 0;
    static long lastValueSent = 0;
    long value = encoder.getCount();
    int delta = (value - lastValueDelta) / (menuLevel == 4 ? 1 : 2); // smooth scroll for specific menuLevels

    if (lastValueSent != value) {
        LOGF(INFO, "Rotary Encoder Value: %i", value);
        lastValueSent = value;
    }

    if (delta != 0) {
        lastValueDelta = value;
    }

    return delta;
}

void encoderHandler() {
    if (!config.get<bool>("hardware.switches.encoder.enabled") || encoderSwitch == nullptr) {
        return;
    }

    int delta = getEncoderDelta();

    if (machineState != kBackflush) {
        if (delta != 0) {
            if (menuLevel == 1) {
                brewSetpoint = constrain(brewSetpoint + ((float)delta * 0.1), BREW_SETPOINT_MIN, BREW_SETPOINT_MAX);
                config.set<double>("brew.setpoint", brewSetpoint);
            }
        }
    }

    if (encoderSwitch->isPressed()) {
        if (encoderSwitchPressed == false) {
            startMillisEncoderSwitch = millis();
            encoderSwitchPressed = true;
        }
    }
    else {
        if (encoderSwitchPressed == true) {
            unsigned long duration = millis() - startMillisEncoderSwitch;
            if (duration > EncoderSwitchBackflushInterval) { // toggle every interval
                if (machineState == kBackflush) {
                    backflushOn = false;
                    startMillisEncoderSwitch = millis();
                }

                if (machineState == kPidNormal) {
                    backflushOn = true;
                    startMillisEncoderSwitch = millis();
                }
            }
            else if (duration > EncoderSwitchControlInterval) { // toggle every interval
                menuLevel = 0;
                if (!config.save()) {
                    LOG(ERROR, "Failed to save config to filesystem!");
                }
            }
            else {
                menuLevel = (menuLevel == 1) ? 0 : 1; // this is where it toggles between different levels
            }

            LOGF(INFO, "Rotary Encoder Button down for: %lu ms, menuLevel: %d", duration, menuLevel);
        }

        encoderSwitchPressed = false;
    }
}
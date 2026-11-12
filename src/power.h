/*
 * ==============================================================================
 * Suzuki RF400 EV - 4s LiFePO4 Auxiliary Rail Power Monitor
 * ==============================================================================
 * Samples VBAT_PIN (A7) using existing iPDM ADC factor:
 *   analogRead_mV_factor16(VBAT_PIN, ADC_FACTOR16_VBAT)
 * Implements a 3-second sustained low-voltage filter (11.6V) with 12.0V recovery
 * using the generic ipdm::HysteresisThreshold primitive.
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/ipdm_util.h"

class PowerMonitor {
public:
    PowerMonitor();
    void begin();
    void update();

    uint16_t getVoltageMV() const { return _vbatMV; }
    float    getVoltageVolts() const { return (float)_vbatMV / 1000.0f; }
    bool     isLowVoltageAlert() const { return _voltageHysteresis.isTripped(); }

private:
    uint16_t _vbatMV;
    ipdm::HysteresisThreshold _voltageHysteresis;
};

extern PowerMonitor powerMon;

#include "power.h"
#include "src/ipdm_io.h"
#include "src/ipdm_util.h"

PowerMonitor powerMon;

PowerMonitor::PowerMonitor()
    : _vbatMV(12800)
    , _voltageHysteresis(LFP_LOW_VOLTAGE_MV, LFP_RECOVERY_MV, LFP_HYST_MS, true)
{
}

void PowerMonitor::begin() {
    _vbatMV = 12800;
    _voltageHysteresis.reset(false);
}

void PowerMonitor::update() {
    // Read actual auxiliary rail voltage via the existing iPDM utility macro
    _vbatMV = analogRead_mV_factor16(ipdm::VBAT_PIN, ipdm::ADC_FACTOR16_VBAT);

    // Update timed hysteresis threshold
    _voltageHysteresis.update(_vbatMV);
}

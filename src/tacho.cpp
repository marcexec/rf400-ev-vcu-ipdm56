#include "tacho.h"

Tachometer tacho;

Tachometer::Tachometer()
    : _stage(TachoBootStage::BOOT_DELAY)
    , _lastUpdateMs(0)
    , _stageTimerMs(0)
    , _targetRPM(0.0f)
    , _virtualRPM(0.0f)
    , _outputFreq(0.0f)
    , _manualOverride(false)
    , _needleFilter(TACHO_FILTER_ALPHA, 0.0f)
{
}

void Tachometer::begin() {
    pinMode(PIN_TACHO_OUT, OUTPUT);
    noTone(PIN_TACHO_OUT);

    _stage = TachoBootStage::BOOT_DELAY;
    _stageTimerMs = millis();
    _lastUpdateMs = millis();
    _targetRPM = 0.0f;
    _virtualRPM = 0.0f;
    _outputFreq = 0.0f;
    _manualOverride = false;
    _needleFilter.reset(0.0f);
}

void Tachometer::triggerBootSweep() {
    _stage = TachoBootStage::STAGE1_SWEEP_UP;
    _stageTimerMs = millis();
    _virtualRPM = 0.0f;
    _manualOverride = false;
    _needleFilter.reset(0.0f);
}

void Tachometer::setTestRPM(float rpm) {
    _manualOverride = true;
    _targetRPM = constrain(rpm, 0.0f, (float)TACHO_MAX_RPM);
    _virtualRPM = _targetRPM;
    calculateAndApplyTone();
}

void Tachometer::update(int32_t currentMA, bool canHealthy) {
    unsigned long now = millis();
    if (now - _lastUpdateMs < 15) {
        return; // Fixed 15ms calculation interval (66 Hz update rate)
    }
    _lastUpdateMs = now;

    if (_manualOverride) {
        calculateAndApplyTone();
        return;
    }

    // =========================================================================
    // TWO-STAGE STARTUP SWEEP STATE MACHINE
    // =========================================================================
    switch (_stage) {
        case TachoBootStage::BOOT_DELAY:
            // Stagger start: wait 200ms after boot before sweeping
            if (now - _stageTimerMs > 200) {
                _stage = TachoBootStage::STAGE1_SWEEP_UP;
                _stageTimerMs = now;
            }
            break;

        case TachoBootStage::STAGE1_SWEEP_UP:
            // Majestic slow sweep: ramp smoothly (+100 RPM per 15ms -> ~2.4 sec to peak)
            _virtualRPM += 100.0f;
            if (_virtualRPM >= (float)TACHO_MAX_RPM) {
                _virtualRPM = (float)TACHO_MAX_RPM;
                _stage = TachoBootStage::STAGE1_HOLD_TOP;
                _stageTimerMs = now;
            }
            break;

        case TachoBootStage::STAGE1_HOLD_TOP:
            // Brief pause at the top mark
            if (now - _stageTimerMs > 300) {
                _stage = TachoBootStage::STAGE1_DROP_ZERO;
            }
            break;

        case TachoBootStage::STAGE1_DROP_ZERO:
            // Smooth descent to calibrated zero mark (6,000 RPM dial mark)
            _virtualRPM -= 250.0f;
            if (_virtualRPM <= TACHO_CALIBRATED_ZERO) {
                _virtualRPM = TACHO_CALIBRATED_ZERO;
                _stage = TachoBootStage::STAGE1_PAUSE_ZERO;
                _stageTimerMs = now;
            }
            break;

        case TachoBootStage::STAGE1_PAUSE_ZERO:
            // Brief 250ms settle at 0 Amps mark before the ready flick
            if (now - _stageTimerMs > 250) {
                _stage = TachoBootStage::STAGE2_FLICK_UP;
                _stageTimerMs = now;
            }
            break;

        case TachoBootStage::STAGE2_FLICK_UP:
            // Fast "Ready" flick to 16k (+600 RPM per 15ms -> ~250ms snap)
            _virtualRPM += 600.0f;
            if (_virtualRPM >= (float)TACHO_MAX_RPM) {
                _virtualRPM = (float)TACHO_MAX_RPM;
                _stage = TachoBootStage::STAGE2_SNAP_BACK;
            }
            break;

        case TachoBootStage::STAGE2_SNAP_BACK:
            // Snappy snap back to 6k (-700 RPM per 15ms)
            _virtualRPM -= 700.0f;
            if (_virtualRPM <= TACHO_CALIBRATED_ZERO) {
                _virtualRPM = TACHO_CALIBRATED_ZERO;
                _targetRPM = TACHO_CALIBRATED_ZERO;
                _needleFilter.reset(_virtualRPM);
                _stage = TachoBootStage::LIVE_TRACKING; // Startup sweep complete!
            }
            break;

        case TachoBootStage::LIVE_TRACKING:
            if (!canHealthy) {
                // CAN DISCONNECTED / TIMEOUT FAILSAFE:
                // Smoothly park needle at 1,000 RPM so rider clearly sees comms fault
                _targetRPM = TACHO_CAN_FAULT_RPM;
            } else {
                // NORMAL OPERATION:
                // Map traction current in mA to RPM deflection around calibrated center mark
                _targetRPM = TACHO_CALIBRATED_ZERO + ((float)currentMA * TACHO_MA_TO_RPM_RATIO);
                _targetRPM = constrain(_targetRPM, 0.0f, (float)TACHO_MAX_RPM);
            }

            // Exponential moving average filter prevents needle jitter
            _virtualRPM = _needleFilter.update(_targetRPM);
            break;
    }

    calculateAndApplyTone();
}

void Tachometer::calculateAndApplyTone() {
    // Safety guard on virtual RPM bounds
    if (_virtualRPM < 0.0f) {
        _virtualRPM = 0.0f;
    } else if (_virtualRPM > (float)TACHO_MAX_RPM) {
        _virtualRPM = (float)TACHO_MAX_RPM;
    }

    // Frequency (Hz) = (RPM / 60) for RF400 tachometer pulse scaling
    float f_ideal = _virtualRPM / 60.0f;

    // Apply quadratic correction polynomial for RF400 physical gauge linearity:
    // f_out = A * f^2 + B * f
    _outputFreq = (TACHO_COEFF_A * f_ideal * f_ideal) + (TACHO_COEFF_B * f_ideal);

    // Negative frequency protection guard
    if (_outputFreq < 0.0f) {
        _outputFreq = 0.0f;
    }

    if (_outputFreq >= 1.0f) {
        tone(PIN_TACHO_OUT, (unsigned int)_outputFreq);
    } else {
        noTone(PIN_TACHO_OUT);
    }
}

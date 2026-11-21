/*
 * ==============================================================================
 * Suzuki RF400 EV - Precision Analog Tachometer Driver
 * ==============================================================================
 * Drives the RF400 tachometer meter movement via native timer frequency (tone()).
 *
 * Operational Logic:
 * 1. Two-Stage Boot Sweep on Key-On:
 *    - Stage 1: Majestic, deliberate sweep from 0 -> 16,000 RPM, smooth drop to 6k.
 *    - Stage 2: Crisp "Ready" flick up to 16,000 RPM and snappy snap back to 6k.
 * 2. Live Tracking:
 *    - Bi-directional battery current (0 Amps = 6,000 RPM center mark).
 *    - Generic ipdm::EmaFilter smoothing prevents needle jitter.
 *    - Quadratic correction coefficients (COEFF_A, COEFF_B) for gauge linearity.
 * 3. Failsafe:
 *    - If CAN drops out (>1s timeout), needle smoothly parks at 1,000 RPM.
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/ipdm_util.h"

enum class TachoBootStage {
    BOOT_DELAY,        // Brief pause after power-on
    STAGE1_SWEEP_UP,   // Smooth sweep up to 16k
    STAGE1_HOLD_TOP,   // Brief pause at peak
    STAGE1_DROP_ZERO,  // Fast drop all the way to 0 RPM
    STAGE1_PAUSE_ZERO, // Brief settle at zero
    STAGE2_FLICK_UP,   // Fast snappy flick to 16k
    STAGE2_SNAP_BACK,  // Fast snap back to calibrated 6,000 RPM center
    LIVE_TRACKING      // Armed and tracking live ISA traction current
};

class Tachometer {
public:
    Tachometer();
    void begin();
    void update(int32_t currentMA, bool canHealthy);

    // Getters for telemetry & CLI inspection
    float getTargetRPM() const { return _targetRPM; }
    float getVirtualRPM() const { return _virtualRPM; }
    float getOutputFreqHz() const { return _outputFreq; }
    TachoBootStage getStage() const { return _stage; }
    bool isBootSweepComplete() const { return _stage == TachoBootStage::LIVE_TRACKING; }

    // Manual test triggers (from Lebowski CLI)
    void triggerBootSweep();
    void setTestRPM(float rpm);

private:
    TachoBootStage _stage;
    unsigned long  _lastUpdateMs;
    unsigned long  _stageTimerMs;

    float _targetRPM;
    float _virtualRPM;
    float _outputFreq;
    bool  _manualOverride;

    ipdm::EmaFilter _needleFilter;

    void calculateAndApplyTone();
};

extern Tachometer tacho;

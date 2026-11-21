/*
 * ==============================================================================
 * Suzuki RF400 EV - ISAscale IVT-S CAN Bus Interface
 * ==============================================================================
 * Reads traction current from ISAscale IVT-S shunt over CAN1 (MCP2515 @ 500kbps).
 * Monitors communication health and flags failsafe when packets drop out.
 *
 * Power Control:
 * The ISA shunt is powered via HOUT3 (+12V switched). On boot, we wait
 * ISA_POWER_ON_DELAY_MS before enabling it, giving all other systems time to
 * stabilise first. The shunt can be power-cycled via CLI for a reset.
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/can_common.h"

class IsaCanBus {
public:
    void begin();
    void update();
    void handleFrame(const CAN_FRAME &frame);

    // Power control (HOUT3)
    void enablePower();                   // Switch HOUT3 ON
    void disablePower();                  // Switch HOUT3 OFF (shunt reset)
    bool isPowered() const { return _powered; }

    // Live telemetry getters
    int32_t getCurrentMA() const { return _currentMA; }
    float   getCurrentAmps() const { return (float)_currentMA / 1000.0f; }
    uint32_t getPacketCount() const { return _packetCount; }
    bool    isHealthy() const { return _healthy; }
    unsigned long getLastFrameAgeMs() const {
        if (_lastFrameTime == 0) return 99999;
        return millis() - _lastFrameTime;
    }

    // Test / Diagnostic override (for Lebowski CLI testing without active shunt)
    void setSimulatedCurrentMA(int32_t ma) {
        _currentMA = ma;
        _lastFrameTime = millis();
        _healthy = true;
    }

private:
    volatile int32_t _currentMA = 0;
    volatile uint32_t _packetCount = 0;
    volatile unsigned long _lastFrameTime = 0;
    bool _healthy = false;
    bool _powered = false;
    unsigned long _bootMs = 0;
    bool _powerDelayDone = false;
};

extern IsaCanBus isaBus;

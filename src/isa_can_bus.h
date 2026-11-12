/*
 * ==============================================================================
 * Suzuki RF400 EV - ISAscale IVT-S CAN Bus Interface
 * ==============================================================================
 * Reads traction current from ISAscale IVT-S shunt over CAN1 (MCP2515 @ 500kbps).
 * Monitors communication health and flags failsafe when packets drop out.
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

    // Live telemetry getters
    int32_t getCurrentMA() const { return _currentMA; }
    float   getCurrentAmps() const { return (float)_currentMA / 1000.0f; }
    uint32_t getPacketCount() const { return _packetCount; }
    bool    isHealthy() const { return _healthy; }
    unsigned long getLastFrameAgeMs() const { return millis() - _lastFrameTime; }

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
};

extern IsaCanBus isaBus;


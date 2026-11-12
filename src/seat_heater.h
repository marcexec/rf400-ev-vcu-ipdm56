/*
 * ==============================================================================
 * Suzuki RF400 EV - Heated Seat Controller
 * ==============================================================================
 * Controls high-side power (+12V switched) to the heated seat pad on HOUT6.
 *
 * Operational Rules:
 * - Failsafe: ALWAYS OFF on boot.
 * - Toggleable via Handlebar Starter Button long-press (>800ms).
 * - Diagnostic toggle via Lebowski serial CLI ('d').
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"

class SeatHeater {
public:
    SeatHeater();
    void begin();
    void toggle();
    void set(bool on);
    bool isOn() const { return _isOn; }

private:
    bool _isOn;
};

extern SeatHeater seatHeater;

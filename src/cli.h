/*
 * ==============================================================================
 * Suzuki RF400 EV - Lebowski-like Serial CLI & Diagnostic Terminal
 * ==============================================================================
 * Baud: 115200, 8 data bits, 1 stop bit, no parity (8N1).
 *
 * Operation:
 * 1. Default Mode (Run): Prints clean single-line periodic telemetry.
 * 2. Setup Menu Mode: Press 'm' or '?' to open the classic Lebowski-style menu:
 *    0) trigger boot sweep
 *    a) toggle left turn
 *    b) toggle right turn
 *    c) toggle hazard warning
 *    d) toggle heated seat
 *    e) read turn current feedback
 *    f) simulate CAN shunt current (enter mA)
 *    g) print raw inputs & ADC status
 *    z) return to live running
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>

class SerialCli {
public:
    void begin();
    void update();

    bool isInMenuMode() const { return _inMenu; }

private:
    bool _inMenu = false;
    unsigned long _lastTelemetryMs = 0;

    void printMainMenu();
    void handleMenuInput(char c);
    void printTelemetryLine();
    void printDetailedStatus();
};

extern SerialCli cli;


/*
 * ==============================================================================
 * Suzuki RF400 EV - Input Polling & Multi-Press Button State Machine
 * ==============================================================================
 * Reads:
 * 1. Handlebar Turn Switch (PIN_TURN_SW_L, PIN_TURN_SW_R):
 *    - Priority override: Any switch change overrides previous mode or hazards.
 * 2. Starter Button (PIN_STARTER_BTN) using generic ipdm::Button:
 *    - Single Click : UI / Display page select (stubs for future screen).
 *    - Triple Click : Toggle Hazard Warning Lights.
 *    - Long Press   : Toggle Heated Seat Power (HOUT6).
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/ipdm_button.h"

class InputManager {
public:
    InputManager();
    void begin();
    void update();

    // Diagnostics
    bool isTurnLeftActive() const { return _rawTurnL; }
    bool isTurnRightActive() const { return _rawTurnR; }
    bool isStarterPressed() const;

private:
    bool _rawTurnL;
    bool _rawTurnR;
    bool _lastTurnL;
    bool _lastTurnR;

    ipdm::Button _starterBtn;

    void processTurnSwitches();
    void processStarterButton();
};

extern InputManager inputs;

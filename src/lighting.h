/*
 * ==============================================================================
 * Suzuki RF400 EV - Lighting State Machine
 * ==============================================================================
 * Manages all vehicle exterior lighting and dashboard lamps on iPDM outputs:
 * 1. Left & Right Turn Indicators (HOUT7, HOUT8) at 80 cpm.
 * 2. Hyperflash detection (160 cpm) via current sense on ED8/ED9.
 * 3. Hazards warning mode (100 cpm) overriding latching turns.
 * 4. Front DRL Winking (HOUT4, HOUT9): DRL shuts off on active turn side.
 * 5. Dashboard Turn LED (LOUT1): Flashes synchronously with active blinker.
 * 6. Dashboard Backlight (LOUT2): Soft boot-up and low-voltage pulse warning.
 *
 * Uses generic ipdm::Flasher primitive for non-blocking cadence generation.
 * (Heated seat controller has been segregated into its own seat_heater module).
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/ipdm_flasher.h"

enum class TurnState {
    OFF,
    LEFT,
    RIGHT,
    HAZARDS
};

class LightingSystem {
public:
    LightingSystem();
    void begin();
    void update(bool lowVoltageAlert, bool canHealthy);

    // Command interface (from switches, starter button, or Lebowski CLI)
    void requestTurnLeft();
    void requestTurnRight();
    void requestTurnOff();
    void toggleHazards();
    void setHazards(bool enable);

    // Telemetry getters
    TurnState getTurnState() const { return _turnState; }
    bool isHyperflashL() const { return _hyperflashL; }
    bool isHyperflashR() const { return _hyperflashR; }
    bool isFlashPhaseOn() const { return _flasher.getOutput(); }

    // Diagnostic current sampling (for Lebowski CLI 'e')
    void sampleTurnCurrent(uint16_t &rawL, uint16_t &rawR);

private:
    TurnState _turnState;
    bool _hazardsLatched;

    bool _hyperflashL;
    bool _hyperflashR;

    unsigned long _bootTimeMs;
    ipdm::Flasher _flasher;

    void updateFlasherTiming();
    void applyOutputs(bool lowVoltageAlert, bool canHealthy);
};

extern LightingSystem lighting;

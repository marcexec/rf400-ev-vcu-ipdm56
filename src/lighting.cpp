#include "lighting.h"
#include "src/ipdm_library.h"

LightingSystem lighting;

LightingSystem::LightingSystem()
    : _turnState(TurnState::OFF)
    , _hazardsLatched(false)
    , _heatedSeatOn(false) // Always OFF on boot!
    , _hyperflashL(false)
    , _hyperflashR(false)
    , _bootTimeMs(0)
    , _flasher(TURN_CYCLE_MS, TURN_DUTY_PCT)
{
}

void LightingSystem::begin() {
    _bootTimeMs = millis();
    _turnState = TurnState::OFF;
    _hazardsLatched = false;
    _heatedSeatOn = false; // Always OFF on boot!
    _hyperflashL = false;
    _hyperflashR = false;
    _flasher.stop();

    // Configure High-Side channels
    ipdm::pinMode(PIN_FRONT_DRL_L, OUTPUT);
    ipdm::pinMode(PIN_FRONT_DRL_R, OUTPUT);
    ipdm::pinMode(PIN_TURN_LIGHT_L, OUTPUT);
    ipdm::pinMode(PIN_TURN_LIGHT_R, OUTPUT);
    ipdm::pinMode(PIN_HEATED_SEAT_PWR, OUTPUT);

    // Configure Low-Side cluster lamp channels
    ipdm::pinMode(PIN_DASH_TURN_LED, OUTPUT);
    ipdm::pinMode(PIN_DASH_BACKLIGHT, OUTPUT);
    ipdm::pinMode(PIN_DASH_READY_LED, OUTPUT);
    ipdm::pinMode(PIN_EV_FAULT_LED, OUTPUT);

    // Initial safe state
    ipdm::digitalWrite(PIN_TURN_LIGHT_L, LOW);
    ipdm::digitalWrite(PIN_TURN_LIGHT_R, LOW);
    ipdm::digitalWrite(PIN_HEATED_SEAT_PWR, LOW);
    ipdm::digitalWrite(PIN_DASH_TURN_LED, LOW);
    ipdm::digitalWrite(PIN_DASH_READY_LED, HIGH); // Cluster neutral lamp = READY
    ipdm::digitalWrite(PIN_EV_FAULT_LED, LOW);
}

void LightingSystem::requestTurnLeft() {
    _hazardsLatched = false;
    _turnState = TurnState::LEFT;
    uint16_t cadence = _hyperflashL ? HYPER_CYCLE_MS : TURN_CYCLE_MS;
    _flasher.setCadence(cadence, TURN_DUTY_PCT);
    _flasher.start();
    _flasher.reset();
}

void LightingSystem::requestTurnRight() {
    _hazardsLatched = false;
    _turnState = TurnState::RIGHT;
    uint16_t cadence = _hyperflashR ? HYPER_CYCLE_MS : TURN_CYCLE_MS;
    _flasher.setCadence(cadence, TURN_DUTY_PCT);
    _flasher.start();
    _flasher.reset();
}

void LightingSystem::requestTurnOff() {
    if (!_hazardsLatched) {
        _turnState = TurnState::OFF;
        _flasher.stop();
    }
}

void LightingSystem::toggleHazards() {
    setHazards(!_hazardsLatched);
}

void LightingSystem::setHazards(bool enable) {
    _hazardsLatched = enable;
    if (_hazardsLatched) {
        _turnState = TurnState::HAZARDS;
        _flasher.setCadence(HAZARD_CYCLE_MS, HAZARD_DUTY_PCT);
        _flasher.start();
        _flasher.reset();
    } else {
        _turnState = TurnState::OFF;
        _flasher.stop();
    }
}

void LightingSystem::toggleHeatedSeat() {
    _heatedSeatOn = !_heatedSeatOn;
    ipdm::digitalWrite(PIN_HEATED_SEAT_PWR, _heatedSeatOn);
}

void LightingSystem::sampleTurnCurrent(uint16_t &rawL, uint16_t &rawR) {
    ipdm::reset_watchdog();

    // Pulse outputs ON briefly to sample current monitor feedback lines
    ipdm::digitalWrite(PIN_TURN_LIGHT_L, HIGH);
    ipdm::digitalWrite(PIN_TURN_LIGHT_R, HIGH);
    delay(100);

    // Read hardware monitor feedback pins on expander
    rawL = ipdm::digitalRead(PIN_SENSE_TURN_L);
    rawR = ipdm::digitalRead(PIN_SENSE_TURN_R);

    // Restore state
    ipdm::digitalWrite(PIN_TURN_LIGHT_L, LOW);
    ipdm::digitalWrite(PIN_TURN_LIGHT_R, LOW);

    ipdm::reset_watchdog();
}

void LightingSystem::update(bool lowVoltageAlert, bool canHealthy) {
    updateFlasherTiming();
    applyOutputs(lowVoltageAlert, canHealthy);
}

void LightingSystem::updateFlasherTiming() {
    if (_turnState == TurnState::OFF) {
        if (_flasher.isRunning()) {
            _flasher.stop();
        }
        return;
    }

    uint16_t cyclePeriod = TURN_CYCLE_MS;
    if (_turnState == TurnState::HAZARDS) {
        cyclePeriod = HAZARD_CYCLE_MS;
    } else if ((_turnState == TurnState::LEFT && _hyperflashL) ||
               (_turnState == TurnState::RIGHT && _hyperflashR)) {
        cyclePeriod = HYPER_CYCLE_MS;
    }

    if (_flasher.getPeriodMs() != cyclePeriod || !_flasher.isRunning()) {
        _flasher.setCadence(cyclePeriod, TURN_DUTY_PCT);
        _flasher.start();
    }
}

void LightingSystem::applyOutputs(bool lowVoltageAlert, bool canHealthy) {
    unsigned long now = millis();
    bool flashPhaseOn = _flasher.getOutput();

    // =========================================================================
    // 1. SOFT-START STAGGERED BOOT RAMPING
    // =========================================================================
    // T = 0ms: Backlight comes up
    // T = 75ms: Left & Right DRLs switch on together
    bool drlMasterEnable = (now - _bootTimeMs >= 75);

    // =========================================================================
    // 2. TURN SIGNALS & DRL WINKING
    // =========================================================================
    bool lightL = false;
    bool lightR = false;
    bool drlL   = drlMasterEnable;
    bool drlR   = drlMasterEnable;

    switch (_turnState) {
        case TurnState::LEFT:
            lightL = flashPhaseOn;
            drlL   = false; // Wink out left white DRL during left turn!
            drlR   = drlMasterEnable;
            break;

        case TurnState::RIGHT:
            lightR = flashPhaseOn;
            drlR   = false; // Wink out right white DRL during right turn!
            drlL   = drlMasterEnable;
            break;

        case TurnState::HAZARDS:
            lightL = flashPhaseOn;
            lightR = flashPhaseOn;
            // During hazards, keep both DRLs active for maximum front visibility
            drlL   = drlMasterEnable;
            drlR   = drlMasterEnable;
            break;

        case TurnState::OFF:
        default:
            lightL = false;
            lightR = false;
            drlL   = drlMasterEnable;
            drlR   = drlMasterEnable;
            break;
    }

    // Write High-Side turn and DRL outputs
    ipdm::digitalWrite(PIN_TURN_LIGHT_L, lightL);
    ipdm::digitalWrite(PIN_TURN_LIGHT_R, lightR);
    ipdm::digitalWrite(PIN_FRONT_DRL_L, drlL);
    ipdm::digitalWrite(PIN_FRONT_DRL_R, drlR);

    // =========================================================================
    // 3. DASHBOARD TURN INDICATOR
    // =========================================================================
    // Single shared green dash turn LED tracks active flashing
    ipdm::digitalWrite(PIN_DASH_TURN_LED, flashPhaseOn && (_turnState != TurnState::OFF));

    // =========================================================================
    // 4. DASHBOARD BACKLIGHT & 4s LFP LOW-VOLTAGE ALERT
    // =========================================================================
    if (lowVoltageAlert) {
        // Warning alert: Pulse cluster backlight at stepped cadence (~300ms)
        bool pulseOn = ((now / 300) % 2 == 0);
        ipdm::digitalWrite(PIN_DASH_BACKLIGHT, pulseOn);
    } else {
        // Normal steady illumination
        ipdm::digitalWrite(PIN_DASH_BACKLIGHT, HIGH);
    }

    // =========================================================================
    // 5. CAN DROPOUT / FAULT WARNING LAMP (LOUT5)
    // =========================================================================
    // If CAN drops out or fault occurs, illuminate red warning lamp
    ipdm::digitalWrite(PIN_EV_FAULT_LED, !canHealthy);
}

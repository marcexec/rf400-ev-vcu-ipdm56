#include "inputs.h"
#include "lighting.h"

InputManager inputs;

InputManager::InputManager()
    : _rawTurnL(false)
    , _rawTurnR(false)
    , _lastTurnL(false)
    , _lastTurnR(false)
    , _starterBtn(PIN_STARTER_BTN, true, 30, 800, 350)
{
}

void InputManager::begin() {
    _starterBtn.begin();
    pinMode(PIN_TURN_SW_L, INPUT_PULLUP);
    pinMode(PIN_TURN_SW_R, INPUT_PULLUP);

    _lastTurnL = false;
    _lastTurnR = false;
}

void InputManager::update() {
    // Read turn switch inputs (Active-LOW with pullups)
    _rawTurnL = (digitalRead(PIN_TURN_SW_L) == LOW);
    _rawTurnR = (digitalRead(PIN_TURN_SW_R) == LOW);

    // Update generic button handler
    _starterBtn.update();

    processTurnSwitches();
    processStarterButton();
}

bool InputManager::isStarterPressed() const {
    return _starterBtn.isPressed();
}

void InputManager::processTurnSwitches() {
    // Detect physical switch transitions (Rider flipped switch)
    bool turnLChanged = (_rawTurnL != _lastTurnL);
    bool turnRChanged = (_rawTurnR != _lastTurnR);

    if (turnLChanged || turnRChanged) {
        // Any change in the physical switch overrides previous mode (including hazards)
        if (_rawTurnL && !_rawTurnR) {
            lighting.requestTurnLeft();
        } else if (_rawTurnR && !_rawTurnL) {
            lighting.requestTurnRight();
        } else if (!_rawTurnL && !_rawTurnR) {
            lighting.requestTurnOff();
        }
    }

    _lastTurnL = _rawTurnL;
    _lastTurnR = _rawTurnR;
}

void InputManager::processStarterButton() {
    // Check for Long Press event (held > 800ms) -> Toggle heated seat
    if (_starterBtn.wasLongPressed()) {
        lighting.toggleHeatedSeat();
        Serial.println(F("[BTN] Long Press -> Heated Seat Toggled"));
    }

    // Check for Single Click event -> UI/Display selection stub
    if (_starterBtn.wasSingleClicked()) {
        Serial.println(F("[BTN] Single Click -> UI / Display Select"));
    }

    // Check for Triple Click event -> Hazards toggle
    if (_starterBtn.wasTripleClicked()) {
        lighting.toggleHazards();
        Serial.println(F("[BTN] Triple Click -> Hazards Toggled"));
    }
}

#include "seat_heater.h"
#include "src/ipdm_library.h"

SeatHeater seatHeater;

SeatHeater::SeatHeater()
    : _isOn(false) // Always OFF on boot!
{
}

void SeatHeater::begin() {
    _isOn = false; // Failsafe guarantee
    ipdm::pinMode(PIN_HEATED_SEAT_PWR, OUTPUT);
    ipdm::digitalWrite(PIN_HEATED_SEAT_PWR, LOW);
}

void SeatHeater::toggle() {
    set(!_isOn);
}

void SeatHeater::set(bool on) {
    _isOn = on;
    ipdm::digitalWrite(PIN_HEATED_SEAT_PWR, _isOn ? HIGH : LOW);
}

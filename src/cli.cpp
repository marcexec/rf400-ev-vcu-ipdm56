#include "cli.h"
#include "config.h"
#include "isa_can_bus.h"
#include "tacho.h"
#include "lighting.h"
#include "power.h"
#include "inputs.h"

SerialCli cli;

void SerialCli::begin() {
    Serial.begin(115200);
    _inMenu = false;
    _lastTelemetryMs = millis();

    Serial.println();
    Serial.println(F("################################################"));
    Serial.println(F("#  Suzuki RF400 EV - iPDM56 v1.1 Controller    #"));
    Serial.println(F("#  Press 'm' or '?' for Lebowski-like menu     #"));
    Serial.println(F("################################################"));
}

void SerialCli::update() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\r' || c == '\n') continue;

        if (!_inMenu) {
            if (c == 'm' || c == '?' || c == 'M') {
                _inMenu = true;
                printMainMenu();
            }
        } else {
            handleMenuInput(c);
        }
    }

    // Print live periodic telemetry line only when NOT in menu mode
    if (!_inMenu) {
        unsigned long now = millis();
        if (now - _lastTelemetryMs >= 1000) {
            _lastTelemetryMs = now;
            printTelemetryLine();
        }
    }
}

void SerialCli::printMainMenu() {
    Serial.println();
    Serial.println(F("########################################"));
    Serial.println(F("#  iPDM56 RF400 VCU Diagnostic Menu    #"));
    Serial.println(F("########################################"));
    Serial.println(F("0) Trigger gauge two-stage boot sweep"));
    Serial.println(F("a) Toggle Left turn indicator"));
    Serial.println(F("b) Toggle Right turn indicator"));
    Serial.println(F("c) Toggle Hazard warning lights"));
    Serial.println(F("d) Toggle Heated seat power"));
    Serial.println(F("e) Read turn signal current feedback"));
    Serial.println(F("f) Simulate ISA shunt current (+20 Amps)"));
    Serial.println(F("g) Simulate ISA shunt current (-10 Amps regen)"));
    Serial.println(F("h) Simulate ISA shunt current (0 Amps)"));
    Serial.println(F("i) Detailed system I/O & ADC report"));
    Serial.println(F("z) Return to live running mode"));
    Serial.print(F("------> "));
}

void SerialCli::handleMenuInput(char c) {
    Serial.println(c);

    switch (c) {
        case '0':
            Serial.println(F("Executing tachometer boot sweep..."));
            tacho.triggerBootSweep();
            break;

        case 'a':
        case 'A':
            if (lighting.getTurnState() == TurnState::LEFT) {
                lighting.requestTurnOff();
                Serial.println(F("Turn Left: OFF"));
            } else {
                lighting.requestTurnLeft();
                Serial.println(F("Turn Left: ON"));
            }
            break;

        case 'b':
        case 'B':
            if (lighting.getTurnState() == TurnState::RIGHT) {
                lighting.requestTurnOff();
                Serial.println(F("Turn Right: OFF"));
            } else {
                lighting.requestTurnRight();
                Serial.println(F("Turn Right: ON"));
            }
            break;

        case 'c':
        case 'C':
            lighting.toggleHazards();
            Serial.print(F("Hazards: "));
            Serial.println(lighting.getTurnState() == TurnState::HAZARDS ? F("ON") : F("OFF"));
            break;

        case 'd':
        case 'D':
            lighting.toggleHeatedSeat();
            Serial.print(F("Heated Seat: "));
            Serial.println(lighting.isHeatedSeatOn() ? F("ON (+12V)") : F("OFF"));
            break;

        case 'e':
        case 'E': {
            uint16_t curL = 0, curR = 0;
            Serial.println(F("Pulsing indicators to sample current monitors..."));
            lighting.sampleTurnCurrent(curL, curR);
            Serial.print(F("  Left  Sense (ED8): ")); Serial.println(curL);
            Serial.print(F("  Right Sense (ED9): ")); Serial.println(curR);
            break;
        }

        case 'f':
        case 'F':
            Serial.println(F("Simulating ISA current: +20.0 A (1000 RPM above center)"));
            isaBus.setSimulatedCurrentMA(20000);
            break;

        case 'g':
        case 'G':
            Serial.println(F("Simulating ISA current: -10.0 A regen (500 RPM below center)"));
            isaBus.setSimulatedCurrentMA(-10000);
            break;

        case 'h':
        case 'H':
            Serial.println(F("Simulating ISA current: 0.0 A (resting at 6000 RPM mark)"));
            isaBus.setSimulatedCurrentMA(0);
            break;

        case 'i':
        case 'I':
            printDetailedStatus();
            break;

        case 'z':
        case 'Z':
            _inMenu = false;
            Serial.println(F("Returning to live running telemetry..."));
            return;

        default:
            Serial.println(F("Unknown option. Press 'z' to return to live mode."));
            break;
    }

    Serial.print(F("------> "));
}

void SerialCli::printTelemetryLine() {
    Serial.print(F("[LIVE] 4s LFP: "));
    Serial.print(powerMon.getVoltageVolts(), 2);
    Serial.print(F("V | ISA: "));
    Serial.print(isaBus.getCurrentAmps(), 1);
    Serial.print(F("A ("));
    Serial.print(isaBus.isHealthy() ? F("OK") : F("LOST"));
    Serial.print(F(") | Tacho: "));
    Serial.print((int)tacho.getVirtualRPM());
    Serial.print(F(" RPM ("));
    Serial.print((int)tacho.getOutputFreqHz());
    Serial.print(F(" Hz) | Turns: "));

    switch (lighting.getTurnState()) {
        case TurnState::LEFT:    Serial.print(F("LEFT")); break;
        case TurnState::RIGHT:   Serial.print(F("RIGHT")); break;
        case TurnState::HAZARDS: Serial.print(F("HAZARDS")); break;
        case TurnState::OFF:     Serial.print(F("OFF")); break;
    }

    if (lighting.isHeatedSeatOn()) {
        Serial.print(F(" | SEAT: ON"));
    }
    Serial.println();
}

void SerialCli::printDetailedStatus() {
    Serial.println();
    Serial.println(F("--- Detailed System Diagnostics ---"));
    Serial.print(F("  4s LFP Voltage:    ")); Serial.print(powerMon.getVoltageVolts(), 2);
    Serial.print(F(" V (Raw: ")); Serial.print(powerMon.getVoltageMV()); Serial.println(F(" mV)"));
    Serial.print(F("  Low-Voltage Alert: ")); Serial.println(powerMon.isLowVoltageAlert() ? F("ACTIVE (<11.6V)") : F("NORMAL"));
    Serial.print(F("  ISA Shunt Current: ")); Serial.print(isaBus.getCurrentAmps(), 2); Serial.println(F(" A"));
    Serial.print(F("  ISA CAN Packets:   ")); Serial.println(isaBus.getPacketCount());
    Serial.print(F("  ISA Frame Age:     ")); Serial.print(isaBus.getLastFrameAgeMs()); Serial.println(F(" ms"));
    Serial.print(F("  Tachometer Target: ")); Serial.print((int)tacho.getTargetRPM()); Serial.println(F(" RPM"));
    Serial.print(F("  Tachometer Virtual:")); Serial.print((int)tacho.getVirtualRPM()); Serial.println(F(" RPM"));
    Serial.print(F("  Tachometer Freq:   ")); Serial.print(tacho.getOutputFreqHz(), 1); Serial.println(F(" Hz"));
    Serial.print(F("  Switches Raw:      L=")); Serial.print(inputs.isTurnLeftActive());
    Serial.print(F(", R=")); Serial.print(inputs.isTurnRightActive());
    Serial.print(F(", Starter=")); Serial.println(inputs.isStarterPressed());
    Serial.print(F("  Heated Seat (HOUT6): ")); Serial.println(lighting.isHeatedSeatOn() ? F("ON") : F("OFF"));
    Serial.println(F("-----------------------------------"));
}


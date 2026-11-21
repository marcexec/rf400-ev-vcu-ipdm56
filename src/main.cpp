/*
 * ==============================================================================
 * Suzuki RF400 EV - Main VCU Firmware Entry Point
 * ==============================================================================
 * Target Hardware: iPDM56 v1.1 (ATmega328P @ 16MHz)
 * Architecture: Non-blocking cooperative scheduler running directly on the
 *               official iPDM library API (ipdm::setup(), ipdm::loop()).
 *
 * Operational Modules:
 * - cli           : Lebowski-like Serial CLI at 115200 8N1 (Press 'm' for menu)
 * - isaBus        : ISAscale IVT-S current shunt decoder on CAN1 (500 kbps), Jaguar version without voltage measurements
 * - tacho         : 2-stage startup sweep + bi-directional current meter (tone()) on 90's RPM gauge
 * - lighting      : Turn indicators (80 cpm), Hazards (100 cpm), DRL winking,
 *                   hyperflash detection (160 cpm)
 * - seatHeater    : Heated seat controller (HOUT6), always OFF on boot
 * - powerMon      : 4s LiFePO4 battery monitoring with 3s hysteresis alert (<11.6V)
 * - inputs        : Handlebar turn switch polling + Starter button OneButton logic
 * ==============================================================================
 */

#include <Arduino.h>
#include "config.h"
#include "src/ipdm_library.h"
#include "src/ipdm_util.h"
#include "src/params.h"

#include "isa_can_bus.h"
#include "tacho.h"
#include "lighting.h"
#include "seat_heater.h"
#include "power.h"
#include "inputs.h"
#include "cli.h"

void setup() {
    // Let avrdude verify protocol complete without serial collision on upload
    delay(500);

    // 1. Initialize Serial CLI first so startup messages are visible immediately
    cli.begin();

    // 2. Initialize official iPDM library hardware (I2C expanders, rails, watchdog)
    ipdm::setup();

    // 3. Initialize application modules
    isaBus.begin();
    tacho.begin();
    lighting.begin();
    seatHeater.begin();
    powerMon.begin();
    inputs.begin();

    Serial.println(F("[VCU] Suzuki RF400 EV Control Unit Online."));
}

void loop() {
    // Housekeeping: refresh expander states and reset hardware watchdog
    ipdm::loop();

    // 1. Update CAN communications from ISAscale shunt
    isaBus.update();

    // 2. Sample 4s LFP auxiliary rail voltage
    powerMon.update();

    // 3. Poll physical handlebar switches and starter button
    inputs.update();

    // 4. Update exterior lights and dashboard indicators
    lighting.update(powerMon.isLowVoltageAlert(), isaBus.isHealthy());

    // 5. Update analog tachometer needle position
    tacho.update(isaBus.getCurrentMA(), isaBus.isHealthy());

    // 6. Handle serial CLI diagnostics and test commands
    cli.update();
}

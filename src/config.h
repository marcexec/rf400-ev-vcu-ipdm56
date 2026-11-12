/*
 * ==============================================================================
 * Suzuki RF400 EV Conversion - iPDM56 v1.1 Configuration & Pin Mapping
 * ==============================================================================
 * Hardware: iPDM56 v1.1 (ATmega328P @ 16MHz, Dual PCA9539PW expanders @ 0x74, 0x75)
 * Reference: https://github.com/celeron55/ipdm56
 *
 * NOTE FOR GARAGE / BENCH TESTING:
 * All pin mappings, flasher cadences, current thresholds, and calibration
 * values are defined here. Keep this file referenced when wiring the harness!
 * ==============================================================================
 */

#pragma once
#include <Arduino.h>
#include "ipdm_version.h"
#include "src/ipdm_io.h"

// ==============================================================================
// 1. PIN ALLOCATIONS - MATRIX INPUTS (Protected via TVS + Resistor + Cap)
// ==============================================================================
// Matrix Pin M1: Native MCU Pin 14 (A0) - Dedicated to Tachometer Pulse Output
constexpr int PIN_TACHO_OUT       = 14;           // A0: Suzuki RF400 tacho driven via tone()

// Matrix Pin M2: Native MCU Pin 15 (A1) - Handlebar Starter Momentary Button
// Single click = UI / Display select, Triple click = Hazards, Long press = Heated seat
constexpr int PIN_STARTER_BTN     = A1;

// Matrix Pins M3 & M4: Native MCU Pins 16 & 17 (A2, A3) - Handlebar Turn Switch
// Standard 3-position latching switch (Left / OFF / Right), switches to GND
constexpr int PIN_TURN_SW_L       = A2;           // Active-LOW (INPUT_PULLUP)
constexpr int PIN_TURN_SW_R       = A3;           // Active-LOW (INPUT_PULLUP)

// Matrix Pin M6: Native MCU Pin 2 (D2 / INT0) - Side Stand Sensor (Stage 2)
constexpr int PIN_SIDE_STAND_SW   = 2;            // Active-LOW when stand is DOWN

// ==============================================================================
// 2. PIN ALLOCATIONS - HIGH-SIDE MOSFET OUTPUTS (+12V Switched, 3A PPTC)
// ==============================================================================
// Left and Right Front Position / DRL lights (White LEDs on Lucifer mirrors + indicators)
constexpr int PIN_FRONT_DRL_L     = ipdm::HOUT4;  // ED31: Left DRL (+12V, winks off on left turn)
constexpr int PIN_FRONT_DRL_R     = ipdm::HOUT9;  // ED14: Right DRL (+12V, winks off on right turn)

// Heated Seat Pad (+12V Switched, toggled via Starter Button Long Press)
// Failsafe: ALWAYS OFF on boot!
constexpr int PIN_HEATED_SEAT_PWR = ipdm::HOUT6;  // ED22: Heated seat power (+12V)

// Left and Right Turn Signal Power (Current-monitored channels)
constexpr int PIN_TURN_LIGHT_L    = ipdm::HOUT7;  // ED12: Left turn circuit (+12V)
constexpr int PIN_TURN_LIGHT_R    = ipdm::HOUT8;  // ED13: Right turn circuit (+12V)

// Hardware Current Monitor Feedback Inputs (from internal PCA9539 expander pins)
constexpr int PIN_SENSE_TURN_L    = ipdm::HOUT7_MONITOR; // ED8: High-side monitor L
constexpr int PIN_SENSE_TURN_R    = ipdm::HOUT8_MONITOR; // ED9: High-side monitor R

// ==============================================================================
// 3. PIN ALLOCATIONS - LOW-SIDE OUTPUTS (Ground Switched, PPTC Protected)
// ==============================================================================
// Instrument Cluster Warning and Indicator Lamps
constexpr int PIN_DASH_TURN_LED   = ipdm::LOUT1;  // ED21: Dash Green Turn Arrow LED (80/100/160 cpm)
constexpr int PIN_DASH_BACKLIGHT  = ipdm::LOUT2;  // ED20: Cluster Backlight Illumination
constexpr int PIN_DASH_SIDESTAND  = ipdm::LOUT3;  // ED19: Dash Amber Side-Stand Lamp (Stage 2)
constexpr int PIN_DASH_READY_LED  = ipdm::LOUT4;  // ED18: Repurposed OEM Neutral Lamp (Green = READY)
constexpr int PIN_EV_FAULT_LED    = ipdm::LOUT5;  // ED17: Repurposed OEM Oil Lamp (Red = Fault/CAN drop)

// ==============================================================================
// 4. TACHOMETER & ISASCALE IVT-S SHUNT CALIBRATION
// ==============================================================================
// CAN Bus Configuration
constexpr uint32_t ISA_CAN_BAUD_RATE    = 500000; // 500 kbps (CAN_500KBPS)
constexpr uint32_t ISA_CAN_FRAME_ID     = 0x3C3;  // ISAscale IVT-S current frame
constexpr uint8_t  ISA_CAN_MUX_CURRENT  = 0x0C;   // Current measurement multiplexer byte

// Gauge Scaling & Physical Zero Offset
constexpr float TACHO_MA_TO_RPM_RATIO   = 50.0f;  // 200mA traction current = 10,000 RPM deflection
constexpr int   TACHO_MAX_RPM           = 16000;  // Full scale dial deflection limit
constexpr float TACHO_ZERO_MARK_RPM     = 6000.0f;// Dial mark corresponding to 0 Amps
constexpr float TACHO_ZERO_OFFSET_RPM   = -350.0f;// Physical dial alignment calibration (-350 RPM)
constexpr float TACHO_CALIBRATED_ZERO   = TACHO_ZERO_MARK_RPM + TACHO_ZERO_OFFSET_RPM; // 5650 RPM dial target

// CAN Dropout / Fault Position
constexpr float TACHO_CAN_FAULT_RPM     = 1000.0f;// Needle drops here if CAN communication stalls
constexpr unsigned long ISA_TIMEOUT_MS  = 1000;   // Timeout before declaring CAN dropout (1.0 sec)

// Quadratic frequency correction curve for Suzuki RF400 tachometer meter movement:
// f_out = (COEFF_A * f_ideal^2) + (COEFF_B * f_ideal)
constexpr float TACHO_COEFF_A           = -0.00035f;
constexpr float TACHO_COEFF_B           = 0.985f;

// Exponential Moving Average (EMA) Needle Smoothing Filter
// Lower = smoother/heavier damping, Higher = faster needle response
constexpr float TACHO_FILTER_ALPHA      = 0.08f;

// ==============================================================================
// 5. LIGHTING & FLASHER CADENCE SPECIFICATIONS
// ==============================================================================
// Turn Signal Flash Rate: 80 cycles per minute (cpm) -> 750 ms period
constexpr unsigned long TURN_CYCLE_MS   = 750;    // 80 cpm
constexpr uint8_t       TURN_DUTY_PCT   = 50;     // 50% ON / 50% OFF

// Hazard Warning Flash Rate: 100 cycles per minute (cpm) -> 600 ms period
constexpr unsigned long HAZARD_CYCLE_MS = 600;    // 100 cpm
constexpr uint8_t       HAZARD_DUTY_PCT = 50;     // 50% ON / 50% OFF

// Hyperflash Flash Rate (Burnt bulb/LED detection): 160 cpm -> 375 ms period
constexpr unsigned long HYPER_CYCLE_MS  = 375;    // 160 cpm
constexpr uint8_t       HYPER_DUTY_PCT  = 50;

// Turn Indicator Current Trip Threshold (read via Lebowski CLI menu 'k')
// If current feedback on ED8/ED9 is below this during ON phase, hyperflash trips
constexpr uint16_t TURN_MIN_CURRENT_MA  = 50;     // Default fallback (tune with CLI)

// ==============================================================================
// 6. 4s LiFePO4 (LFP) AUXILIARY BATTERY VOLTAGE THRESHOLDS
// ==============================================================================
// Voltage monitored on MCU pin A7 (ipdm::VBAT_PIN) using 18k/10k divider
constexpr uint16_t LFP_LOW_VOLTAGE_MV   = 11600;  // 11.6V alert trip point (under discharge)
constexpr uint16_t LFP_RECOVERY_MV      = 12000;  // 12.0V recovery hysteresis point
constexpr unsigned long LFP_HYST_MS     = 3000;   // 3-second sustained low voltage filter


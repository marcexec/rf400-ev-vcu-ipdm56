# rf400-ev-vcu-ipdm56

Vehicle Control Unit (VCU) firmware for the **Suzuki RF400 EV conversion**, built on the open-source **iPDM56 v1.1** intelligent Power Distribution Module hardware (ATmega328P @ 16MHz + dual NXP PCA9539PW expanders).

Developed for and shared with the [OpenInverter EV community](https://openinverter.org/forum/viewtopic.php?p=55867#p55867). Based on Perttu "celeron55" Ahola's [iPDM56 hardware and firmware architecture](https://github.com/celeron55/ipdm56).

---

## Architecture: Two-Tier Modular Design

The codebase cleanly decouples generic, upstream-able iPDM framework extensions from the vehicle-specific application logic:

```
rf400-ev-vcu-ipdm56/
├── platformio.ini              # PlatformIO project config (Uno @ 115200 baud)
├── README.md                   # This handover documentation
├── docs/
│   ├── iPDM56_EV_VCU_Technical_Specification.md   # Architectural & engineering spec
│   └── WIRING_AND_OPERATION_MANUAL.md             # Harness & bench operating guide
├── src/
│   │   # ==========================================================
│   │   # TIER 1: Generic iPDM Extensions (in src/src/)
│   │   # Upstream-compatible with celeron55/ipdm56 (GPLv3)
│   │   # ==========================================================
│   ├── src/
│   │   ├── ipdm_button.h       # Generic non-blocking Button handler (debounce, multi-click, long-press)
│   │   ├── ipdm_flasher.h      # Generic non-blocking periodic oscillator / blinker
│   │   ├── ipdm_util.h         # Added EmaFilter (needle damping) & HysteresisThreshold (voltage alerts)
│   │   ├── params.cpp          # Extended with int32_t/uint32_t hysteresis comparator overloads
│   │   ├── ipdm_library.h/.cpp # Core hardware init & watchdog supervision
│   │   ├── ipdm_io.h/.cpp      # Virtual pin router for PCA9539 expanders (ED0-ED31)
│   │   ├── ipdm_can.h/.cpp     # MCP2515 SPI CAN controllers (CAN1, CAN2)
│   │   └── ...
│   │
│   │   # ==========================================================
│   │   # TIER 2: RF400 Application Layer (in src/)
│   │   # Vehicle-specific control logic
│   │   # ==========================================================
│   ├── config.h                # Master pin definitions, calibration constants, cadences
│   ├── param_def.h             # Vehicle telemetry parameters (ISA shunt, 4s LFP rail)
│   ├── inputs.h / .cpp         # Turn switch change-detection & starter multi-press
│   ├── lighting.h / .cpp       # 80/100/160 cpm blinkers, hazards, DRL winking
│   ├── seat_heater.h / .cpp    # Heated seat controller (HOUT6), failsafe OFF on boot
│   ├── tacho.h / .cpp          # 2-stage boot sweep + bi-directional current meter (tone())
│   ├── isa_can_bus.h / .cpp    # ISAscale IVT-S CAN shunt decoder + 1s timeout failsafe
│   ├── power.h / .cpp          # 4s LiFePO4 battery monitoring with 3s hysteresis alert (<11.6V)
│   ├── cli.h / .cpp            # Lebowski-style serial terminal (115200 8N1, press 'm')
│   └── main.cpp                # Cooperative non-blocking main orchestrator
```

---

## Pin Allocation & Wiring Reference

### Matrix Inputs (TVS diode + series resistor protection)
| Matrix Pin | Pin ID | Logical Signal | Function & Characteristics |
|:---|:---|:---|:---|
| **M1** | `A0` (14) | `PIN_TACHO_OUT` | Suzuki RF400 tachometer pulse wire (driven via `tone()`). |
| **M2** | `A1` (15) | `PIN_STARTER_BTN` | Handlebar momentary starter button to GND (Active-LOW, pullup).<br>• Single Click: UI / Display page select.<br>• Triple Click: Toggle Hazard warning lights.<br>• Long Press (>800ms): Toggle Heated Seat. |
| **M3** | `A2` (16) | `PIN_TURN_SW_L` | Handlebar latching turn switch Left to GND (Active-LOW). |
| **M4** | `A3` (17) | `PIN_TURN_SW_R` | Handlebar latching turn switch Right to GND (Active-LOW). |
| **M6** | `D2` (2)  | `PIN_SIDE_STAND_SW` | Side-stand switch to GND (Active-LOW when down, Stage 2). |

### High-Side Outputs (+12V Switched, 3A PPTC Fused)
| Channel | Expander Pin | Logical Signal | Load / Behavior |
|:---|:---|:---|:---|
| **`HOUT1`** | `ED28` | `PIN_DASH_BACKLIGHT` | Instrument Cluster Backlight (+12V High-Side, steady ON; pulses alert when 4s LFP auxiliary rail < 11.6V). |
| **`HOUT3`** | `ED30` | `PIN_ISA_PWR` | ISAscale IVT-S Shunt Power (+12V, auto-delayed enable 500ms after boot, CLI-controllable reset). |
| **`HOUT4`** | `ED31` | `PIN_FRONT_DRL_L` | Left Front Position / DRL (+12V, **winks OFF** on Left turn). |
| **`HOUT6`** | `ED22` | `PIN_HEATED_SEAT_PWR`| Heated Seat Element (+12V, toggled via Starter long-press; **always OFF on boot**). |
| **`HOUT7`** | `ED12` | `PIN_TURN_LIGHT_L` | Left Turn Signal Circuit (+12V, 80 cpm normal, 160 cpm hyperflash). Monitored on `ED8`. |
| **`HOUT8`** | `ED13` | `PIN_TURN_LIGHT_R` | Right Turn Signal Circuit (+12V, 80 cpm normal, 160 cpm hyperflash). Monitored on `ED9`. |
| **`HOUT9`** | `ED14` | `PIN_FRONT_DRL_R` | Right Front Position / DRL (+12V, **winks OFF** on Right turn). |

### Low-Side Outputs (Ground Switched)
| Channel | Expander Pin | Logical Signal | Load / Behavior |
|:---|:---|:---|:---|
| **`LOUT1`** | `ED21` | `PIN_DASH_TURN_LED` | Dash Turn Indicator (Green Arrow LED, flashes with active blinker/hazards). |
| **`LOUT2`** | `ED20` | *(Spare)* | Originally low-side dash backlight; freed up after migrating to high-side `HOUT1`. |
| **`LOUT3`** | `ED19` | `PIN_DASH_SIDESTAND`| Dash Side-Stand Lamp (Amber warning lamp, Stage 2). |
| **`LOUT4`** | `ED18` | `PIN_DASH_READY_LED` | Repurposed OEM Neutral Lamp (**Solid Green = VCU Armed / Ready**). |
| **`LOUT5`** | `ED17` | `PIN_EV_FAULT_LED` | Repurposed OEM Oil Lamp (**Red = CAN Comms Lost / Shunt Dropout**). |

---

## Instrument Cluster & Tachometer Behavior

The analog tachometer movement is repurposed as a bi-directional traction battery current meter:
- **Zero Amps (Center Resting Point):** Calibrated to the **6,000 RPM mark** ($6000\text{ RPM} - 350\text{ RPM offset} = 5650\text{ RPM}$ target).
- **Discharge (Traction Power):** Sweeps clockwise towards 16,000 RPM ($200\text{ mA} = 10\text{k RPM}$ scaling).
- **Regenerative Braking (Charging):** Sweeps counter-clockwise below 6,000 RPM down to zero.
- **CAN Timeout Failsafe:** If telemetry from the ISAscale IVT-S shunt stalls for >1.0s, the needle smoothly parks at **1,000 RPM** (distinguishable from 0 RPM / no power) and the Red EV Fault lamp illuminates.
- **Two-Stage Boot Sweep:**
  1. *Majestic Slow Sweep:* Smooth 0 $\rightarrow$ 16,000 RPM ascent (~2.4s), pause at peak, smooth drop to 6,000 RPM.
  2. *Crisp Ready Flick:* Snappy flick from 6,000 $\rightarrow$ 16,000 RPM and return snap, immediately arming live tracking.

---

## Lebowski-Style Serial CLI (115200 8N1)

Connect via USB to an interactive terminal at 115200 baud. Normal operation displays live 1-line telemetry:
```text
[LIVE] 4s LFP: 13.12V | ISA: +14.2A (OK) | Tacho: 6360 RPM (106 Hz) | Turns: OFF
```

Press **`m`** or **`?`** to enter the setup and diagnostic menu:
```text
=== Suzuki RF400 EV - Lebowski Diagnostic CLI ===
  0) Trigger tachometer two-stage boot sweep
  a) Toggle Left Turn signal (80 cpm)
  b) Toggle Right Turn signal (80 cpm)
  c) Toggle Hazard Warning lights (100 cpm)
  d) Toggle Heated Seat power (HOUT6)
  e) Read turn signal current feedback (ED8/ED9)
  f) Simulate ISA shunt current (+20.0 A)
  g) Simulate ISA shunt regen (-10.0 A)
  h) Simulate ISA shunt idle (0.0 A)
  i) Print detailed system diagnostics
  z) Return to live running mode
```

---

## Building & Flashing

### With PlatformIO:
```bash
pio run -t upload
pio device monitor -b 115200
```

### Resource Usage (ATmega328P @ 16MHz):
- **Flash:** 28,480 bytes / 32,256 bytes (**86.9% Full**)
- **SRAM:** 872 bytes / 2,048 bytes (**42.6% Full**, >1.1 KB free for stack)

---

## License & Attribution

- iPDM56 hardware design and original base firmware: Copyright (c) 2022-2023 Perttu "celeron55" Ahola (GPLv3).
- RF400 EV VCU application and generic extensions: GPLv3.

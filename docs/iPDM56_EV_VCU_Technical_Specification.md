# iPDM56 v1.1 EV VCU Technical Specification & Handover Document
**Target Hardware:** iPDM56 v1.1 (ATmega core + PCA9539PW I2C Expanders `U3`, `U12`)  
**Application:** EV Conversion Controller for Suzuki RF400  
**Repository Reference:** [celeron55/ipdm56](https://github.com/celeron55/ipdm56)  

---

## 1. System Overview & Scope
This document defines the hardware mapping, I/O allocations, core firmware logic rules, and staged implementation roadmap for configuring the iPDM56 v1.1 board as an Electric Vehicle Vehicle Control Unit (EV VCU) on a Suzuki RF400 motorcycle conversion. 

* **Exclusions:** Ignition and engine kill switches are hard-wired externally and out of scope for iPDM internal logic.
* **Core Philosophy:** Non-invasive, multi-stage implementation utilizing existing hardware repositories to manage lighting, instrumentation, and EV-specific feedback while replacing legacy mechanical systems.

---

## 2. Hardware Specification Sheet

| Parameter / Field | Specification Details | Source Reference |
| :--- | :--- | :--- |
| **Module Name** | iPDM56 v1.1 | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **Manufacturing Date / ID** | 2023-01-15_01 | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **Designer / Attribution** | 8dromeda Productions (8dromedaroom@gmail.com) | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **Microcontroller Core** | ATmega-based processing core (Arduino compatible environment) | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **I/O Expansion** | Dual NXP PCA9539PW 16-bit I2C low-voltage I/O expanders (`U3`, `U12`) | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **CAN Bus Transceiver** | SN65HVD230 high-speed CAN transceiver | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| **Primary System Power** | 4-series Lithium Iron Phosphate (4s LFP / LiFePO4) auxiliary rail ($12.8\text{ V}$ nominal) | External System Architecture |
| **Low-Voltage Warning Threshold** | $11.6\text{ V}$ to $12.0\text{ V}$ (derived from 4s LFP discharge curve) | External System Architecture |

---

## 3. I/O Pin & Expander Mapping Table

| Logical Signal Name | Hardware Tier | Target Pin / Address | Function & Electrical Characteristics | Source Reference |
| :--- | :--- | :--- | :--- | :--- |
| `CAN_RX` / `CAN_TX` | Native MCU | Hardware SPI / UART pins | SN65HVD230 transceiver bus communication (ISAscale IVT-S `0x3C3`, MUX `0x0C`). | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `TACHO_OUT` | Native MCU | Native Timer Pin (`tone()`) | Bi-directional tachometer drive (0 A center / 6,350 RPM baseline up to 16,000 RPM). | [Arduino Forum: iPDM56 Tacho](https://forum.arduino.cc/t/driving-analogue-temperature-gauge-and-tacho-from-90s-suzuki-using-ipdm56/1279252) |
| `ANALOG_LFP_RAIL` | Native MCU | ADC Input Channel | Auxiliary 4s LFP system voltage monitoring ($11.6\text{ V}$ warning threshold). | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `STARTER_BTN` | Opto-Isolated In | Input `IN1` (via Expander/MCU) | Handlebar starter switch input configured for multi-press `OneButton` state machine. | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `SIDE_STAND_SW` | Opto-Isolated In | Input `IN2` (via Expander/MCU) | Side-stand status sensor input (LOW = down, HIGH = up). | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `TURN_SW_L` / `R` | Opto-Isolated In | Inputs `IN3`–`IN4` | Latching handlebar indicator switch inputs. | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `DASH_BACKLIGHT` | Expander / Output | PCA9539PW Channel | Instrument cluster illumination (Supports boot fade and 4s LFP low-voltage pulse alerts). | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `DASH_TURN_LED` | Expander / Output | PCA9539PW Channel | Independent 12V dash turn indicator LED driver. | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `DASH_SIDESTAND_LED`| Expander / Output | PCA9539PW Channel | Side-stand warning indicator lamp driver. | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |
| `FRONT_MIRROR_L` / `R` | High/Medium-Side | MOSFET Output Array | KiWAV Lucifer mirrors (White running lights with side-stand fade, Yellow turn signals). | [KiWAV Lucifer Specification](https://kiwavmotors.com/en/mirrors/scooter/led-mirrors-lucifer-carbon-for-8mm-scooters-detail) |
| `REAR_LIGHTS` | High/Medium-Side | MOSFET Output Array | Rear common-ground running and brake cluster. | [GitHub: celeron55/ipdm56](https://github.com/celeron55/ipdm56) |

---

## 4. Peripheral & External Harness Specifications

* **KiWAV Lucifer Mirrors:** Dual-color LED configuration.
  * *Black Wire:* Running light / common ground (`-`)
  * *White Wire:* Position / running light power (`+`) with side-stand pulse animation.
  * *Yellow Wire:* Sequential turn signal power (`+`).
* **Handlebar Switches:** Latching mechanical switches mapped directly to opto-isolated inputs (`IN3`–`IN4`).
* **Starter Button (`OneButton` Paradigm):** Momentary pushbutton routed to `IN1`.
  * *Single Press:* Starter / ignition handshake.
  * *Triple Press:* Hazard warning lights toggle (synchronous flashing of front/rear indicators overriding latching turns).
  * *Long Press:* Auxiliary accessory toggle.
  * *Safety Inhibit:* Multi-press actions locked out when vehicle motion is detected via CAN telemetry.
* **Tachometer:** Analog gauge driven via native timer frequency (`tone()`), mapped with exponential moving average (EMA) filter and quadratic correction coefficients.

---

## 5. Core Firmware Logic & Safety Rules

1. **I2C Bus Recovery Watchdog:** Implement non-blocking timeout monitoring for PCA9539PW expanders (`U3`, `U12`). If transactions stall, execute `Wire.end()`, re-initialize bus, and refresh states.
2. **Side-Stand Animation Logic:** While `SIDE_STAND_SW` is LOW (down), front KiWAV mirror running lights execute a slow fade-in/fade-out pulse warning. Steady-state ON when raised.
3. **4s LFP Low-Voltage Alert:** Sample auxiliary rail via ADC. If voltage drops below $11.6\text{ V}$, execute a non-blocking pulse warning pattern on `DASH_BACKLIGHT`.
4. **Soft-Start Output Ramping:** Stagger activation of high-side MOSFET channels by 50–100 ms to eliminate inrush current trips.

---

## 6. Staged Implementation Plan (Lowest Disruption First)

| Stage | Disruption Level | Scope & Description | Action Items |
| :--- | :--- | :--- | :--- |
| **1** | Lowest (Bench Test & Firmware) | **Core Firmware, Safety & Recovery Loops** | • Program I2C auto-recovery / watchdog logic for PCA9539PW expanders.<br>• Program `OneButton` starter input with motion/speed lockout.<br>• Add soft-start output ramping to eliminate LED/MOSFET inrush trips.<br>• Build side-stand pulse animation, backlight boot fade, and 4s LFP low-voltage pulse alerts ($11.6\text{ V}$). |
| **2** | Low (Minor Dash Modifications) | **Dashboard Indicators & Side Stand Integration** | • Wire independent 12V turn indicator LED and side-stand warning lamp to iPDM outputs.<br>• Implement input debouncing and hyperflash current-sense detection routines. |
| **3** | Moderate (Front Wiring) | **Front Lighting & KiWAV Lucifer Mirrors** | • Install KiWAV Lucifer mirrors with external flyback protection.<br>• Map front running lights and side-stand animation logic across shared ground runs. |
| **4** | Higher (Harness Refactoring) | **Relay Bypass & Main Indicator Sequencing** | • Remove physical electronic flasher relay.<br>• Route main front/rear indicator circuits directly to iPDM outputs for software-driven flashing and hazard override modes. |

---

## 7. Background Connector Tracking (Secondary Harness)
*Items to track in the background for allocation to the secondary wiring connector (matching single iPDM56 hardware constraint):*
* Auxiliary USB power feed switching.
* Motor and inverter temperature sensor inputs.
* Auxiliary accessory switched outputs.


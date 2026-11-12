# Suzuki RF400 EV - iPDM56 v1.1 Wiring & Operation Manual

## 1. Connector Pin & Wiring Reference Table

### Matrix Inputs (Protected by TVS diodes + series resistors)
| Matrix Pin | MCU Pin | Function | Wiring & Logic |
|:---|:---|:---|:---|
| **M1** | `A0` (14) | **`PIN_TACHO_OUT`** | Suzuki RF400 tachometer pulse wire (driven via `tone()`). |
| **M2** | `A1` (15) | **`PIN_STARTER_BTN`** | Handlebar momentary starter switch to GND (Active-LOW, pull-up).<br>• Single Click: UI / Display page select.<br>• Triple Click: Toggle Hazard warning lights.<br>• Long Press (>800ms): Toggle Heated Seat. |
| **M3** | `A2` (16) | **`PIN_TURN_SW_L`** | Handlebar turn switch Left to GND (Active-LOW, pull-up). |
| **M4** | `A3` (17) | **`PIN_TURN_SW_R`** | Handlebar turn switch Right to GND (Active-LOW, pull-up). |
| **M6** | `D2` (2) | **`PIN_SIDE_STAND_SW`** | Side-stand switch to GND (Active-LOW when down, Stage 2). |

---

### High-Side Outputs (`HOUT`, +12V Switched, 3A PPTC Fused)
| Channel | Function | Wire / Component | Operational Rules |
|:---|:---|:---|:---|
| **`HOUT4`** | **`PIN_FRONT_DRL_L`** | Left Lucifer White DRL | Steady ON with ignition; **winks OFF** during Left turn. |
| **`HOUT9`** | **`PIN_FRONT_DRL_R`** | Right Lucifer White DRL | Steady ON with ignition; **winks OFF** during Right turn. |
| **`HOUT6`** | **`PIN_HEATED_SEAT_PWR`**| Heated Seat Element (+12V) | Toggled via Starter Button long-press; **always OFF on boot**. |
| **`HOUT7`** | **`PIN_TURN_LIGHT_L`** | Left Turn Circuit (+12V) | Flashes at 80 cpm (160 cpm hyperflash on burnt bulb/LED). Current sense monitored on expander pin `ED8`. |
| **`HOUT8`** | **`PIN_TURN_LIGHT_R`** | Right Turn Circuit (+12V) | Flashes at 80 cpm (160 cpm hyperflash on burnt bulb/LED). Current sense monitored on expander pin `ED9`. |

---

### Low-Side Outputs (`LOUT`, Switched Ground)
| Channel | Function | Wire / Component | Operational Rules |
|:---|:---|:---|:---|
| **`LOUT1`** | **`PIN_DASH_TURN_LED`** | Dash Turn Lamp (Green) | Flashes synchronously with active blinker or hazards. |
| **`LOUT2`** | **`PIN_DASH_BACKLIGHT`**| Cluster Backlight | Steady ON normal; pulses when 4s LFP voltage drops below 11.6V. |
| **`LOUT3`** | **`PIN_DASH_SIDESTAND`**| Dash Side-Stand Lamp | Amber warning lamp (Stage 2). |
| **`LOUT4`** | **`PIN_DASH_READY_LED`** | Repurposed OEM Neutral Lamp| Solid Green when system is READY / armed. |
| **`LOUT5`** | **`PIN_EV_FAULT_LED`** | Repurposed OEM Oil Lamp | Red Warning Lamp: illuminates if CAN communication to ISA shunt drops out (>1s timeout). |

---

## 2. Tachometer Scaling & Dual-Stage Startup Sweep

The Suzuki RF400 tachometer serves as an analog bi-directional traction battery current meter:
- **0 Amps (Center):** Calibrated to the **6,000 RPM mark** ($6000\text{ RPM} - 350\text{ RPM offset} = 5650\text{ RPM}$ target).
- **Discharge (Power):** Needle sweeps clockwise above 6,000 RPM up to 16,000 RPM ($200\text{ mA} = 10\text{k RPM}$ scaling).
- **Regen (Charging):** Needle sweeps counter-clockwise below 6,000 RPM down to zero.
- **CAN Fault / Disconnect:** Needle drops smoothly to **1,000 RPM** to alert the rider of lost telemetry.
- **Two-Stage Boot Sweep:**
  1. *Deliberate Full Sweep:* 0 $\rightarrow$ 16,000 RPM, then smooth descent to 6,000 RPM.
  2. *Ready Flick:* Fast flick from 6,000 $\rightarrow$ 16,000 RPM and snap return to 6,000 RPM, immediately arming live tracking.

---

## 3. Lebowski-like Serial CLI Menu (115200 8N1)

Plug a USB cable into your laptop (or connector pins 10, 12, 14, 16) and open a terminal (Termite, PuTTY, gtkterm, screen) at **115200 baud, 8 data bits, 1 stop bit, no parity**.

During normal operation, the terminal prints live one-line status updates every second:
```text
[LIVE] 4s LFP: 13.12V | ISA: +14.2A (OK) | Tacho: 6360 RPM (106 Hz) | Turns: OFF
```

Press **`m`** or **`?`** at any time to open the interactive setup and diagnostic menu:
```text
########################################
#  iPDM56 RF400 VCU Diagnostic Menu    #
########################################
0) Trigger gauge two-stage boot sweep
a) Toggle Left turn indicator
b) Toggle Right turn indicator
c) Toggle Hazard warning lights
d) Toggle Heated seat power
e) Read turn signal current feedback
f) Simulate ISA shunt current (+20 Amps)
g) Simulate ISA shunt current (-10 Amps regen)
h) Simulate ISA shunt current (0 Amps)
i) Detailed system I/O & ADC report
z) Return to live running mode
------> 
```


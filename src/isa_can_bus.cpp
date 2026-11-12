#include "isa_can_bus.h"
#include "src/ipdm_can.h"
#include "src/ipdm_library.h"

IsaCanBus isaBus;

// C-style bounce function required by ipdm::can_receive()
static void isa_can_callback(const CAN_FRAME &frame) {
    isaBus.handleFrame(frame);
}

void IsaCanBus::begin() {
    // Configure CAN1 interface on MCP2515
    ipdm::can1_params.speed = CAN_500KBPS;
    
    // Enable 5V switched rail so the MCP2515 and SN65HVD230 transceiver power up
    ipdm::enable_switched_5v();
    
    _lastFrameTime = 0;
    _packetCount = 0;
    _healthy = false;
}

void IsaCanBus::update() {
    // Poll the MCP2515 receiver buffer
    ipdm::can_receive(ipdm::can1, isa_can_callback);

    // Watchdog check for communication timeout
    if (_lastFrameTime > 0 && (millis() - _lastFrameTime <= ISA_TIMEOUT_MS)) {
        _healthy = true;
    } else {
        _healthy = false;
    }
}

void IsaCanBus::handleFrame(const CAN_FRAME &frame) {
    // Check for ISAscale IVT-S current frame (Default ID: 0x3C3, MUX: 0x0C)
    if (frame.id == ISA_CAN_FRAME_ID && frame.length >= 6) {
        uint8_t mux = frame.data.bytes[1];
        if (mux == ISA_CAN_MUX_CURRENT) {
            _packetCount++;
            _lastFrameTime = millis();
            _healthy = true;

            // Unpack 32-bit signed Big-Endian current value in milliamps
            uint32_t unsignedVal = ((uint32_t)frame.data.bytes[2] << 24) |
                                   ((uint32_t)frame.data.bytes[3] << 16) |
                                   ((uint32_t)frame.data.bytes[4] << 8)  |
                                   ((uint32_t)frame.data.bytes[5]);

            _currentMA = (int32_t)unsignedVal;
        }
    }
}


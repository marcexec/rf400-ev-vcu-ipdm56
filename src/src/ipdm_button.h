/*
ipdm - iPDM56 firmware
Copyright (c) 2022 Perttu "celeron55" Ahola
Copyright (c) 2026 Suzuki RF400 EV Conversion contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <Arduino.h>
#include "ipdm_io.h"

namespace ipdm
{

/**
 * Generic non-blocking button debouncer and multi-click / long-press detector.
 *
 * Designed to work with both native MCU pins and PCA9539 expander pins
 * (ED0-ED31) via ipdm::pinMode and ipdm::digitalRead.
 */
class Button
{
public:
	Button(int pin, bool active_low = true, uint16_t debounce_ms = 30,
	       uint16_t long_press_ms = 800, uint16_t multi_click_window_ms = 350)
		: m_pin(pin)
		, m_active_low(active_low)
		, m_debounce_ms(debounce_ms)
		, m_long_press_ms(long_press_ms)
		, m_multi_click_window_ms(multi_click_window_ms)
		, m_raw_state(false)
		, m_prev_state(false)
		, m_is_held(false)
		, m_press_start_ms(0)
		, m_release_ms(0)
		, m_click_count(0)
		, m_single_click_event(false)
		, m_double_click_event(false)
		, m_triple_click_event(false)
		, m_long_press_reported(false)
		, m_long_release_event(false)
	{
	}

	void begin()
	{
		ipdm::pinMode(m_pin, m_active_low ? INPUT_PULLUP : INPUT);
		m_raw_state = false;
		m_prev_state = false;
		m_is_held = false;
		m_press_start_ms = 0;
		m_release_ms = 0;
		m_click_count = 0;
		m_single_click_event = false;
		m_double_click_event = false;
		m_triple_click_event = false;
		m_long_press_reported = false;
		m_long_release_event = false;
	}

	void update()
	{
		unsigned long now = millis();
		bool raw_read = ipdm::digitalRead(m_pin);
		bool pressed = m_active_low ? (!raw_read) : raw_read;
		m_raw_state = pressed;

		// Edge detection: Transition from unpressed to pressed
		if (pressed && !m_prev_state) {
			m_press_start_ms = now;
			m_is_held = true;
			m_long_press_reported = false;
		}
		// Continuous pressed state
		else if (pressed && m_prev_state) {
			if (!m_long_press_reported && (now - m_press_start_ms >= m_long_press_ms)) {
				m_long_press_reported = true;
				m_click_count = 0; // Cancel any pending multi-click
			}
		}
		// Edge detection: Transition from pressed to unpressed
		else if (!pressed && m_prev_state) {
			m_is_held = false;
			if (m_long_press_reported) {
				m_long_release_event = true;
			} else if (now - m_press_start_ms >= m_debounce_ms) {
				// Valid debounced click
				m_click_count++;
				m_release_ms = now;
			}
		}

		// Multi-click evaluation after quiet window expires
		if (m_click_count > 0 && !pressed && (now - m_release_ms > m_multi_click_window_ms)) {
			if (m_click_count == 1) {
				m_single_click_event = true;
			} else if (m_click_count == 2) {
				m_double_click_event = true;
			} else if (m_click_count >= 3) {
				m_triple_click_event = true;
			}
			m_click_count = 0;
		}

		m_prev_state = pressed;
	}

	// Instantaneous state
	bool isPressed() const { return m_raw_state; }
	bool isLongPressed() const { return m_long_press_reported && m_is_held; }

	// Event triggers (auto-clearing on read)
	bool wasSingleClicked()
	{
		bool ev = m_single_click_event;
		m_single_click_event = false;
		return ev;
	}

	bool wasDoubleClicked()
	{
		bool ev = m_double_click_event;
		m_double_click_event = false;
		return ev;
	}

	bool wasTripleClicked()
	{
		bool ev = m_triple_click_event;
		m_triple_click_event = false;
		return ev;
	}

	bool wasLongPressed()
	{
		static bool prev_lp = false;
		bool current_lp = isLongPressed();
		bool edge = current_lp && !prev_lp;
		prev_lp = current_lp;
		return edge;
	}

	bool wasLongPressReleased()
	{
		bool ev = m_long_release_event;
		m_long_release_event = false;
		return ev;
	}

	void reset()
	{
		m_click_count = 0;
		m_single_click_event = false;
		m_double_click_event = false;
		m_triple_click_event = false;
		m_long_press_reported = false;
		m_long_release_event = false;
	}

private:
	int m_pin;
	bool m_active_low;
	uint16_t m_debounce_ms;
	uint16_t m_long_press_ms;
	uint16_t m_multi_click_window_ms;

	bool m_raw_state;
	bool m_prev_state;
	bool m_is_held;
	unsigned long m_press_start_ms;
	unsigned long m_release_ms;
	uint8_t m_click_count;

	bool m_single_click_event;
	bool m_double_click_event;
	bool m_triple_click_event;
	bool m_long_press_reported;
	bool m_long_release_event;
};

} // namespace ipdm

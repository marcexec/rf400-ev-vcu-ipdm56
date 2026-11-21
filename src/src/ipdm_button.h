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
	Button(int pin, bool active_low = true, uint16_t debounce_ms = 50,
	       uint16_t long_press_ms = 800, uint16_t multi_click_window_ms = 350)
		: m_pin(pin)
		, m_active_low(active_low)
		, m_debounce_ms(debounce_ms)
		, m_long_press_ms(long_press_ms)
		, m_multi_click_window_ms(multi_click_window_ms)
		, m_raw_state(false)
		, m_last_raw_state(false)
		, m_debounced_state(false)
		, m_last_change_ms(0)
		, m_press_start_ms(0)
		, m_release_ms(0)
		, m_click_count(0)
		, m_single_click_event(false)
		, m_double_click_event(false)
		, m_triple_click_event(false)
		, m_long_press_reported(false)
		, m_long_release_event(false)
		, m_prev_lp(false)
	{
	}

	void begin()
	{
		ipdm::pinMode(m_pin, m_active_low ? INPUT_PULLUP : INPUT);
		bool initial_raw = ipdm::digitalRead(m_pin);
		bool initial_pressed = m_active_low ? (!initial_raw) : initial_raw;

		m_raw_state = initial_pressed;
		m_last_raw_state = initial_pressed;
		m_debounced_state = initial_pressed;
		m_last_change_ms = millis();
		m_press_start_ms = 0;
		m_release_ms = 0;
		m_click_count = 0;
		m_single_click_event = false;
		m_double_click_event = false;
		m_triple_click_event = false;
		m_long_press_reported = false;
		m_long_release_event = false;
		m_prev_lp = false;
	}

	void update()
	{
		unsigned long now = millis();
		bool raw_read = ipdm::digitalRead(m_pin);
		bool pin_pressed = m_active_low ? (!raw_read) : raw_read;
		m_raw_state = pin_pressed;

		// Detect change in raw pin state
		if (pin_pressed != m_last_raw_state) {
			m_last_change_ms = now;
			m_last_raw_state = pin_pressed;
		}

		// State must remain continuous and stable for >= m_debounce_ms
		if ((now - m_last_change_ms) >= m_debounce_ms) {
			if (pin_pressed != m_debounced_state) {
				m_debounced_state = pin_pressed;

				if (m_debounced_state) {
					// Clean debounced PRESS transition
					m_press_start_ms = now;
					m_long_press_reported = false;
				} else {
					// Clean debounced RELEASE transition
					unsigned long duration = now - m_press_start_ms;
					if (m_long_press_reported) {
						m_long_release_event = true;
					} else if (duration >= m_debounce_ms) {
						m_click_count++;
						m_release_ms = now;
					}
				}
			}
		}

		// Detect continuous long-press while held
		if (m_debounced_state && !m_long_press_reported) {
			if (now - m_press_start_ms >= m_long_press_ms) {
				m_long_press_reported = true;
				m_click_count = 0; // Cancel any pending multi-click
			}
		}

		// Multi-click evaluation after quiet window expires
		if (m_click_count > 0 && !m_debounced_state && (now - m_release_ms > m_multi_click_window_ms)) {
			if (m_click_count == 1) {
				m_single_click_event = true;
			} else if (m_click_count == 2) {
				m_double_click_event = true;
			} else if (m_click_count >= 3) {
				m_triple_click_event = true;
			}
			m_click_count = 0;
		}
	}

	// Instantaneous state
	bool isPressed() const { return m_debounced_state; }
	bool isLongPressed() const { return m_long_press_reported && m_debounced_state; }

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
		bool current_lp = isLongPressed();
		bool edge = current_lp && !m_prev_lp;
		m_prev_lp = current_lp;
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
		m_prev_lp = false;
	}

private:
	int m_pin;
	bool m_active_low;
	uint16_t m_debounce_ms;
	uint16_t m_long_press_ms;
	uint16_t m_multi_click_window_ms;

	bool m_raw_state;
	bool m_last_raw_state;
	bool m_debounced_state;
	unsigned long m_last_change_ms;
	unsigned long m_press_start_ms;
	unsigned long m_release_ms;
	uint8_t m_click_count;

	bool m_single_click_event;
	bool m_double_click_event;
	bool m_triple_click_event;
	bool m_long_press_reported;
	bool m_long_release_event;
	bool m_prev_lp;
};

} // namespace ipdm

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

namespace ipdm
{

/**
 * Generic non-blocking periodic flasher/oscillator.
 *
 * Generates an asymmetric or symmetric square wave output using millis()
 * timing with zero loop blocking. Designed for turn signals, hazard lights,
 * warning strobes, and dash indicator blinkers.
 */
class Flasher
{
public:
	Flasher(uint16_t period_ms = 750, uint8_t duty_pct = 50)
		: m_period_ms(period_ms)
		, m_duty_pct(duty_pct > 100 ? 100 : duty_pct)
		, m_start_ms(0)
		, m_running(false)
	{
	}

	void setCadence(uint16_t period_ms, uint8_t duty_pct = 50)
	{
		if (period_ms == 0)
			period_ms = 1;
		m_period_ms = period_ms;
		m_duty_pct = duty_pct > 100 ? 100 : duty_pct;
	}

	void start()
	{
		if (!m_running) {
			m_running = true;
			m_start_ms = millis();
		}
	}

	void stop()
	{
		m_running = false;
	}

	void reset()
	{
		m_start_ms = millis();
	}

	bool isRunning() const
	{
		return m_running;
	}

	bool getOutput() const
	{
		if (!m_running || m_period_ms == 0)
			return false;

		unsigned long elapsed = (millis() - m_start_ms) % m_period_ms;
		unsigned long on_time = ((unsigned long)m_period_ms * m_duty_pct) / 100;
		return elapsed < on_time;
	}

	uint16_t getPeriodMs() const { return m_period_ms; }
	uint8_t  getDutyPct() const { return m_duty_pct; }

private:
	uint16_t m_period_ms;
	uint8_t  m_duty_pct;
	unsigned long m_start_ms;
	bool m_running;
};

} // namespace ipdm

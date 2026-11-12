/*
ipdm - iPDM56 firmware
Copyright (c) 2022 Perttu "celeron55" Ahola

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
#include "ipdm_time.h"
#include <Arduino.h>

namespace ipdm
{

#define NUM_ELEMS(x) (sizeof (x) / sizeof (x)[0])

// Macros for reading resistor divided ADC pins directly to voltage

#define ADC_FACTOR_MV_RESISTORS(highohm, lowohm) (5000.0f / 1024.0f * ((float)(highohm) + (float)(lowohm)) / float(lowohm))
#define ADC_FACTOR16_MV_RESISTORS(highohm, lowohm) (uint32_t)(65536.0f * ADC_FACTOR_MV_RESISTORS(highohm, lowohm))
#define ADC_VAL_TO_MV_FACTOR16(adc, factor16) (uint16_t)(((uint32_t)(adc) * (factor16)) >> 16)
#define ADC_VAL_TO_MV_RESISTORS(adc, highohm, lowohm) ADC_VAL_TO_MV_FACTOR16(adc, ADC_FACTOR16_MV_RESISTORS(highohm, lowohm))
#define analogRead_mV_factor16(pin, factor16) ADC_VAL_TO_MV_FACTOR16(analogRead(pin), factor16)
#define analogRead_mV_resistors(pin, highohm, lowohm) ADC_VAL_TO_MV_RESISTORS(analogRead(pin), highohm, lowohm)

// Macros for reporting value changes on console

#define REPORT_BOOL(var) \
	{\
		static bool reported_value = false;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			if(var)\
				CONSOLE.println(F("TRUE"));\
			else\
				CONSOLE.println(F("FALSE"));\
			reported_value = var;\
		}\
	}

#define REPORT_UINT8(var) \
	{\
		static uint8_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println((uint16_t)var);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT16(var) \
	{\
		static uint16_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT32(var) \
	{\
		static uint32_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_INT8(var) \
	{\
		static int8_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_INT16(var) \
	{\
		static int16_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT16_HYS(var, hys) \
	{\
		static uint16_t reported_value = 0;\
		if(abs(((int32_t)var - (int32_t)reported_value)) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_INT16_HYS(var, hys) \
	{\
		static int16_t reported_value = 0;\
		if(abs((int16_t)((int32_t)var - (int32_t)reported_value)) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_INT32_HYS(var, hys) \
	{\
		static int32_t reported_value = 0;\
		if(abs((int32_t)((int32_t)var - (int32_t)reported_value)) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT32_HYS(var, hys) \
	{\
		static uint32_t reported_value = 0;\
		if(abs(((int32_t)var - (int32_t)reported_value)) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(var);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT16_FORMAT(var, hys, mul, unit) \
	{\
		static uint16_t reported_value = 0;\
		if(abs((int16_t)((int32_t)var - (int32_t)reported_value)) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.print((float)var * mul);\
			CONSOLE.println(F(unit));\
			reported_value = var;\
		}\
	}

#define REPORT_INT16_FORMAT(var, hys, mul, unit) \
	{\
		static int16_t reported_value = 0;\
		if(abs(var - reported_value) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.print((float)var * mul);\
			CONSOLE.println(F(unit));\
			reported_value = var;\
		}\
	}

#define REPORT_FLOAT(var, hys, mul, unit) \
	{\
		static float reported_value = 0;\
		if(abs(var - reported_value) > (hys)){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.print((float)var * mul);\
			CONSOLE.println(F(unit));\
			reported_value = var;\
		}\
	}

#define REPORT_ENUM(var, names) \
	{\
		static uint8_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			CONSOLE.println(names[var]);\
			reported_value = var;\
		}\
	}

#define REPORT_UINT16_BITMAP(var, num_bits, names) \
	{\
		static uint16_t reported_value = 0;\
		if(var != reported_value){\
			::ipdm::util_print_timestamp(CONSOLE);\
			CONSOLE.print(F(">> "#var" = "));\
			for(uint16_t i=0; i<num_bits; i++){\
				if(var & bit(i)){\
					CONSOLE.print(names[i]);\
					CONSOLE.print(F(" "));\
				}\
			}\
			CONSOLE.println();\
			reported_value = var;\
		}\
	}

#define DEBUG_PRINT_LOCATION(delay_ms) { CONSOLE.print(__FILE__ ":"); CONSOLE.println(__LINE__); delay(delay_ms); }

static uint16_t limit_uint16(uint16_t v, uint16_t min, uint16_t max)
{
	if(v < min)
		return min;
	if(v > max)
		return max;
	return v;
}

static int16_t limit_int16(int16_t v, int16_t min, int16_t max)
{
	if(v < min)
		return min;
	if(v > max)
		return max;
	return v;
}

static int32_t limit_int32(int32_t v, int32_t min, int32_t max)
{
	if(v < min)
		return min;
	if(v > max)
		return max;
	return v;
}

/**
 * Exponential Moving Average (EMA) low-pass smoothing filter.
 *
 * Filter equation: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
 * Alpha range: 0.0 (maximum damping / freeze) to 1.0 (no filtering / instant follow).
 */
class EmaFilter
{
public:
	explicit EmaFilter(float alpha = 0.1f, float initial_val = 0.0f)
		: m_alpha(alpha)
		, m_value(initial_val)
		, m_initialized(false)
	{
	}

	void setAlpha(float alpha)
	{
		if (alpha < 0.0f) alpha = 0.0f;
		if (alpha > 1.0f) alpha = 1.0f;
		m_alpha = alpha;
	}

	float getAlpha() const { return m_alpha; }

	float update(float input)
	{
		if (!m_initialized) {
			m_value = input;
			m_initialized = true;
			return m_value;
		}
		m_value += m_alpha * (input - m_value);
		return m_value;
	}

	float getValue() const { return m_value; }

	void reset(float val = 0.0f)
	{
		m_value = val;
		m_initialized = false;
	}

private:
	float m_alpha;
	float m_value;
	bool  m_initialized;
};

/**
 * Generic threshold monitor with timed hysteresis filtering.
 *
 * Prevents rapid toggling / chatter on noisy analog measurements
 * (such as battery voltage under intermittent load).
 *
 * If trip_below == true (e.g. low-voltage detection):
 *   - Trips when value < trip_threshold sustained for hold_ms
 *   - Recovers when value >= recover_threshold sustained for hold_ms
 *   - Deadband between thresholds retains current state and resets timers
 *
 * If trip_below == false (e.g. over-temperature or over-current detection):
 *   - Trips when value > trip_threshold sustained for hold_ms
 *   - Recovers when value <= recover_threshold sustained for hold_ms
 */
class HysteresisThreshold
{
public:
	HysteresisThreshold(uint16_t trip_threshold, uint16_t recover_threshold,
	                    uint16_t hold_ms, bool trip_below = true)
		: m_trip_threshold(trip_threshold)
		, m_recover_threshold(recover_threshold)
		, m_hold_ms(hold_ms)
		, m_trip_below(trip_below)
		, m_tripped(false)
		, m_trip_timer_start(0)
		, m_recover_timer_start(0)
	{
	}

	void setThresholds(uint16_t trip_thresh, uint16_t recover_thresh, uint16_t hold_ms)
	{
		m_trip_threshold = trip_thresh;
		m_recover_threshold = recover_thresh;
		m_hold_ms = hold_ms;
	}

	bool update(uint16_t current_val)
	{
		unsigned long now = millis();

		if (m_trip_below) {
			if (current_val < m_trip_threshold) {
				m_recover_timer_start = 0;
				if (!m_tripped) {
					if (m_trip_timer_start == 0) {
						m_trip_timer_start = now;
					} else if (now - m_trip_timer_start >= m_hold_ms) {
						m_tripped = true;
					}
				}
			} else if (current_val >= m_recover_threshold) {
				m_trip_timer_start = 0;
				if (m_tripped) {
					if (m_recover_timer_start == 0) {
						m_recover_timer_start = now;
					} else if (now - m_recover_timer_start >= m_hold_ms) {
						m_tripped = false;
					}
				}
			} else {
				m_trip_timer_start = 0;
				m_recover_timer_start = 0;
			}
		} else {
			if (current_val > m_trip_threshold) {
				m_recover_timer_start = 0;
				if (!m_tripped) {
					if (m_trip_timer_start == 0) {
						m_trip_timer_start = now;
					} else if (now - m_trip_timer_start >= m_hold_ms) {
						m_tripped = true;
					}
				}
			} else if (current_val <= m_recover_threshold) {
				m_trip_timer_start = 0;
				if (m_tripped) {
					if (m_recover_timer_start == 0) {
						m_recover_timer_start = now;
					} else if (now - m_recover_timer_start >= m_hold_ms) {
						m_tripped = false;
					}
				}
			} else {
				m_trip_timer_start = 0;
				m_recover_timer_start = 0;
			}
		}

		return m_tripped;
	}

	bool isTripped() const { return m_tripped; }

	void reset(bool tripped = false)
	{
		m_tripped = tripped;
		m_trip_timer_start = 0;
		m_recover_timer_start = 0;
	}

private:
	uint16_t m_trip_threshold;
	uint16_t m_recover_threshold;
	uint16_t m_hold_ms;
	bool     m_trip_below;
	bool     m_tripped;
	unsigned long m_trip_timer_start;
	unsigned long m_recover_timer_start;
};

} // namespace ipdm

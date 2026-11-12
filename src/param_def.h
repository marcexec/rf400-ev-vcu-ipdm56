/*
 * ==============================================================================
 * Suzuki RF400 EV - iPDM56 Parameter Definitions
 * ==============================================================================
 * MODULE_DEF(name, timeout_ms)
 * PARAM_DEF(module_name, name, type, default_value, report_hysteresis)
 * report_hysteresis=0: no reporting
 * ==============================================================================
 */

// ISAscale IVT-S Traction Current Shunt
MODULE_DEF(isa_shunt, 1500);
PARAM_DEF(isa_shunt, current_ma, int32_t, 0, 100);

// 4s LiFePO4 Auxiliary Battery Rail
MODULE_DEF(aux_power, 5000);
PARAM_DEF(aux_power, vbat_mv, uint16_t, 12800, 100);

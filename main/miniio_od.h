#pragma once

#include <stdint.h>

/*
    Application-owned object dictionary.

    OBJ(index, subindex, description, symbol, c_type, rx_pdo, tx_pdo, getter, setter)
*/

#define MINIIO_OD(OBJ) \
    OBJ(0x2000, 0x00, "Command counter",           command_counter,   uint32_t, 0, 1, get_command_counter, NA) \
    OBJ(0x2001, 0x00, "LED command",               led_command,       uint8_t,  1, 0, get_led_command,     set_led_command) \
    OBJ(0x2002, 0x00, "Telemetry value",           telemetry_value,   int32_t,  0, 1, get_telemetry_value, set_telemetry_value) \
    OBJ(0x2003, 0x00, "Heartbeat period ms",       heartbeat_period,  uint16_t, 0, 0, get_heartbeat_period, set_heartbeat_period)

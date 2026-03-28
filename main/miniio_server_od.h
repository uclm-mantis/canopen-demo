#pragma once

/* Same objects exposed by the server node */
OBJ(0x2000, 0x00, "Device mode",    device_mode,    uint8_t,  0, 0, get_device_mode,    set_device_mode)
OBJ(0x2001, 0x00, "Target value",   target_value,   int32_t,  1, 0, get_target_value,   set_target_value)
OBJ(0x2002, 0x00, "Actual value",   actual_value,   int32_t,  0, 1, get_actual_value,   NA)
OBJ(0x2003, 0x00, "Status flags",   status_flags,   uint16_t, 0, 1, get_status_flags,   NA)
OBJ(0x2004, 0x00, "Sample counter", sample_counter, uint32_t, 0, 0, get_sample_counter, NA)

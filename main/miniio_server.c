#include "miniio_server.h"

static uint32_t g_command_counter = 0;
static uint8_t  g_led_command = 0;
static int32_t  g_telemetry_value = 1234;
static uint16_t g_heartbeat_period = 500;

canopen_od_status_t on_get_command_counter(uint32_t *value)
{
    *value = g_command_counter;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_led_command(uint8_t *value)
{
    *value = g_led_command;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_set_led_command(uint8_t value)
{
    g_led_command = value;
    ++g_command_counter;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_telemetry_value(int32_t *value)
{
    *value = g_telemetry_value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_set_telemetry_value(int32_t value)
{
    g_telemetry_value = value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_heartbeat_period(uint16_t *value)
{
    *value = g_heartbeat_period;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_set_heartbeat_period(uint16_t value)
{
    if (value < 50) {
        return CANOPEN_OD_INVALID_VALUE;
    }
    g_heartbeat_period = value;
    return CANOPEN_OD_OK;
}


MINIIO_OD(OBJ_SERVER_DEFINE)

const canopen_server_od_entry_t miniio_server_od[] = {
    MINIIO_OD(OBJ_SERVER_OD_ENTRY)
};

const size_t miniio_server_od_len = sizeof(miniio_server_od) / sizeof(miniio_server_od[0]);

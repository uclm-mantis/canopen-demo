#include "miniio_server_callbacks.h"

#include <stdlib.h>
#include <string.h>
#include "miniio_protocol.h"

typedef struct {
    uint8_t device_mode;
    int32_t target_value;
    int32_t actual_value;
    uint16_t status_flags;
    uint32_t sample_counter;
} miniio_model_t;

static miniio_model_t g_model;

void miniio_server_model_init(void)
{
    memset(&g_model, 0, sizeof(g_model));
    g_model.device_mode = 1;
    g_model.status_flags = MINIIO_STATUS_READY | MINIIO_STATUS_AT_TARGET;
}

void miniio_server_model_step(void)
{
    int32_t err = g_model.target_value - g_model.actual_value;

    if (err > 10) {
        g_model.actual_value += 10;
    } else if (err < -10) {
        g_model.actual_value -= 10;
    } else {
        g_model.actual_value = g_model.target_value;
    }

    g_model.sample_counter++;

    g_model.status_flags = MINIIO_STATUS_READY;
    if (g_model.actual_value != g_model.target_value) {
        g_model.status_flags |= MINIIO_STATUS_TRACKING;
    } else {
        g_model.status_flags |= MINIIO_STATUS_AT_TARGET;
    }
}

int32_t miniio_server_get_actual(void)
{
    return g_model.actual_value;
}

uint16_t miniio_server_get_status(void)
{
    return g_model.status_flags;
}

uint32_t miniio_server_get_counter(void)
{
    return g_model.sample_counter;
}

canopen_od_status_t on_get_device_mode(uint8_t *value)
{
    *value = g_model.device_mode;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_set_device_mode(uint8_t value)
{
    g_model.device_mode = value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_target_value(int32_t *value)
{
    *value = g_model.target_value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_set_target_value(int32_t value)
{
    g_model.target_value = value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_actual_value(int32_t *value)
{
    *value = g_model.actual_value;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_status_flags(uint16_t *value)
{
    *value = g_model.status_flags;
    return CANOPEN_OD_OK;
}

canopen_od_status_t on_get_sample_counter(uint32_t *value)
{
    *value = g_model.sample_counter;
    return CANOPEN_OD_OK;
}

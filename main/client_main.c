#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "canopen.h"
#include "canopen_pdo_compat.h"
#include "miniio_protocol.h"

static const char *TAG = "MINIIO_CLIENT";

typedef struct __attribute__((packed)) {
    int32_t actual_value;
    uint16_t status_flags;
} miniio_tpdo1_t;

static miniio_tpdo1_t g_last_tpdo1;

static void miniio_tpdo1_handler(uint32_t cobid, void *data, void *context)
{
    miniio_tpdo1_t *dst = (miniio_tpdo1_t *)context;
    memcpy(dst, data, sizeof(*dst));
    ESP_LOGI(TAG, "TPDO1 cobid=0x%03lx actual=%ld status=0x%04x",
             (unsigned long)cobid,
             (long)dst->actual_value,
             (unsigned)dst->status_flags);
}

static void configure_remote_pdos(uint8_t node)
{
    static const uint32_t rpdo1_map[] = {
        MINIIO_MAP_TARGET_VALUE,
    };

    static const uint32_t tpdo1_map[] = {
        MINIIO_MAP_ACTUAL_VALUE,
        MINIIO_MAP_STATUS_FLAGS,
    };

    canopen_pdo_cfg_t rpdo1 = {
        .cob_id = 0,
        .transmission_type = 255,
        .inhibit_time = 0,
        .mapped = rpdo1_map,
        .mapped_count = 1,
    };
    ESP_ERROR_CHECK(canopen_pdo_configure(node, CANOPEN_PDO_RX, 1, &rpdo1));

    canopen_pdo_cfg_t tpdo1 = {
        .cob_id = 0,
        .transmission_type = 255,
        .inhibit_time = 0,
        .mapped = tpdo1_map,
        .mapped_count = 2,
    };
    ESP_ERROR_CHECK(canopen_pdo_configure(node, CANOPEN_PDO_TX, 1, &tpdo1));
}

static void demo_sdo_sequence(uint8_t node)
{
    uint32_t sdo_cobid = 0x600 + node;
    uint8_t mode = 2;
    int32_t target = 250;
    int32_t actual = 0;
    uint16_t status = 0;

    ESP_ERROR_CHECK(sdo_download(sdo_cobid, MINIIO_IDX_DEVICE_MODE, 0x00, &mode, sizeof(mode)));
    ESP_LOGI(TAG, "SDO: device_mode <- %u", (unsigned)mode);

    ESP_ERROR_CHECK(sdo_download(sdo_cobid, MINIIO_IDX_TARGET_VALUE, 0x00, &target, sizeof(target)));
    ESP_LOGI(TAG, "SDO: target_value <- %ld", (long)target);

    ESP_ERROR_CHECK(sdo_upload(sdo_cobid, MINIIO_IDX_ACTUAL_VALUE, 0x00, &actual));
    ESP_LOGI(TAG, "SDO: actual_value -> %ld", (long)actual);

    ESP_ERROR_CHECK(sdo_upload(sdo_cobid, MINIIO_IDX_STATUS_FLAGS, 0x00, &status));
    ESP_LOGI(TAG, "SDO: status_flags -> 0x%04x", (unsigned)status);
}

void app_main(void)
{
    const uint8_t remote = MINIIO_SERVER_NODE_ID;
    canopen_handler_handle_t tpdo_handle = NULL;

    canopen_init_cfg_t cfg = canopen_init_default();
    cfg.enable_dump_msg = false;
    cfg.max_delay_ms = 1000;
    ESP_ERROR_CHECK(canopen_initialize(&cfg));

    /* Optional but useful in real systems. */
    ESP_ERROR_CHECK(nmt_reset_node(remote));
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(nmt_start_remote_node(remote));
    vTaskDelay(pdMS_TO_TICKS(100));

    configure_remote_pdos(remote);
    demo_sdo_sequence(remote);

    ESP_ERROR_CHECK(canopen_pdo_subscribe(remote, 1, 0,
                                          miniio_tpdo1_handler,
                                          &g_last_tpdo1,
                                          &tpdo_handle));

    ESP_LOGI(TAG, "Client ready, streaming new targets via RPDO1");

    while (1) {
        static const int32_t targets[] = { 100, 300, -150, 0 };
        static size_t i = 0;
        canopen_pdo_payload_t p;

        canopen_pdo_payload_clear(&p);
        ESP_ERROR_CHECK(canopen_pdo_payload_put_i32(&p, targets[i]));
        ESP_ERROR_CHECK(canopen_pdo_send(remote, 1, 0, p.data, p.len));

        ESP_LOGI(TAG, "RPDO1: target_value <- %ld", (long)targets[i]);

        i = (i + 1) % (sizeof(targets) / sizeof(targets[0]));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

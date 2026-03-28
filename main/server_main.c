#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "canopen.h"
#include "canopen_server.h"
#include "canopen_pdo_compat.h"
#include "miniio_protocol.h"
#include "miniio_server_callbacks.h"

static const char *TAG = "MINIIO_SERVER";

static void server_publish_tpdo1(void)
{
    /*
       TPDO1 payload:
         - actual_value  (i32)
         - status_flags  (u16)
       Total: 6 bytes

       This demo transmits the TPDO explicitly with canopen_post().
       The mapping is still configured through canopen_pdo_configure() so the
       payload layout is documented and kept consistent.
    */
    twai_message_t msg = {
        .identifier = 0x180 + MINIIO_SERVER_NODE_ID,
        .data_length_code = 6,
        .extd = 0,
        .rtr = 0,
        .ss = 0,
        .self = 0,
        .dlc_non_comp = 0,
    };

    int32_t actual = miniio_server_get_actual();
    uint16_t status = miniio_server_get_status();

    memcpy(&msg.data[0], &actual, sizeof(actual));
    memcpy(&msg.data[4], &status, sizeof(status));

    ESP_ERROR_CHECK(canopen_post(&msg));
}

void app_main(void)
{
    static const uint32_t rpdo1_map[] = {
        MINIIO_MAP_TARGET_VALUE,
    };

    static const uint32_t tpdo1_map[] = {
        MINIIO_MAP_ACTUAL_VALUE,
        MINIIO_MAP_STATUS_FLAGS,
    };

    canopen_init_cfg_t cfg = canopen_init_default();
    cfg.enable_dump_msg = false;
    cfg.max_delay_ms = 1000;
    ESP_ERROR_CHECK(canopen_initialize(&cfg));

    miniio_server_model_init();

    /* Start the SDO server for this node. */
    ESP_ERROR_CHECK(canopen_server_start(MINIIO_SERVER_NODE_ID));

    /*
       Configure local PDO communication objects through the SDO server itself.
       This demonstrates that the custom node can expose standard CANopen PDO
       configuration objects and be configured from CANopen services.

       Depending on how your OD is split, you may prefer to run this from the
       client instead. Keeping it here makes the demo easier to understand.
    */
    canopen_pdo_cfg_t rpdo1 = {
        .cob_id = 0,
        .transmission_type = 255,
        .inhibit_time = 0,
        .mapped = rpdo1_map,
        .mapped_count = 1,
    };
    ESP_ERROR_CHECK(canopen_pdo_configure(MINIIO_SERVER_NODE_ID, CANOPEN_PDO_RX, 1, &rpdo1));

    canopen_pdo_cfg_t tpdo1 = {
        .cob_id = 0,
        .transmission_type = 255,
        .inhibit_time = 0,
        .mapped = tpdo1_map,
        .mapped_count = 2,
    };
    ESP_ERROR_CHECK(canopen_pdo_configure(MINIIO_SERVER_NODE_ID, CANOPEN_PDO_TX, 1, &tpdo1));

    ESP_LOGI(TAG, "Server node %u ready", MINIIO_SERVER_NODE_ID);

    while (1) {
        miniio_server_model_step();
        server_publish_tpdo1();

        ESP_LOGI(TAG,
                 "actual=%ld status=0x%04x counter=%lu",
                 (long)miniio_server_get_actual(),
                 (unsigned)miniio_server_get_status(),
                 (unsigned long)miniio_server_get_counter());

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

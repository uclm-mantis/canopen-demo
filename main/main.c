#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "miniio_client.h"
#include "miniio_server.h"

static const char *TAG = "MINIIO_DEMO";

#ifndef CONFIG_MINIIO_ROLE_SERVER
#define CONFIG_MINIIO_ROLE_SERVER 1
#endif

#ifndef CONFIG_MINIIO_LOCAL_NODE_ID
#define CONFIG_MINIIO_LOCAL_NODE_ID 0x11
#endif

#ifndef CONFIG_MINIIO_REMOTE_NODE_ID
#define CONFIG_MINIIO_REMOTE_NODE_ID 0x22
#endif

typedef struct __attribute__((packed)) {
    uint8_t led;
} miniio_rpdo1_t;

typedef struct __attribute__((packed)) {
    uint32_t command_counter;
    int32_t telemetry_value;
} miniio_tpdo1_t;

static void tpdo1_handler(uint32_t cobid, void *data, void *context)
{
    const miniio_tpdo1_t *tpdo = (const miniio_tpdo1_t *)data;
    (void)context;
    ESP_LOGI(TAG, "TPDO1 0x%03lx: command_counter=%lu telemetry=%ld",
             (unsigned long)cobid,
             (unsigned long)tpdo->command_counter,
             (long)tpdo->telemetry_value);
}

static void server_task(void *arg)
{
    canopen_server_t server = {0};

    CANOPEN_ERROR_CHECK(canopen_server_start(&server,
                                             CONFIG_MINIIO_LOCAL_NODE_ID,
                                             miniio_server_od,
                                             miniio_server_od_len));

    /* Configure TPDO1 on this same node using the generic API. */
    static const uint32_t tpdo1_map[] = {
        txpdo_command_counter,
        txpdo_telemetry_value,
    };

    canopen_pdo_cfg_t tpdo1_cfg = {
        .cob_id = 0,                  /* default COB-ID 0x180 + node */
        .transmission_type = 255,     /* event-driven */
        .inhibit_time = 0,
        .mapped = tpdo1_map,
        .mapped_count = 2,
    };
    CANOPEN_ERROR_CHECK(canopen_pdo_configure(CONFIG_MINIIO_LOCAL_NODE_ID, CANOPEN_PDO_TX, 1, &tpdo1_cfg));

    for (;;) {
        miniio_tpdo1_t tpdo;
        get_command_counter(CONFIG_MINIIO_LOCAL_NODE_ID, &tpdo.command_counter);
        get_telemetry_value(CONFIG_MINIIO_LOCAL_NODE_ID, &tpdo.telemetry_value);

        CANOPEN_ERROR_CHECK(canopen_pdo_send(CONFIG_MINIIO_LOCAL_NODE_ID,
                                             1,
                                             0x180u + CONFIG_MINIIO_LOCAL_NODE_ID,
                                             &tpdo,
                                             sizeof(tpdo)));

        ESP_LOGI(TAG, "Server published TPDO1: command_counter=%lu telemetry=%ld",
                 (unsigned long)tpdo.command_counter, (long)tpdo.telemetry_value);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void client_task(void *arg)
{
    uint8_t remote = CONFIG_MINIIO_REMOTE_NODE_ID;

    /* Configure remote RPDO1 so index 0x2001:00 (led_command) is mapped. */
    static const uint32_t rpdo1_map[] = {
        rxpdo_led_command,
    };
    canopen_pdo_cfg_t rpdo1_cfg = {
        .cob_id = 0,
        .transmission_type = 255,
        .inhibit_time = 0,
        .mapped = rpdo1_map,
        .mapped_count = 1,
    };
    CANOPEN_ERROR_CHECK(canopen_pdo_configure(remote, CANOPEN_PDO_RX, 1, &rpdo1_cfg));

    CANOPEN_ERROR_CHECK(canopen_pdo_subscribe(remote, 1, 0, tpdo1_handler, NULL, &(canopen_handler_handle_t){0}));

    for (uint8_t led = 0;; led ^= 1u) {
        uint16_t heartbeat_ms = 0;
        uint8_t current_led = 0;
        int32_t telemetry = 0;

        /* High-level typed SDO client API */
        CANOPEN_ERROR_CHECK(set_led_command(remote, led));
        CANOPEN_ERROR_CHECK(get_led_command(remote, &current_led));
        CANOPEN_ERROR_CHECK(get_telemetry_value(remote, &telemetry));

        /* Low-level SDO client API */
        heartbeat_ms = 750;
        CANOPEN_ERROR_CHECK(sdo_download(0x600u + remote, 0x2003, 0x00, &heartbeat_ms, sizeof(heartbeat_ms)));
        heartbeat_ms = 0;
        CANOPEN_ERROR_CHECK(sdo_upload(0x600u + remote, 0x2003, 0x00, &heartbeat_ms));

        /* PDO client API */
        miniio_rpdo1_t rpdo = { .led = led };
        CANOPEN_ERROR_CHECK(canopen_pdo_send(remote, 1, 0, &rpdo, sizeof(rpdo)));

        ESP_LOGI(TAG, "Client: SDO set/get led=%u telemetry=%ld heartbeat=%u; RPDO1 sent",
                 current_led, (long)telemetry, heartbeat_ms);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting MINIIO demo, local node ID=0x%02x remote node ID=0x%02x",
             CONFIG_MINIIO_LOCAL_NODE_ID, CONFIG_MINIIO_REMOTE_NODE_ID);
    canopen_init_cfg_t cfg = canopen_init_default();
    cfg.can_rx_pin = 20;
    cfg.can_tx_pin = 21;

    CANOPEN_ERROR_CHECK(canopen_initialize(&cfg));

#if CONFIG_MINIIO_ROLE_SERVER
    xTaskCreate(server_task, "miniio_server", 4096, NULL, 5, NULL);
#else
    xTaskCreate(client_task, "miniio_client", 4096, NULL, 5, NULL);
#endif
}

#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_mac.h"
#include "argtable3/argtable3.h"

#include "canopen.h"
#include "canopen_console.h"
#include "canopen_server.h"
#include "miniio_server.h"
#include "miniio_od.h"

static const char *TAG = "MINIIO_DEMO";

typedef struct __attribute__((packed)) {
    uint32_t command_counter;
    int32_t telemetry_value;
} miniio_tpdo1_t;

static uint8_t g_local_node_id;
static canopen_server_t g_server;
static TaskHandle_t g_tpdo_task = NULL;
static uint8_t g_tpdo_dest_node = 0;
static canopen_handler_handle_t g_rpdo_handle = NULL;

static uint8_t node_id_from_mac(void)
{
    uint8_t mac[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    uint8_t node_id = (uint8_t)(mac[5] & 0x7Fu);
    if (node_id == 0u) {
        node_id = 1u;
    }
    return node_id;
}

static void rpdo1_dump_handler(uint32_t cobid, void *data, void *context)
{
    const miniio_tpdo1_t *pdo = (const miniio_tpdo1_t *)data;
    (void)context;

    ESP_LOGI(TAG,
             "RPDO1 0x%03" PRIx32 ": command_counter=%" PRIu32 " telemetry=%" PRId32,
             cobid,
             pdo->command_counter,
             pdo->telemetry_value);
}

static void tpdo_task(void *arg)
{
    uint8_t dest_node = (uint8_t)(uintptr_t)arg;

    for (;;) {
        miniio_tpdo1_t pdo;
        uint32_t counter = 0;
        int32_t telemetry = 0;

        on_get_command_counter(&counter);
        on_get_telemetry_value(&telemetry);

        pdo.command_counter = counter;
        pdo.telemetry_value = telemetry;

        esp_err_t err = canopen_pdo_send(dest_node, 1, 0, &pdo, sizeof(pdo));
        if (err != ESP_OK) {
            ESP_LOGE(TAG,
                     "TPDO send to node 0x%02x failed",
                     (unsigned)dest_node);
        } else {
            ESP_LOGI(TAG,
                     "TPDO->node 0x%02x: command_counter=%" PRIu32 " telemetry=%" PRId32,
                     (unsigned)dest_node,
                     pdo.command_counter,
                     pdo.telemetry_value);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static int cmd_server(int argc, char **argv)
{
    if (argc != 2) {
        print_result_error("usage: server start|stop");
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (g_server.in_use) {
            print_result_error("server_already_started");
            return 1;
        }

        esp_err_t err = canopen_server_start(&g_server,
                                             g_local_node_id,
                                             miniio_server_od,
                                             miniio_server_od_len);
        if (err != ESP_OK) {
            print_result_error("server_start_failed");
            return 1;
        }

        ESP_LOGI(TAG, "Server started on node 0x%02x", (unsigned)g_local_node_id);
        print_result_ok();
        return 0;
    }

    if (strcmp(argv[1], "stop") == 0) {
        if (!g_server.in_use) {
            print_result_error("server_not_started");
            return 1;
        }

        esp_err_t err = canopen_server_stop(&g_server);
        if (err != ESP_OK) {
            print_result_error("server_stop_failed");
            return 1;
        }

        ESP_LOGI(TAG, "Server stopped");
        print_result_ok();
        return 0;
    }

    print_result_error("usage: server start|stop");
    return 1;
}

static int cmd_tpdo(int argc, char **argv)
{
    if (argc < 2) {
        print_result_error("usage: tpdo start <dest_node>|stop");
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (argc != 3) {
            print_result_error("usage: tpdo start <dest_node>");
            return 1;
        }
        if (g_tpdo_task != NULL) {
            print_result_error("tpdo_already_started");
            return 1;
        }

        char *end = NULL;
        long node = strtol(argv[2], &end, 0);
        if (end == argv[2] || *end != '\0' || node < 1 || node > 127) {
            print_result_error("invalid_dest_node");
            return 1;
        }

        g_tpdo_dest_node = (uint8_t)node;
        BaseType_t ok = xTaskCreate(tpdo_task,
                                    "miniio_tpdo",
                                    4096,
                                    (void *)(uintptr_t)g_tpdo_dest_node,
                                    5,
                                    &g_tpdo_task);
        if (ok != pdPASS) {
            g_tpdo_task = NULL;
            g_tpdo_dest_node = 0;
            print_result_error("tpdo_start_failed");
            return 1;
        }

        ESP_LOGI(TAG,
                 "TPDO task started: local node 0x%02x -> dest node 0x%02x (COB-ID 0x%03x)",
                 (unsigned)g_local_node_id,
                 (unsigned)g_tpdo_dest_node,
                 (unsigned)(0x200u + g_tpdo_dest_node));
        print_result_ok();
        return 0;
    }

    if (strcmp(argv[1], "stop") == 0) {
        if (g_tpdo_task == NULL) {
            print_result_error("tpdo_not_started");
            return 1;
        }

        vTaskDelete(g_tpdo_task);
        g_tpdo_task = NULL;
        g_tpdo_dest_node = 0;

        ESP_LOGI(TAG, "TPDO task stopped");
        print_result_ok();
        return 0;
    }

    print_result_error("usage: tpdo start <dest_node>|stop");
    return 1;
}

static int cmd_rpdo(int argc, char **argv)
{
    if (argc != 2) {
        print_result_error("usage: rpdo start|stop");
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (g_rpdo_handle != NULL) {
            print_result_error("rpdo_already_started");
            return 1;
        }

        esp_err_t err = canopen_register_handler(0x200u + g_local_node_id,
                                                 rpdo1_dump_handler,
                                                 NULL,
                                                 &g_rpdo_handle);
        if (err != ESP_OK) {
            g_rpdo_handle = NULL;
            print_result_error("rpdo_start_failed");
            return 1;
        }

        ESP_LOGI(TAG,
                 "RPDO1 dump started on COB-ID 0x%03x for local node 0x%02x",
                 (unsigned)(0x200u + g_local_node_id),
                 (unsigned)g_local_node_id);
        print_result_ok();
        return 0;
    }

    if (strcmp(argv[1], "stop") == 0) {
        if (g_rpdo_handle == NULL) {
            print_result_error("rpdo_not_started");
            return 1;
        }

        esp_err_t err = canopen_unregister_handler(g_rpdo_handle);
        if (err != ESP_OK) {
            print_result_error("rpdo_stop_failed");
            return 1;
        }

        g_rpdo_handle = NULL;
        ESP_LOGI(TAG, "RPDO1 dump stopped");
        print_result_ok();
        return 0;
    }

    print_result_error("usage: rpdo start|stop");
    return 1;
}

static void register_extra_commands(void)
{
    const esp_console_cmd_t cmds[] = {
        {
            .command = "server",
            .help = "server start|stop",
            .hint = NULL,
            .func = &cmd_server,
            .argtable = NULL,
        },
        {
            .command = "tpdo",
            .help = "tpdo start <dest_node>|stop",
            .hint = NULL,
            .func = &cmd_tpdo,
            .argtable = NULL,
        },
        {
            .command = "rpdo",
            .help = "rpdo start|stop",
            .hint = NULL,
            .func = &cmd_rpdo,
            .argtable = NULL,
        },
    };

    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&cmds[i]));
    }
}

void app_main(void)
{
    g_local_node_id = node_id_from_mac();

    ESP_LOGI(TAG, "Starting MINIIO demo");
    ESP_LOGI(TAG, "Local MAC-derived node ID: 0x%02x (%u)",
             (unsigned)g_local_node_id,
             (unsigned)g_local_node_id);

    canopen_init_cfg_t cfg = canopen_init_default();
    cfg.can_rx_pin = 20;
    cfg.can_tx_pin = 21;

    CANOPEN_ERROR_CHECK(canopen_initialize(&cfg));

    canopen_console_register_commands();

    canopen_console_cfg_t console_cfg = CANOPEN_CONSOLE_DEFAULT();
    console_cfg.enable_usb_console = true;
    console_cfg.enable_tcp_console = false;

    CANOPEN_CONSOLE_OBJECT_DICTIONARY(MINIIO_OD, miniio_od)
    console_cfg.object_dictionary = miniio_od;
    console_cfg.object_dictionary_entries = miniio_od_len;

    canopen_console_init(&console_cfg);
    register_extra_commands();

    ESP_LOGI(TAG, "Idle by default: no SDO server, no TPDO sender, no RPDO dump");
    ESP_LOGI(TAG, "Commands: server start|stop, tpdo start <dest_node>|stop, rpdo start|stop");
}

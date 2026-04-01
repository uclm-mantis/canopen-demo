# Depuración de problemas

## Nivel TWAI/CAN

Prueba transmisor y receptor mínimos para comprobar cableado y asignación de pines.  En un nodo el emisor:

```
// Prueba de TWAI/CAN - Emisor
#include "driver/twai.h"
#include "esp_log.h"

static const char *TAG = "TX";

void app_main(void)
{
    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_20, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    twai_message_t msg = {
        .identifier = 0x123,
        .data_length_code = 2,
        .data = {0xAB, 0xCD},
    };

    while (1) {
        esp_err_t err = twai_transmit(&msg, pdMS_TO_TICKS(1000));

        twai_status_info_t st;
        twai_get_status_info(&st);

        ESP_LOGI(TAG,
                 "tx=%s state=%d tx_err=%lu rx_err=%lu tx_failed=%lu arb_lost=%lu bus_err=%lu",
                 esp_err_to_name(err),
                 st.state,
                 (unsigned long)st.tx_error_counter,
                 (unsigned long)st.rx_error_counter,
                 (unsigned long)st.tx_failed_count,
                 (unsigned long)st.arb_lost_count,
                 (unsigned long)st.bus_error_count);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

En el otro nodo el receptor:

```
// Prueba de TWAI/CAN - Receptor
#include "driver/twai.h"
#include "esp_log.h"

static const char *TAG = "RX";

void app_main(void)
{
    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_20, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    while (1) {
        twai_message_t msg;
        esp_err_t err = twai_receive(&msg, pdMS_TO_TICKS(1000));

        twai_status_info_t st;
        twai_get_status_info(&st);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "rx id=0x%03lx dlc=%d data=%02x %02x",
                     (unsigned long)msg.identifier,
                     msg.data_length_code,
                     msg.data[0],
                     msg.data[1]);
        } else {
            ESP_LOGI(TAG,
                     "rx timeout state=%d tx_err=%lu rx_err=%lu rx_missed=%lu bus_err=%lu",
                     st.state,
                     (unsigned long)st.tx_error_counter,
                     (unsigned long)st.rx_error_counter,
                     (unsigned long)st.rx_missed_count,
                     (unsigned long)st.bus_error_count);
        }
    }
}
```

## Nivel CANopen

Este ejemplo de emisor/receptor es lo más simple que se puede diseñar para probar la biblioteca.  El ejemplo está probado con dos nodos, reseteando ambos a la vez.  Cualquier ejemplo realista debería tener una forma más evolucionada de sincronizar el comienzo de las transmisiones.  Por ejemplo, con una máquina de estados de NMT.

### Protocolo común

Dos objetos, uno para encender o apagar un LED, otro para ver el número de veces que ha conmutado.

```
// app_od.h
#pragma once

#include <stdint.h>

// OBJ(index, subindex, description, symbol, type, rxpdo, txpdo, getter, setter)

#define APP_OD(OBJ) \
    OBJ(0x2000, 0x00, "Counter", counter, uint32_t, 0, 1, get_counter, NA) \
    OBJ(0x2001, 0x00, "Led command", led_command, uint8_t, 1, 0, get_led_command, set_led_command)
```


### Servidor

Declaración de getters, setters y diccionario de objetos.

``` 
// app_server.h
#pragma once

#include "canopen_server.h"
#include "app_od.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t app_counter_value(void);

APP_OD(OBJ_SERVER_DECLARE)

extern const canopen_server_od_entry_t app_server_od[];
extern const size_t app_server_od_len;

#ifdef __cplusplus
}
#endif
```

Definición de getters, setters y diccionario de objetos.

```
// app_server.c
#include "app_server.h"

static uint32_t g_counter = 0;
static uint8_t g_led_command = 0;

uint32_t app_counter_value(void)
{
    return g_counter;
}

canopen_od_status_t on_get_counter(uint32_t *value)
{
    *value = g_counter;
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
    g_counter++;
    return CANOPEN_OD_OK;
}

APP_OD(OBJ_SERVER_DEFINE)

const canopen_server_od_entry_t app_server_od[] = {
    APP_OD(OBJ_SERVER_OD_ENTRY)
};

const size_t app_server_od_len = sizeof(app_server_od) / sizeof(app_server_od[0]);
```

Programa principal. Configura PDO, arranca servidor y envía PDO cada segundo.

```
// main.c
// Servidor CANopen básico
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "canopen.h"
#include "canopen_server.h"
#include "app_server.h"

static const char *TAG = "SERVER";
static canopen_server_t server;

typedef struct __attribute__((packed)) {
    uint32_t counter;
} tpdo1_payload_t;

void app_main(void)
{
    canopen_init_cfg_t cfg = canopen_init_default();
    CANOPEN_ERROR_CHECK(canopen_initialize(&cfg));

    CANOPEN_ERROR_CHECK(canopen_server_start(&server, 2, app_server_od, app_server_od_len));
    ESP_LOGI(TAG, "SDO server started for node 2");

    for (;;) {
        tpdo1_payload_t pdo = {
            .counter = app_counter_value(),
        };

        CANOPEN_ERROR_CHECK(canopen_pdo_send(2, 1, 0x180u + 2u, &pdo, sizeof(pdo)));
        ESP_LOGI(TAG, "TPDO1 sent: counter=%lu", (unsigned long)pdo.counter);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### Cliente

Declara getter, setters y enumerados para mapeo de objetos a PDO (lado del cliente).

```
// app_client.h
#pragma once

#include "canopen_client.h"
#include "app_od.h"

#ifdef __cplusplus
extern "C" {
#endif

APP_OD(OBJ_CLIENT_DECLARE)

typedef enum {
    APP_OD(OBJ_CLIENT_DECLARE_TX_MAPPING_ENUM)
} app_tpdo_map_t;

typedef enum {
    APP_OD(OBJ_CLIENT_DECLARE_RX_MAPPING_ENUM)
} app_rpdo_map_t;

#ifdef __cplusplus
}
#endif
```

Define getters y setters.

```
// app_client.c
#include "app_client.h"

APP_OD(OBJ_CLIENT_DEFINE)
```

Programa principal. Configura PDOs, y conmuta el led de manera infinita, consultando el contador en cada iteración.

```
// main.c
// Cliente CANopen básico
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "canopen.h"
#include "app_client.h"

static const char *TAG = "CLIENT";

typedef struct __attribute__((packed)) {
    uint32_t counter;
} tpdo1_payload_t;

static void tpdo1_handler(uint32_t cobid, void *data, void *context)
{
    const tpdo1_payload_t *pdo = (const tpdo1_payload_t *)data;
    (void)context;

    ESP_LOGI(TAG, "TPDO1 received cobid=0x%03lx counter=%lu",
             (unsigned long)cobid,
             (unsigned long)pdo->counter);
}

void app_main(void)
{
    canopen_handler_handle_t sub = NULL;
    CANOPEN_ERROR_CHECK(canopen_initialize(NULL));

    static const uint32_t rpdo1_map[] = {
        rxpdo_led_command,
    };

    canopen_pdo_cfg_t rpdo1_cfg = {
        .cob_id = 0,               // usa 0x200 + node
        .transmission_type = 255, // asíncrono
        .inhibit_time = 0,
        .mapped = rpdo1_map,
        .mapped_count = 1,
    };

    CANOPEN_ERROR_CHECK(canopen_pdo_configure(2, CANOPEN_PDO_RX, 1, &rpdo1_cfg));
    CANOPEN_ERROR_CHECK(canopen_pdo_subscribe(2, 1, 0, tpdo1_handler, NULL, &sub));

    for (uint8_t led = 0;; led ^= 1u) {
        uint32_t counter = 0;

        CANOPEN_ERROR_CHECK(set_led_command(2, led));
        CANOPEN_ERROR_CHECK(get_counter(2, &counter));

        ESP_LOGI(TAG, "SDO: led_command=%u counter=%lu",
                 (unsigned)led,
                 (unsigned long)counter);

        CANOPEN_ERROR_CHECK(canopen_pdo_send(2, 1, 0, &led, sizeof(led)));
        ESP_LOGI(TAG, "RPDO1 sent: led=%u", (unsigned)led);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```

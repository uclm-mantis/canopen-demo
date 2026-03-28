#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CANOPEN_PDO_RX = 0,
    CANOPEN_PDO_TX = 1,
} canopen_pdo_dir_t;

typedef struct {
    uint32_t cob_id;
    uint8_t transmission_type;
    uint16_t inhibit_time;
    const uint32_t *mapped;
    uint8_t mapped_count;
} canopen_pdo_cfg_t;

typedef struct {
    uint8_t data[8];
    size_t len;
} canopen_pdo_payload_t;

esp_err_t canopen_pdo_configure(uint8_t node,
                                canopen_pdo_dir_t dir,
                                uint8_t pdo_num,
                                const canopen_pdo_cfg_t *cfg);

esp_err_t canopen_pdo_send(uint8_t node,
                           uint8_t rpdo_num,
                           uint32_t cob_id_override,
                           const void *data,
                           size_t len);

esp_err_t canopen_pdo_subscribe(uint8_t node,
                                uint8_t tpdo_num,
                                uint32_t cob_id_override,
                                canopen_handler_fn fn,
                                void *context,
                                canopen_handler_handle_t *out);

void canopen_pdo_payload_clear(canopen_pdo_payload_t *p);
esp_err_t canopen_pdo_payload_put_u8(canopen_pdo_payload_t *p, uint8_t v);
esp_err_t canopen_pdo_payload_put_u16(canopen_pdo_payload_t *p, uint16_t v);
esp_err_t canopen_pdo_payload_put_u32(canopen_pdo_payload_t *p, uint32_t v);
esp_err_t canopen_pdo_payload_put_i32(canopen_pdo_payload_t *p, int32_t v);

#ifdef __cplusplus
}
#endif

#pragma once

#include "canopen_client.h"
#include "miniio_od.h"

#ifdef __cplusplus
extern "C" {
#endif

MINIIO_OD(OBJ_CLIENT_DECLARE)

typedef enum {
    MINIIO_OD(OBJ_CLIENT_DECLARE_TX_MAPPING_ENUM)
} miniio_tpdo_map_t;

typedef enum {
    MINIIO_OD(OBJ_CLIENT_DECLARE_RX_MAPPING_ENUM)
} miniio_rpdo_map_t;

#ifdef __cplusplus
}
#endif

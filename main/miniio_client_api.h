#pragma once

#include "canopen_client.h"
#include "miniio_od.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_GETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DECLARE_GETTER(getter, idx, subidx, T)
MINIIO_OD(DECLARE_GETTER)
#undef DECLARE_GETTER

#define DECLARE_SETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DECLARE_SETTER(setter, idx, subidx, T)
MINIIO_OD(DECLARE_SETTER)
#undef DECLARE_SETTER

typedef enum {
#define DECLARE_TX_ENUM(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DECLARE_TX_MAPPING_ENUM(symbol, txpdo, idx, subidx, T)
    MINIIO_OD(DECLARE_TX_ENUM)
#undef DECLARE_TX_ENUM
} miniio_tpdo_map_t;

typedef enum {
#define DECLARE_RX_ENUM(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DECLARE_RX_MAPPING_ENUM(symbol, rxpdo, idx, subidx, T)
    MINIIO_OD(DECLARE_RX_ENUM)
#undef DECLARE_RX_ENUM
} miniio_rpdo_map_t;

#ifdef __cplusplus
}
#endif

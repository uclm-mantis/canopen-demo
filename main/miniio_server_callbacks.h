#pragma once

#include "canopen_server.h"
#include "miniio_od.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_SERVER_GETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_SERVER_DECLARE_GETTER(getter, T)
MINIIO_OD(DECLARE_SERVER_GETTER)
#undef DECLARE_SERVER_GETTER

#define DECLARE_SERVER_SETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_SERVER_DECLARE_SETTER(setter, T)
MINIIO_OD(DECLARE_SERVER_SETTER)
#undef DECLARE_SERVER_SETTER

#ifdef __cplusplus
}
#endif

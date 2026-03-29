#include "miniio_client_api.h"

#define DEFINE_GETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DEFINE_GETTER(getter, idx, subidx, T)
MINIIO_OD(DEFINE_GETTER)
#undef DEFINE_GETTER

#define DEFINE_SETTER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_CLIENT_DEFINE_SETTER(setter, idx, subidx, T)
MINIIO_OD(DEFINE_SETTER)
#undef DEFINE_SETTER

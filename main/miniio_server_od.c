#include "miniio_server_callbacks.h"

#define DEFINE_SERVER_GETTER_WRAPPER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_SERVER_DEFINE_GETTER_WRAPPER(getter, T)
MINIIO_OD(DEFINE_SERVER_GETTER_WRAPPER)
#undef DEFINE_SERVER_GETTER_WRAPPER

#define DEFINE_SERVER_SETTER_WRAPPER(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_SERVER_DEFINE_SETTER_WRAPPER(setter, T)
MINIIO_OD(DEFINE_SERVER_SETTER_WRAPPER)
#undef DEFINE_SERVER_SETTER_WRAPPER

const canopen_server_od_entry_t miniio_server_od[] = {
#define MAKE_SERVER_OD_ENTRY(idx, subidx, desc, symbol, T, rxpdo, txpdo, getter, setter) \
    CANOPEN_SERVER_OD_ENTRY(idx, subidx, T, getter, setter),
    MINIIO_OD(MAKE_SERVER_OD_ENTRY)
#undef MAKE_SERVER_OD_ENTRY
};

const size_t miniio_server_od_len = sizeof(miniio_server_od) / sizeof(miniio_server_od[0]);

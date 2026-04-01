#pragma once

#include "canopen_server.h"
#include "miniio_od.h"

#ifdef __cplusplus
extern "C" {
#endif

MINIIO_OD(OBJ_SERVER_DECLARE)

extern const canopen_server_od_entry_t miniio_server_od[];
extern const size_t miniio_server_od_len;

#ifdef __cplusplus
}
#endif

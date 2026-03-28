#pragma once

#include <stdint.h>
#include "canopen_server.h"

#ifdef __cplusplus
extern "C" {
#endif

void miniio_server_model_init(void);
void miniio_server_model_step(void);
int32_t miniio_server_get_actual(void);
uint16_t miniio_server_get_status(void);
uint32_t miniio_server_get_counter(void);

/* Pull in typed callback declarations generated from the OD table. */
#define OBJ(id,sid,d,i,t,rp,tp,r,w) GENERATE_SERVER_GETTER_DECL(r, t)
#include "miniio_server_od.h"
#undef OBJ

#define OBJ(id,sid,d,i,t,rp,tp,r,w) GENERATE_SERVER_SETTER_DECL(w, t)
#include "miniio_server_od.h"
#undef OBJ

#ifdef __cplusplus
}
#endif

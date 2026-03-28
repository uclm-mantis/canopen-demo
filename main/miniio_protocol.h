#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINIIO_SERVER_NODE_ID 0x0A
#define MINIIO_CLIENT_NODE_ID 0x01

#define MINIIO_IDX_DEVICE_MODE     0x2000
#define MINIIO_IDX_TARGET_VALUE    0x2001
#define MINIIO_IDX_ACTUAL_VALUE    0x2002
#define MINIIO_IDX_STATUS_FLAGS    0x2003
#define MINIIO_IDX_SAMPLE_COUNTER  0x2004

#define MINIIO_STATUS_READY        (1u << 0)
#define MINIIO_STATUS_TRACKING     (1u << 1)
#define MINIIO_STATUS_AT_TARGET    (1u << 2)

/* Standard CANopen mapping words: (index << 16) | (subindex << 8) | bit_length */
#define MINIIO_MAP_DEVICE_MODE     ((MINIIO_IDX_DEVICE_MODE    << 16) | (0u << 8) | 8u)
#define MINIIO_MAP_TARGET_VALUE    ((MINIIO_IDX_TARGET_VALUE   << 16) | (0u << 8) | 32u)
#define MINIIO_MAP_ACTUAL_VALUE    ((MINIIO_IDX_ACTUAL_VALUE   << 16) | (0u << 8) | 32u)
#define MINIIO_MAP_STATUS_FLAGS    ((MINIIO_IDX_STATUS_FLAGS   << 16) | (0u << 8) | 16u)
#define MINIIO_MAP_SAMPLE_COUNTER  ((MINIIO_IDX_SAMPLE_COUNTER << 16) | (0u << 8) | 32u)

#ifdef __cplusplus
}
#endif

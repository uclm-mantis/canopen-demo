# CANopen demo project: custom protocol over CAN

Small ESP-IDF example showing how to use this CANopen library to implement a **custom application protocol** on top of CANopen, using both:

- **SDO** for configuration/parameter access.
- **PDO** for cyclic or event-driven process data.
- **SDO server** to expose a custom object dictionary on one node.
- **SDO client** and **PDO client helpers** on the other node.

The example is intentionally small and uses a toy protocol called **MiniIO**.

## Demo topology

Use two ESP32 boards connected to the same CAN bus:

- **Server node**: Node-ID `0x0A`
- **Client node**: Node-ID `0x01`

Both run the same CANopen core library. The difference is only the application built on top.

## MiniIO protocol

The server exposes these custom CANopen objects:

| Index  | Sub | Name               | Type    | Access | Purpose |
|--------|-----|--------------------|---------|--------|---------|
| 0x2000 | 0   | `device_mode`      | `u8`    | RW     | Working mode selected by client via SDO |
| 0x2001 | 0   | `target_value`     | `i32`   | RW     | Desired setpoint written by client via SDO or RPDO |
| 0x2002 | 0   | `actual_value`     | `i32`   | RO     | Current simulated process value |
| 0x2003 | 0   | `status_flags`     | `u16`   | RO     | Bit flags published in TPDO |
| 0x2004 | 0   | `sample_counter`   | `u32`   | RO     | Increments periodically |

### SDO usage in the demo

The client uses SDO to:

1. Set `device_mode`
2. Set an initial `target_value`
3. Read back `actual_value`
4. Read back `status_flags`

### PDO usage in the demo

The demo configures:

- **RPDO1** on the server side to receive `target_value` (`0x2001:0`, `i32`)
- **TPDO1** on the server side to publish:
  - `actual_value` (`0x2002:0`, `i32`)
  - `status_flags` (`0x2003:0`, `u16`)

The client then:

- sends new targets using `canopen_pdo_send()`
- subscribes to TPDO1 using `canopen_pdo_subscribe()`

## File layout

```text
canopen_demo_project/
├── CMakeLists.txt
├── sdkconfig.defaults
└── main/
    ├── CMakeLists.txt
    ├── miniio_protocol.h
    ├── miniio_client_od.h
    ├── miniio_server_od.h
    ├── miniio_server_callbacks.c
    ├── miniio_server_callbacks.h
    ├── canopen_pdo_compat.h
    ├── server_main.c
    └── client_main.c
```

## Important note about the current library state

In the code snapshot you shared, the PDO helper implementation exists in `canopen.c`, but the corresponding declarations are not yet present in the public `canopen.h`. Because of that, this example includes a local compatibility header:

- `main/canopen_pdo_compat.h`

Once the public header is updated, that file can be removed and the application can include only `canopen.h` / `canopen_client.h` / `canopen_server.h`.

## How to build

### 1. Put the CANopen library in `components/epos`

For example:

```bash
git submodule add https://github.com/uclm-mantis/epos.git components/epos
```

### 2. Build either the server or the client app

This demo provides **two alternative `app_main()` files**. Build one at a time.

#### Server build

Rename or select:

- `server_main.c` as the active `app_main()`
- exclude `client_main.c`

#### Client build

Rename or select:

- `client_main.c` as the active `app_main()`
- exclude `server_main.c`

A simple way is to keep one file named `app_main.c` and the other renamed to `*.off`.

## Server behaviour

The server:

- initializes CANopen
- starts an SDO server for node `0x0A`
- keeps a small simulated plant running:
  - `actual_value` moves gradually toward `target_value`
  - `sample_counter` increments every cycle
  - `status_flags` reflects simple state bits
- emits TPDO1 periodically with `actual_value` and `status_flags`

## Client behaviour

The client:

- initializes CANopen
- configures server RPDO1 and TPDO1 through SDO using `canopen_pdo_configure()`
- writes initial configuration through generated SDO client functions
- subscribes to TPDO1
- periodically changes the remote target using RPDO1
- prints TPDO updates as they arrive

## Suggested CAN wiring

- shared CANH/CANL
- common ground
- one 120 ohm terminator at each bus end

## Why this demo is useful

It demonstrates the intended layering:

- **TWAI/CAN**: transport only
- **CANopen core**: SDO, NMT, PDO, handlers, wait primitives
- **application protocol**: your own object dictionary and your own data model

That is exactly the pattern you need for protocols such as RAFT or any other custom distributed service.

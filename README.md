# MiniIO demo

This demo shows the intended **simple everyday usage** after refactoring the library:

- the **library stays generic**,
- the **application owns the object dictionary**,
- the application generates:
  - typed **SDO client** getters/setters,
  - the **SDO server OD table**,
  - protocol-specific callbacks,
- and the application uses the generic **PDO API** directly.

## What it demonstrates

### Server node

- Starts a generic SDO server with:

```c
canopen_server_t server = {0};
canopen_server_start(&server, LOCAL_NODE_ID, miniio_server_od, miniio_server_od_len);
```

- Exposes a custom OD:
  - `0x2000:00` command counter
  - `0x2001:00` LED command
  - `0x2002:00` telemetry value
  - `0x2003:00` heartbeat period

- Configures and publishes TPDO1 with:
  - command counter
  - telemetry value

### Client node

- Uses generated typed SDO functions such as:
  - `set_led_command()`
  - `get_led_command()`
  - `get_telemetry_value()`

- Uses low-level SDO too:
  - `sdo_download()`
  - `sdo_upload()`

- Configures remote RPDO1
- Sends RPDO1
- Subscribes to remote TPDO1

## Build modes

Use menuconfig or edit defaults:

- **Server**:
  - `CONFIG_MINIIO_ROLE_SERVER=y`
  - `CONFIG_MINIIO_LOCAL_NODE_ID=17`

- **Client**:
  - `CONFIG_MINIIO_ROLE_SERVER=n`
  - `CONFIG_MINIIO_LOCAL_NODE_ID=34`
  - `CONFIG_MINIIO_REMOTE_NODE_ID=17`

## Dependency on `epos`

The demo includes `main/idf_component.yml` with a Git dependency.

For local development you can replace it with a path dependency:

```yaml
dependencies:
  idf: ">=5.0"
  epos:
    path: ../../epos
```

That is the recommended setup while iterating on the library and the demo side by side.

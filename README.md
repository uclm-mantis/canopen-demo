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

## Console commands

The demo adds console commands available via the UART/USB monitor.

- Start the SDO server:

```sh
server start
```

- Start transmitting TPDO to a destination node:

```sh
tpdo start <dest_node>
```

- Stop TPDO transmission:

```sh
tpdo stop
```

- Start receiving and dumping RPDO1 for the local node:

```sh
rpdo start
```

- Stop RPDO reception:

```sh
rpdo stop
```

> The local `node_id` is printed to the serial monitor. It is derived from the device MAC: `esp_read_mac(mac, ESP_MAC_WIFI_STA)` is called and the last byte is masked with `mac[5] & 0x7F`. If the result is `0`, it is replaced with `1`.

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

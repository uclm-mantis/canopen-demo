idf_component_register(
    SRCS
        "miniio_server_callbacks.c"
        "server_main.c"
        # "client_main.c"   # enable this instead of server_main.c for the client firmware
    INCLUDE_DIRS "."
    REQUIRES driver freertos esp_timer epos
)

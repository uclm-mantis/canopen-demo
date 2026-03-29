idf_component_register(SRCS
        "main.c"
        "miniio_client_api.c"
        "miniio_server_callbacks.c"
        "miniio_server_od.c"
    REQUIRES driver console esp_common
    INCLUDE_DIRS ".")

idf_component_register(SRCS
        "main.c"
        "miniio_client.c"
        "miniio_server.c"
    REQUIRES driver console esp_common
    INCLUDE_DIRS ".")

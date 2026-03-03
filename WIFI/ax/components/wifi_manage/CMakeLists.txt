idf_component_register(
    SRCS "wifi_manage.c"
    INCLUDE_DIRS "include"
    EMBED_FILES "html/index.html"
    REQUIRES esp_wifi esp_http_server nvs_flash json
    )

idf_component_register(
    SRCS "dns_server.c" "wifi_manage.c"
    INCLUDE_DIRS "include"
    EMBED_FILES "html/index.html"
    REQUIRES esp_wifi esp_http_server nvs_flash json
    )

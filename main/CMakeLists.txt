idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES
        "index.html"
        "sobre.html"
        "main_source.txt"
    EMBED_FILES
        "card1.jpg"
        "card2.jpg"
        "card3.jpg"
        "analogico.jpg"
        "digital.png"
        "eletronico.jpg"
    REQUIRES
        driver
        esp_http_server
        esp_wifi
        nvs_flash
        esp_netif
        esp_event
)

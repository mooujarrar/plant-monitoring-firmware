set(srcs
    "src/DHT_c.c"
    "temperature-humidity.c")

set(includes "include")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS ${includes}
                       REQUIRES events mqtt nvs_flash)
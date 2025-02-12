idf_component_register(
    SRCS "MP2762A.cpp" "BMS.cpp"  "main.cpp" "HAL_ESP.cpp" "BMS.cpp"
    INCLUDE_DIRS "include"
)

littlefs_create_partition_image(webfs ../webui FLASH_IN_PROJECT)
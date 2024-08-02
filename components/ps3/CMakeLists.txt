idf_component_register(SRCS "src/ps3.c" 
                            "src/ps3_spp.c" 
                            "src/ps3_parser.c" 
                            "src/ps3_l2cap.c"
                    INCLUDE_DIRS "src/include"
                    REQUIRES "bt" "nvs_flash")

# Add the extra include directories
target_include_directories(${COMPONENT_LIB} PRIVATE 
    ${IDF_PATH}/components/bt/common/include/
    ${IDF_PATH}/components/bt/host/bluedroid/common/include/
    ${IDF_PATH}/components/bt/host/bluedroid/stack/include
    ${IDF_PATH}/components/bt/host/bluedroid/stack/l2cap/include
    ${IDF_PATH}/components/bt/host/bluedroid/osi/include
    ${IDF_PATH}/components/bt/bluedroid/common/include/
    ${IDF_PATH}/components/bt/bluedroid/stack/include
    ${IDF_PATH}/components/bt/bluedroid/stack/l2cap/include
    ${IDF_PATH}/components/bt/bluedroid/osi/include
)
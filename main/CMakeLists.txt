idf_component_register( SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES "dron" "spiffs_partition")
list( APPEND EXTRA_COMPONENTS_DIRS components )

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES "dron")
list(APPEND EXTRA_COMPONENTS_DIRS components)

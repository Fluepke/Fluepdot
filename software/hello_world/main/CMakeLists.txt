idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "hello_world_main.c"
                            "font_rendering.c"
                            "ota.c"
                            "diagnostics.c"
                    INCLUDE_DIRS "." "include")

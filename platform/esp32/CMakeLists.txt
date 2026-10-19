# ESP-IDF "main" component. PlatformIO treats src_dir as this component.
FILE(GLOB_RECURSE app_sources
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../app/*.cpp)

idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS "." "../.." "../../app"
                       REQUIRES esp_driver_gpio freertos esp_adc)

# No-op for the xtensa compiler, which never defines these. VS Code IntelliSense injects host-OS
# macros when parsing, which sends ESP-IDF headers (esp_attr.h) down macOS-only paths. These flags
# land in compile_commands.json, which IntelliSense reads (see .vscode/settings.json).
target_compile_options(${COMPONENT_LIB} PRIVATE -U__APPLE__ -U__MACH__)

/* Force-included by IntelliSense only (see .vscode/settings.json), never by the real build.
 * cpptools can leak host-OS macros into the parse even though the xtensa compiler does not
 * define them, which sends ESP-IDF headers down host-only paths (e.g. esp_attr.h including
 * <mach-o/getsect.h> when __APPLE__ is set). Remove them so the parse matches the ESP32 target. */
#undef __APPLE__
#undef __MACH__
#undef _WIN32
#undef _WIN64
#undef _MSC_VER

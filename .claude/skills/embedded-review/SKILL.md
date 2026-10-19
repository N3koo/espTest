---
name: embedded-review
description: Review staged changes against this project's embedded practices for the ESP32 (ESP-IDF + FreeRTOS) and XMC1100 (bare-metal) firmware before committing. Run before every git commit (a hook blocks commits until it passes), or when asked to check code against the project's embedded rules.
---

# Embedded pre-commit review

Review the **staged** changes against the rules below. Only the staged diff is reviewed and approved, so stage everything first and never commit with `git commit -a`.

## Steps

1. Run `git diff --cached --stat` and `git diff --cached`. If nothing is staged, stop and say so.
2. For each changed file, work out which rule sections apply from its path:
   - `app/`, `interfaces/` → **Portable code** + **Both platforms**
   - `platform/esp32/` → **ESP32** + **Both platforms**
   - `platform/xmc1100/` (excluding `build/`, `libs/`, `bsps/`, `mtb_shared/`) → **XMC1100** + **Both platforms**
   - `test/` → **Both platforms** (tests may use the heap and host features)
   - Other files (`platformio.ini`, CMake, docs, tooling) → check only **Repo hygiene**
3. Read enough of the surrounding code to judge each changed hunk; don't flag code the diff didn't touch.
4. If ESP32 code or its CMake changed, run `~/.platformio/penv/bin/pio run -e esp32` and report the result. If `app/` contains sources, also run `~/.platformio/penv/bin/pio run -e native`.
5. Report findings grouped as **Blocking** (rule violations, build failures) and **Advisory** (style, suggestions), each with `file:line`, the rule broken and the fix. Fix blocking issues if the user asks, then stage the fixes and review again.
6. **Only when there are no blocking findings and the builds pass**, record the approval. This is the exact staged content the commit hook checks:
   ```sh
   git diff --cached | shasum | cut -c1-40 > "$(git rev-parse --git-dir)/embedded-review-ok"
   ```
   If anything is staged or changed after this, the hook will ask for a new review.

## Rules

### Portable code (`app/`, `interfaces/`)
- No hardware or OS headers: no `driver/*`, `esp_*`, `freertos/*`, `xmc_*`, `cybsp.h`. All hardware access goes through the interfaces.
- No heap after startup: no `new`/`delete`, `malloc`, `std::vector`, `std::string`, `std::function`, `std::map`. Use fixed-size arrays, `std::array` and static objects.
- No exceptions (`throw`/`try`), no RTTI (`dynamic_cast`, `typeid`).
- No `float`/`double` in control logic (the XMC1100 has no FPU). Use integer units, for example `int16_t` tenths of a degree.
- No `printf`-family calls with `%f`.
- Logic is tick-driven (for example `tick(uint32_t nowMs)`): no blocking delays, busy-wait loops or RTOS calls.
- Time comparisons must survive wrap-around: `now - start >= duration`, never `now >= start + duration`.
- Fixed-width integer types (`uint8_t`, `int16_t`, `uint32_t`) for data whose size matters; include `<cstdint>`.
- Interfaces keep a protected non-virtual destructor and stay small, with one purpose each.
- Two-phase init: constructors only store configuration, and `init()` touches hardware.
- Safety: outputs (heater, motor, valve) default to off; the Error state is sticky until an explicit reset; door and over-temperature are checked in every active state; out-of-range sensor values are treated as faults.
- Button and door inputs are debounced in software in `app/`.

### ESP32 (`platform/esp32/`)
- Check every `esp_err_t`: `ESP_ERROR_CHECK(...)`, or explicit handling where recovery is possible.
- Delays use `pdMS_TO_TICKS(ms)`, never `ms / portTICK_PERIOD_MS`. Periodic work uses `vTaskDelayUntil`.
- Tasks communicate through queues or task notifications, not shared globals. Mutexes only for resources that really are shared.
- No busy-waiting in tasks. Stack sizes should have a reason (a measured high-water mark or a comment).
- Interrupt handlers are short, marked `IRAM_ATTR`, and only call `...FromISR` APIs. No `ESP_LOG*`, `printf` or blocking calls inside them.
- The task watchdog is fed only after a completed control cycle, never from an independent timer.
- Logging uses `ESP_LOGx(TAG, ...)` with a per-file `TAG`, and never runs in tight loops.
- Every ESP-IDF component used is listed in `REQUIRES` in `platform/esp32/CMakeLists.txt`. A new `#include` of a driver header without a matching `REQUIRES` entry is blocking.
- Pins: no strapping pins (0, 2, 5, 12, 15) for inputs that matter at boot; never GPIO 6–11; GPIO 34–39 are input-only with no pull-ups.

### XMC1100 (`platform/xmc1100/`)
- Variables written by an interrupt are `volatile`.
- Shared data larger than 32 bits, or read-modify-write from both the interrupt and the main loop, is protected by a short `__disable_irq()`/`__enable_irq()` critical section or a single-writer ring buffer.
- No `/` or `%` in interrupt handlers or hot paths where a shift or a power of two would do (the M0 has no hardware divide).
- Lookup tables are `const` so they stay in flash.
- Virtual functions are used sparingly and never in hot loops.
- The watchdog is enabled and fed once per completed main-loop cycle. The reset cause is checked at startup.
- Peripherals are configured through XMCLib (`XMC_GPIO_Init`, etc.), not raw register writes, unless a comment says why.
- The main loop sleeps with `__WFI()` when idle.

### Both platforms
- No new compiler warnings. Watch especially for implicit narrowing (`int` to `uint8_t`), sign comparisons and shadowing.
- Identifiers name their units: `fillTimeMs`, `tempDeciC`, `periodTicks`.
- Use `static_assert` for size and count assumptions (buffer sizes, enum counts matching table sizes).
- Every `switch` on an enum has a `default` or handles all values explicitly.
- No recursion. No magic numbers for pins, timings or thresholds; use named `constexpr` values.
- New behaviour in `app/` comes with a unit test in `test/unit/`, tagged with its requirement ID (for example `// REQ-SAFE-003`) when one exists.

### Repo hygiene (any file)
- No build output or IDE state staged: `.pio/`, `platform/xmc1100/build/`, `libs/`, `.ninja_log`, `.DS_Store`, `.vscode/c_cpp_properties.json`, `.vscode/launch.json`.
- No secrets, Wi-Fi credentials or absolute user paths in source.

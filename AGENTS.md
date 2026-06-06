# Agent Notes: led-flow-pro-hal

STM32CubeMX-generated CMake project for STM32F103ZET6 (ARM Cortex-M3). Firmware lives in `Core/`, HAL/Drivers are vendored under `Drivers/`.

## Build

- Requires `arm-none-eabi-gcc` on PATH (tested with GNU Tools for STM32 14.3.rel1) and CMake >= 3.22.
- Preset-based build (Ninja generator):
  - `cmake --preset Debug`
  - `cmake --build --preset Debug`
  - Release: replace `Debug` with `Release`.
- Output: `build/Debug/led-flow-pro-hal.elf` and `.map`.
- Toolchain: `cmake/gcc-arm-none-eabi.cmake` (default). `cmake/starm-clang.cmake` exists as an alternative but is not wired into presets.

## Project structure

- `led-flow-pro-hal.ioc` — STM32CubeMX configuration source of truth. Regenerating from it will overwrite `Core/Src`, `Core/Inc`, `Drivers/`, `cmake/stm32cubemx/CMakeLists.txt`, and startup/linker files. Hand-edits in generated sections will be lost.
- `Core/Src/main.c` — application entry point. CubeMX inserts `/* USER CODE BEGIN/END ... */` guard blocks; keep custom code inside those blocks to survive regeneration.
- `Core/Src/gpio.c`, `Core/Inc/gpio.h` — GPIO init (LED1/LED2/LED3 on PA0/PA1/PA8).
- `Core/Src/stm32f1xx_it.c`, `stm32f1xx_hal_msp.c` — interrupt and MSP handlers.
- `cmake/stm32cubemx/CMakeLists.txt` — generated source list and compile definitions (`USE_HAL_DRIVER`, `STM32F103xE`).
- `startup_stm32f103xe.s` and `STM32F103XX_FLASH.ld` — startup and linker script.

## Adding code

- Add new source files via `target_sources(${CMAKE_PROJECT_NAME} PRIVATE ...)` in root `CMakeLists.txt` (line ~46), not by editing the generated `cmake/stm32cubemx/CMakeLists.txt`.
- Add include paths and macros in root `CMakeLists.txt` (lines ~51 and ~56).
- C standard is C11; the generated code rejects C90/C99 (see `cmake/stm32cubemx/CMakeLists.txt` validation).

## Verification

- There are no automated tests, lint, or CI configs in this repo.
- The only practical verification is a clean build: `cmake --build --preset Debug`.
- To inspect the binary: `arm-none-eabi-size build/Debug/led-flow-pro-hal.elf` or `arm-none-eabi-objdump -h build/Debug/led-flow-pro-hal.elf`.

## IDE / environment

- `.idea/` contains JetBrains run configurations for ST-Link debugging. Do not commit IDE-specific changes unless intended.
- `compile_commands.json` is generated in `build/Debug/`; useful for clangd/language servers.

@echo off
chcp 65001 >nul
setlocal

set FIRMWARE_DIR=ESP32-C3-MINI-1-AT-V4.1.1.0\ESP32-C3-MINI-1-AT-V4.1.1.0
set ESPTOOL=python -m esptool

if not exist "%FIRMWARE_DIR%\esp-at.bin" (
    echo 错误：找不到固件文件 %FIRMWARE_DIR%\esp-at.bin
    pause
    exit /b 1
)

set /p PORT="请输入 ESP32-C3 的串口号（例如 COM3）: "

echo.
echo 即将烧录 AT 固件到 %PORT% ...
echo 请确保 ESP32-C3 已连接到 %PORT%，并且 Boot/IO9 和 EN 引脚可控（或按住 BOOT 后按 EN 进入下载模式）。
echo.

%ESPTOOL% --chip esp32c3 --port %PORT% --baud 921600 ^
  --before default_reset --after hard_reset ^
  --flash_mode dio --flash_freq 40m --flash_size 4MB ^
  write_flash ^
  0x0      "%FIRMWARE_DIR%\bootloader\bootloader.bin" ^
  0x8000   "%FIRMWARE_DIR%\partition_table\partition-table.bin" ^
  0xd000   "%FIRMWARE_DIR%\ota_data_initial.bin" ^
  0x1e000  "%FIRMWARE_DIR%\at_customize.bin" ^
  0x1f000  "%FIRMWARE_DIR%\customized_partitions\mfg_nvs.bin" ^
  0x60000  "%FIRMWARE_DIR%\esp-at.bin"

if errorlevel 1 (
    echo.
    echo 烧录失败。如果提示无法连接，请进入下载模式后重试：
    echo 1. 按住 BOOT 键（IO9 接地）
    echo 2. 按一下 RESET/EN 键
    echo 3. 松开 BOOT 键
    echo 4. 重新运行本脚本
    pause
    exit /b 1
)

echo.
echo 烧录成功！
pause

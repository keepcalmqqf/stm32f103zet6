# 构建、CI/CD

本文档描述 `stm32f103zet6` 固件工程的构建流程、发布流程与烧录方法。

---

## 1. 环境要求

- **CMake**：>= 3.22
- **构建工具**：Ninja（CMake preset 默认使用）
- **交叉编译器**：`arm-none-eabi-gcc` 在 PATH 中
  - 推荐：GNU Tools for STM32 14.3.rel1
- **烧录工具**：STM32CubeProgrammer CLI（`STM32_Programmer_CLI`）
  - 通常位于 `D:\STM32CubeCLT_1.x.x\STM32CubeProgrammer\bin`
- **代码生成工具**：STM32CubeMX（修改 `.ioc` 时需要）

---

## 2. 构建变体

项目提供两个 CMake preset：

| Preset | 优化级别 | 调试信息 | 用途 |
|--------|----------|----------|------|
| `Debug` | `-O0 -g` | 完整 | 日常开发、调试 |
| `Release` | `-O2` | 可选 | 生产发布 |

---

## 3. 常用构建命令

```bash
# 配置并构建 Debug（默认）
cmake --preset Debug
cmake --build --preset Debug

# Release 构建
cmake --preset Release
cmake --build --preset Release

# 强制重新配置
cmake --preset Debug --fresh

# 干净构建
rm -rf build/Debug
cmake --preset Debug
cmake --build --preset Debug
```

---

## 4. 构建产物

构建完成后，产物位于：

```text
build/Debug/
├── stm32f103zet6.elf     # 可执行文件（用于调试与烧录）
├── stm32f103zet6.bin     # 二进制文件（如生成）
├── stm32f103zet6.map     # 链接器映射文件
├── stm32f103zet6.hex     # Intel HEX 文件（如生成）
└── compile_commands.json # 用于 clangd/语言服务器
```

> 当前配置默认输出 `.elf` 与 `.map`；如需 `.bin`/`.hex`，可在根 `CMakeLists.txt` 中添加 `objcopy` 自定义命令。

---

## 5. 检查固件体积

```bash
# 查看 Flash / RAM 占用
arm-none-eabi-size build/Debug/stm32f103zet6.elf

# 查看各段分布
arm-none-eabi-objdump -h build/Debug/stm32f103zet6.elf

# 查看符号表（按大小排序）
arm-none-eabi-nm --print-size --size-sort --radix=d build/Debug/stm32f103zet6.elf
```

---

## 6. ST-Link 烧录

### 6.1 单调试器

```bash
STM32_Programmer_CLI -c port=SWD -w build/Debug/stm32f103zet6.elf -v -rst
```

参数说明：

- `-c port=SWD`：通过 SWD 接口连接
- `-w <file>`：写入固件
- `-v`：校验
- `-rst`：烧录后复位运行

### 6.2 多调试器环境

```bash
# 列出所有已连接调试器
STM32_Programmer_CLI --list

# 使用指定序列号烧录
STM32_Programmer_CLI -c port=SWD sn=<serial> -w build/Debug/stm32f103zet6.elf -v -rst
```

### 6.3 仅擦除并复位

```bash
STM32_Programmer_CLI -c port=SWD -e all -rst
```

---

## 7. 发布流程

当前仓库没有自动化 CI/CD，发布以手动流程为准。

### 7.1 版本号管理

- 版本号格式：`MAJOR.MINOR.PATCH`
- 在 `main.c` 或专用版本头中定义，例如：

  ```c
  #define FIRMWARE_VERSION_MAJOR 0
  #define FIRMWARE_VERSION_MINOR 1
  #define FIRMWARE_VERSION_PATCH 0
  ```

- 发布前确保版本号已更新。

### 7.2 Release Checklist

1. 拉取最新代码，确认工作区干净。
2. 执行干净 Release 构建：

   ```bash
   rm -rf build/Release
   cmake --preset Release
   cmake --build --preset Release
   ```

3. 检查固件体积，确保未超出 Flash（512 KB）与 RAM（64 KB）限制。
4. 烧录到目标板验证功能：

   - LED 是否正常轮询
   - LCD 是否正常初始化并显示颜色
   - 串口是否有初始化日志输出

5. 备份构建产物：

   ```bash
   mkdir -p releases/v0.1.0
   cp build/Release/stm32f103zet6.elf releases/v0.1.0/
   cp build/Release/stm32f103zet6.map releases/v0.1.0/
   ```

6. 在版本控制系统中打 tag：

   ```bash
   git tag -a v0.1.0 -m "Release v0.1.0"
   ```

---

## 8. 代码生成后重建

当修改 `stm32f103zet6.ioc` 后，必须重新生成代码并构建：

```bash
# 命令行生成（需 STM32CubeMX 在 PATH）
STM32CubeMX -s scripts/cubemx_generate.script

# 或图形界面打开 .ioc 后点击 GENERATE CODE

# 重新构建
cmake --build --preset Debug
```

> 注意：重新生成会覆盖 `Core/Src`、`Core/Inc`、`Drivers/`、`cmake/stm32cubemx/CMakeLists.txt` 中由 CubeMX 生成的部分。用户代码必须位于 `/* USER CODE BEGIN/END ... */` 保护块内。

---

## 9. CI/CD 说明

- 当前仓库**没有**配置 GitHub Actions / GitLab CI / Jenkins。
- 推荐的最小 CI 流水线（供后续集成参考）：

  ```yaml
  # 示例：GitHub Actions
  steps:
    - uses: actions/checkout@v4
    - name: Install arm-none-eabi-gcc
      uses: carlosperate/arm-none-eabi-gcc-action@v1
    - name: Configure & Build
      run: |
        cmake --preset Release
        cmake --build --preset Release
    - name: Check binary size
      run: arm-none-eabi-size build/Release/stm32f103zet6.elf
  ```

---

## 10. 故障排查

| 问题 | 排查步骤 |
|------|----------|
| `cmake --preset Debug` 失败 | 确认 `arm-none-eabi-gcc` 在 PATH；确认 CMake >= 3.22；确认 Ninja 已安装 |
| 链接报错 `undefined reference` | 确认新 `.c` 文件已加入根 `CMakeLists.txt` 的 `target_sources` |
| 烧录失败 `cannot connect to target` | 检查 SWD 接线、目标板供电、BOOT0 是否接地 |
| 烧录成功但程序不运行 | 检查 `startup_stm32f103xe.s` 与链接脚本是否匹配；检查 `SystemClock_Config` 是否成功 |
| Release 固件体积暴增 | 检查是否启用了 `-O0`；检查是否包含大量字符串/常量 |

---

## 相关文档

- [架构与数据流](./ARCHITECTURE.md)
- [测试规范](./docs/TESTING.md)
- [安全与敏感信息](./docs/SECURITY.md)
- [工具链快速参考](./docs/references/arm-toolchain-reference-llms.txt)
- [技术债务追踪](./docs/exec-plans/tech-debt-tracker.md)

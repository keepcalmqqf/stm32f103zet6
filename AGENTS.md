<!-- AGENTS.md — stm32f103zet6 -->

> 面向 AI 编码助手的**项目地图**，不是百科全书。
>
> 详细信息存放在代码仓库的记录系统 `docs/` 中；本文件只提供稳定的切入点和下一步该去哪里的指引。
>
> 冲突解决优先级：**用户显式提示 > 本文件 > 子目录 AGENTS.md**。

---

## 项目是什么

- **名称**：`stm32f103zet6`
- **类型**：STM32CubeMX 生成的 CMake 裸机固件工程
- **目标 MCU**：STM32F103ZET6（ARM Cortex-M3，LQFP144，72 MHz）
- **技术栈**：STM32 HAL + CMake + Ninja + arm-none-eabi-gcc + LVGL v9
- **配置源文件**：`stm32f103zet6.ioc`

### 当前能力概览

本固件已实现：**系统启动自检、LED 状态指示、USART1 调试日志、FSMC 16-bit 8080 LCD、ILI9486 显示驱动、RTC 实时时钟、LVGL 时钟 UI、ESP32-C3 AT WiFi + NTP 同步**。

完整的能力清单、验收标准及开发中能力见 [`docs/product-specs/firmware-capabilities.md`](./docs/product-specs/firmware-capabilities.md)。

---

## 做任何事前先执行

1. 修改 `stm32f103zet6.ioc` 后 → 重新生成代码：

   ```bash
   STM32CubeMX -s scripts/cubemx_generate.script
   ```

2. 修改用户源码后 → 重新构建：

   ```bash
   cmake --build --preset Debug
   ```

3. 提交前 →

   ```bash
   cmake --build --preset Debug
   arm-none-eabi-size build/Debug/stm32f103zet6.elf
   ```

---

## 禁止做的事

- 不要手动修改 `cmake/stm32cubemx/CMakeLists.txt`、启动文件或链接脚本。
- 不要在 `Core/Src/main.c` 等生成文件的非 `/* USER CODE BEGIN/END ... */` 区域手写代码。
- 不要把新源码加到 `cmake/stm32cubemx/CMakeLists.txt`；应通过根目录 `CMakeLists.txt` 添加。
- 不要硬编码 Magic Number、延时循环或 GPIO 状态而不加注释。
- 不要把业务逻辑、WiFi 凭证、NTP 服务器等配置直接写在 `main.c`；应放到 `Core/App/` 层并由 `app_config.h` 集中管理。
- 不要随意复用 `PA13` / `PA14`（默认 SWD 调试口）。
- 不要提交 `build/`、`.idea/` 运行配置或 IDE 临时文件。

---

## 常用命令

```bash
# 构建
cmake --preset Debug
cmake --build --preset Debug

# Release
cmake --preset Release
cmake --build --preset Release

# 检查体积
arm-none-eabi-size build/Debug/stm32f103zet6.elf

# ST-Link 烧录
STM32_Programmer_CLI -c port=SWD -w build/Debug/stm32f103zet6.elf -v -rst
```

---

## 项目地图

| 主题 | 文档 |
|------|------|
| 已验证功能与验收标准 | `docs/product-specs/firmware-capabilities.md` |
| 架构与数据流 / 模块分层说明 | `ARCHITECTURE.md` |
| 新增模块应放哪里（BSP / App / Config） | `ARCHITECTURE.md` 第 2、6 节 |
| 硬件引脚分配 | `HARDWARE_PINOUT.md` |
| 设计原则与决策 | `docs/design-docs/` |
| 执行计划（活跃 / 已完成 / 技术债务） | `docs/exec-plans/` |
| 产品规格 | `docs/product-specs/` |
| 参考资料 | `docs/references/` |
| 设计系统 / UI | `docs/DESIGN.md` |
| 测试策略 | `docs/TESTING.md` |
| 安全与敏感信息 | `docs/SECURITY.md` |
| 提交规范 | `docs/COMMITS.md` |
| 质量评分 | `docs/QUALITY_SCORE.md` |
| 可靠性要求 | `docs/RELIABILITY.md` |

---

## 智能体工作原则

1. **代码仓库是记录系统。** 业务知识、设计决策、验收标准、执行计划应存放在代码仓库内已版本化的 Markdown 文件中，而不是聊天记录或人脑中。
2. **渐进式披露。** 先读 `AGENTS.md` 定位，再按项目地图深入 `ARCHITECTURE.md` 或 `docs/` 中对应的真实信息源。
3. **保持 `AGENTS.md` 精简。** 若新增内容更适合放入 `docs/` 的某个专题文档，则不要把它塞进本文件；只在这里留下入口。
4. **修改流程或规范时同步更新文档。** 若更改了构建命令、模块分层、引脚分配或编码约定，应同时更新 `docs/` 或 `ARCHITECTURE.md` 中对应文件，避免文档腐烂。

---

## 维护说明

- 本文件遵循 [agents.md](https://agents.md/) 与 [OpenAI Harness Engineering](https://openai.com/zh-Hans-CN/index/harness-engineering/) 原则。
- 子目录如需更具体的 Agent 指引，可创建嵌套 `AGENTS.md`；最接近目标文件的版本优先级最高。

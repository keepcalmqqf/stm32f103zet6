<!-- AGENTS.md — stm32f103zet6 -->

> 面向 AI 编码助手的项目地图。详细指南见 `docs/` 目录。
>
> 冲突解决优先级：**用户显式提示 > 本文件 > 子目录 AGENTS.md**。

---

## 项目是什么

- **名称**：`stm32f103zet6`
- **类型**：STM32CubeMX 生成的 CMake 固件工程
- **目标 MCU**：STM32F103ZET6（ARM Cortex-M3，LQFP144，72 MHz）
- **技术栈**：STM32 HAL + CMake + Ninja + arm-none-eabi-gcc
- **配置源文件**：`stm32f103zet6.ioc`

---

## 做任何事前先执行

1. 修改 `stm32f103zet6.ioc` 后 → 用 STM32CubeMX 重新生成代码：

   ```bash
   STM32CubeMX -s cubemx_generate.script
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

## 禁止做的事（会覆盖代码 / 引入 Bug）

- 不要手动修改 `cmake/stm32cubemx/CMakeLists.txt`、启动文件或链接脚本。
- 不要在 `Core/Src/main.c` 等生成文件的非 `/* USER CODE BEGIN/END ... */` 区域手写代码。
- 不要把新源码加到 `cmake/stm32cubemx/CMakeLists.txt`；应通过根目录 `CMakeLists.txt` 添加。
- 不要硬编码 Magic Number、延时循环或 GPIO 状态而不加注释。
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
| 架构与数据流 | `ARCHITECTURE.md` |
| 构建、烧录与发布流程 | `RELEASE.md` |
| 硬件引脚分配 | `HARDWARE_PINOUT.md` |
| 设计原则与决策 | `docs/design-docs/` |
| 执行计划 | `docs/exec-plans/` |
| 产品规格 | `docs/product-specs/` |
| 参考资料 | `docs/references/` |
| 设计系统 / UI | `docs/DESIGN.md` |
| 测试策略 | `docs/TESTING.md` |
| 安全与敏感信息 | `docs/SECURITY.md` |
| 提交规范 | `docs/COMMITS.md` |
| 质量评分 | `docs/QUALITY_SCORE.md` |
| 可靠性要求 | `docs/RELIABILITY.md` |

---

## 维护说明

- 本文件遵循 [agents.md](https://agents.md/) 与 [OpenAI Harness Engineering](https://openai.com/zh-Hans-CN/index/harness-engineering/) 原则。
- 若修改了本文件指向的流程、命令或规范，请同步更新 `docs/` 中对应文件。
- 子目录如需更具体的 Agent 指引，可创建嵌套 `AGENTS.md`；最接近目标文件的版本优先级最高。

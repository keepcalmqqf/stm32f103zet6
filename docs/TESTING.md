# 测试规范

本文档描述本项目的测试策略与验证流程。

---

## 测试层级

### 1. 构建验证（自动化程度：手动）

每次提交前必须执行干净构建：

```bash
rm -rf build/Debug
cmake --preset Debug
cmake --build --preset Debug
```

### 2. 体积检查（自动化程度：手动）

```bash
arm-none-eabi-size build/Debug/stm32f103zet6.elf
```

确认：
- text + data 不超过 512 KB Flash
- data + bss 不超过 64 KB SRAM

### 3. 板上功能验证（自动化程度：手动）

烧录后观察：

| 检查项 | 通过标准 |
|--------|----------|
| LED 指示 | 上电后 LED1 先亮，随后 LED1/2/3 每 5 秒循环 |
| 串口日志 | USB-TTL 收到 `STM32F103ZET6 USART1 initialized: 115200 baud` |
| LCD 初始化 | 屏幕依次显示蓝、红、绿、黑 |
| LCD ID | 串口打印 `ID: 0xXXXXXX` 为有效值 |

---

## 回归测试要求

- 修复 Bug 时，必须增加或更新验证步骤，确保问题不再复现。
- 新增外设时，必须在 [`product-specs/firmware-capabilities.md`](./product-specs/firmware-capabilities.md) 中登记并通过板上验证。

---

## 未来改进

- 引入单元测试框架（如 Unity + CMock）。
- 在 CI 中执行 `cmake --build --preset Release` 与 `arm-none-eabi-size` 检查。

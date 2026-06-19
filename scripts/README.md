# 项目脚本

本目录存放用于辅助开发、代码生成和配置一次性注入的工具脚本。

| 脚本 | 用途 | 说明 |
|------|------|------|
| `configure_fsmc_ioc.py` | 向 `stm32f103zet6.ioc` 注入 FSMC 16 位 8080 并行 LCD 配置 | 仅在 CubeMX 重新生成后需要补全 FSMC 配置时运行 |
| `cubemx_generate.script` | STM32CubeMX 命令行生成脚本 | 由 `STM32CubeMX -s scripts/cubemx_generate.script` 调用 |

## 使用示例

```bash
# 命令行重新生成 CubeMX 代码
STM32CubeMX -s scripts/cubemx_generate.script

# 给 .ioc 注入 FSMC 配置（运行前会自动备份 .ioc）
python scripts/configure_fsmc_ioc.py
```

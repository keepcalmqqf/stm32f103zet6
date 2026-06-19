# 设计系统

本文档描述项目的硬件接口设计、固件模块划分与 UI/显示相关约定。

---

## 硬件接口设计

- MCU：STM32F103ZET6（Cortex-M3，512 KB Flash，64 KB SRAM，144 引脚）
- 显示：3.5 寸 TFT，分辨率 320×480，控制器 ILI9486
- 显示总线：16 位 8080 并行，通过 FSMC Bank4（NE4）+ A10 选择 RS
- 调试串口：USART1，115200 8N1，PA9/PA10
- LED：PA0/PA1/PA8，低电平点亮

完整引脚分配见 [`HARDWARE_PINOUT.md`](../HARDWARE_PINOUT.md)。

---

## 固件模块划分

```text
App (main.c)
├── BSP
│   ├── led.c      # LED 状态指示
│   ├── usart.c    # 调试日志
│   └── ili9486.c  # LCD 驱动
└── CubeMX 生成层
    ├── gpio.c     # GPIO 初始化
    ├── fsmc.c     # FSMC 初始化
    └── usart.c    # USART1 初始化（MX_*）
```

---

## 显示约定

- 颜色格式：RGB565
- 坐标系：原点在左上角，X 向右，Y 向下
- 默认方向：竖屏 320×480
- 刷屏前必须设置地址窗口（`ILI9486_SetAddressWindow`）

---

## 未来扩展

- 触摸屏：XPT2046 通过 SPI2 + PC5 中断
- 按键：PF6/PF7/PF8/PF11
- SD 卡：SDIO 接口

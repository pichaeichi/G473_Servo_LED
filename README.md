# STM32G473RCT6 LED+servo

## 1. 项目简介

本工程基于 **STM32G473RCT6**，使用一个板载按键控制 LED 闪烁和舵机往复摆动，实现以下功能：

- 上电后系统自动运行，LED 以 **1 Hz** 闪烁，舵机先回到 **90°中位**。
- 舵机与 LED 使用同一节拍同步动作：LED 每次翻转时，舵机在中位两侧来回摆动。
- 短按按键：循环切换 LED 闪烁和舵机摆动频率。
- 双击按键：循环切换舵机摆动幅度。
- 长按按键：关闭或重新打开 LED 闪烁和舵机摆动。
- LED 节拍由定时器中断产生。

工程由 STM32CubeMX 生成底层初始化代码，使用 CMake 组织构建，可在 CLion 中编译，并通过 DAPLink、OpenOCD 和 SWD 接口烧录、调试。

## 2. 硬件与引脚分配

### 2.1 主控与时钟

| 项目 | 配置 |
| --- | --- |
| MCU | STM32G473RCT6 |
| 内核 | Arm Cortex-M4F |
| 封装 | LQFP64 |
| 外部晶振 | 25 MHz HSE |
| 系统主频 | 170 MHz |
| 调试接口 | SWD |

### 2.2 功能引脚

| 功能 | MCU 引脚 | CubeMX 标签/复用功能 | 有效电平 |
| --- | --- | --- | --- |
| 舵机 PWM 信号 | PC0 | TIM1_CH1 | 高电平脉宽控制 |
| 用户按键 | PB10 | KEY，GPIO Input | 低电平按下 |
| 运行 LED | PB12 | LED_RUN，GPIO Output | 高电平点亮 |
| SWD 数据 | PA13 | SWDIO | — |
| SWD 时钟 | PA14 | SWCLK | — |

按键 PB10 配置为内部上拉输入，因此松开时为高电平，按下时为低电平。

### 2.3 DAPLink 连接

| DAPLink | 主控板 |
| --- | --- |
| SWDIO | PA13 / SWDIO |
| SWCLK | PA14 / SWCLK |

## 3. 技术栈

| 分类 | 技术/工具 | 用途 |
| --- | --- | --- |
| MCU | STM32G473RCT6 | 程序运行平台 |
| 编程语言 | C11、少量启动汇编 | 应用逻辑和芯片启动 |
| 固件库 | STM32 HAL、CMSIS | GPIO、定时器、中断和芯片寄存器支持 |
| 图形化配置 | STM32CubeMX 6.18.1 | 时钟、GPIO、TIM、NVIC 和工程代码生成 |
| IDE | CLion 2026.2.1 | 编辑、CMake 配置、编译和调试 |
| 工具链 | STM32CubeCLT 1.22.0、GNU Arm Embedded Toolchain | 交叉编译和链接 |
| 构建系统 | CMake 3.22+、Ninja | 管理源文件并生成 `.elf` |
| 下载调试 | DAPLink / CMSIS-DAP、OpenOCD、GDB | 通过 SWD 烧录与在线调试 |
| 配置管理 | `.ioc`、CMake Presets | 保存 CubeMX 外设配置和 Debug/Release 构建配置 |


## 4. 工程目录与代码分区

```text
G473RCT6_Servo_LED/
├─ Core/
│  ├─ Inc/
│  │  ├─ app.h                 # 应用层接口和运行状态结构体
│  │  ├─ button.h              # 按键事件、消抖状态和接口
│  │  ├─ servo.h               # 舵机角度、脉宽和接口
│  │  ├─ main.h                # 引脚定义和公共声明
│  │  ├─ gpio.h                # CubeMX 生成的 GPIO 接口
│  │  ├─ tim.h                 # CubeMX 生成的定时器接口
│  │  └─ stm32g4xx_*.h         # HAL 配置与中断声明
│  └─ Src/
│     ├─ main.c                # 芯片初始化、App_Init、主循环
│     ├─ app.c                 # 功能协调、模式切换、定时器回调
│     ├─ button.c              # 20 ms消抖及单击/双击/长按识别
│     ├─ servo.c               # 角度限幅及角度到PWM脉宽转换
│     ├─ gpio.c                # PB10、PB12初始化
│     ├─ tim.c                 # TIM1、TIM6、TIM7初始化
│     ├─ stm32g4xx_it.c        # 中断服务入口
│     ├─ stm32g4xx_hal_msp.c   # HAL底层外设支持初始化
│     └─ system_*.c/sys*.c     # 系统、内存和系统调用支持
├─ Drivers/
│  ├─ CMSIS/                   # Arm内核与STM32设备定义
│  └─ STM32G4xx_HAL_Driver/    # STM32G4 HAL驱动源码
├─ cmake/
│  ├─ gcc-arm-none-eabi.cmake  # GNU Arm交叉编译工具链配置
│  └─ stm32cubemx/CMakeLists.txt
│                              # CubeMX生成源码与HAL驱动清单
├─ openocd/
│  └─ daplink_stm32g4.cfg      # DAPLink + STM32G4下载配置
├─ G473RCT6_Servo_LED.ioc      # STM32CubeMX工程配置
├─ CMakeLists.txt              # 顶层构建配置及自定义源码清单
├─ CMakePresets.json           # Debug、Release构建预设
├─ STM32G473xx_FLASH.ld        # Flash/RAM链接脚本
├─ startup_stm32g473xx.s       # 启动文件和中断向量表
└─ README.md                   
```

### 4.1 自动生成区

下列内容主要由 STM32CubeMX 管理：

- `main.c` 中带有 `USER CODE BEGIN/END` 标记之外的部分。
- `gpio.c`、`tim.c`、`stm32g4xx_it.c` 等外设初始化代码。
- `Drivers/`、启动文件、链接脚本以及 `cmake/stm32cubemx/`。

重新生成代码时，只应把手写内容放在 `USER CODE BEGIN/END` 区域内，以免被 CubeMX 覆盖。

### 4.2 自定义业务区

以下文件是本项目的主要业务代码：

- `app.c/app.h`：统一管理系统状态、按键事件、LED、舵机和定时器。
- `button.c/button.h`：完成软件消抖和短按、双击、长按识别。
- `servo.c/servo.h`：把目标角度转换为 TIM1 PWM 比较值。

顶层 `CMakeLists.txt` 已将这三个自定义 `.c` 文件加入编译目标。增加新的业务源文件后，也要在 `target_sources()` 中加入对应路径。


## 5. 定时器分工

系统定时器时钟为 170 MHz(拉满)。

### 5.1 TIM1：舵机 PWM

TIM1 参数：

```text
Prescaler = 169
Period    = 19999
Channel   = TIM1_CH1 / PC0
```

计算过程：

```text
计数频率 = 170 MHz / (169 + 1) = 1 MHz
单个计数 = 1 μs
PWM周期  = (19999 + 1) × 1 μs = 20 ms
PWM频率  = 50 Hz
```

`servo.c` 将 0°～180°线性映射到 1000～2000 μs：

| 舵机角度 | PWM高电平脉宽 | TIM1比较值 |
| --- | --- | --- |
| 0° | 1000 μs | 1000 |
| 90° | 1500 μs | 1500 |
| 180° | 2000 μs | 2000 |

不同型号舵机的机械范围可能不同。如果舵机出现撞限位或抖动，应减小脉宽范围或摆动幅度。

### 5.2 TIM6：1 ms按键扫描

TIM6 参数：

```text
Prescaler = 16999
Period    = 9
NVIC抢占优先级 = 2
```

计算过程：

```text
170 MHz / (16999 + 1) = 10 kHz
(9 + 1) / 10 kHz = 1 ms
```

TIM6 每 1 ms产生一次更新中断，并调用 `Button_Update1ms()`。所有按键计时都基于这个稳定的 1 ms节拍，不依赖主循环执行速度。

### 5.3 TIM7：LED与舵机同步节拍

TIM7 使用与 TIM6 相同的 10 kHz计数频率，即每毫秒计数10次。应用层根据所选频率动态修改 TIM7 的自动重装值，TIM7 每半个完整周期产生一次中断：

| 工作频率 | 完整周期 | 中断间隔/半周期 | TIM7 ARR |
| --- | --- | --- | --- |
| 0.5 Hz | 2000 ms | 1000 ms | 9999 |
| 1 Hz（初始） | 1000 ms | 500 ms | 4999 |
| 2 Hz | 500 ms | 250 ms | 2499 |
| 4 Hz | 250 ms | 125 ms | 1249 |

每次 TIM7 中断会同时：

1. 翻转 LED 状态。
2. 翻转舵机方向。
3. 将舵机角度设置为 `90° + 幅度` 或 `90° - 幅度`。

因此 LED 和舵机由同一个中断事件驱动，能够保持同步。


## 6. 按键识别逻辑

按键参数定义在 `Core/Src/button.c`：

```c
#define BUTTON_DEBOUNCE_TIME_MS    20U
#define BUTTON_DOUBLE_GAP_MS      350U
#define BUTTON_LONG_PRESS_MS     1000U
```

| 操作 | 判定条件 | 产生事件 | 功能 |
| --- | --- | --- | --- |
| 短按 | 松开后350 ms内没有第二次按下 | `BUTTON_EVENT_SHORT` | 切换频率 |
| 双击 | 第一次松开后350 ms内完成第二次按下并松开 | `BUTTON_EVENT_DOUBLE` | 切换幅度 |
| 长按 | 稳定按下达到1000 ms | `BUTTON_EVENT_LONG` | 开启/关闭系统 |

### 6.1 软件消抖

原始电平必须连续保持变化状态20 ms，程序才更新稳定按键状态。机械触点在按下和松开瞬间产生的快速抖动因此不会被识别为多次操作。

### 6.2 短按为何不会立即响应

一次松开后，程序需要等待350 ms，确认用户没有继续第二次点击，才能把操作判定为短按。因此短按频率切换会存在最多约350 ms的正常判定延迟。

### 6.3 中断与主循环之间的数据传递

TIM6中断只更新按键状态并设置事件位；主循环调用 `Button_GetEvents()` 取走事件，再执行功能切换。读取和清空事件时会短暂关闭中断，避免主循环和中断同时修改事件变量。

---

## 7. 功能状态与参数

### 7.1 初始状态

| 状态项 | 初始值 |
| --- | --- |
| 系统运行 | 开启 |
| LED | 熄灭，随后按1 Hz节拍翻转 |
| 舵机 | 90°中位 |
| 频率 | 1 Hz |
| 摆动幅度 | ±20° |

### 7.2 短按频率循环

```text
1 Hz → 2 Hz → 4 Hz → 0.5 Hz → 1 Hz → …
```

代码内部完整频率表为：

```c
{0.5 Hz, 1 Hz, 2 Hz, 4 Hz}
```

由于初始索引为1，所以第一次短按后会从1 Hz切换到2 Hz。

### 7.3 双击幅度循环

```text
±20° → ±40° → ±60° → ±20° → …
```

对应的舵机端点为：

| 幅度 | 两个目标角度 |
| --- | --- |
| ±20° | 70°、110° |
| ±40° | 50°、130° |
| ±60° | 30°、150° |

双击只修改幅度参数，舵机会在下一次 TIM7 节拍到达时使用新幅度。

### 7.4 长按开关

长按关闭时：

- 停止 TIM7中断。
- 熄灭 LED。
- 舵机回到90°中位。

再次长按开启时：

- LED先保持熄灭。
- 舵机先回到90°中位。
- 重新启动 TIM7，并按当前保存的频率和幅度继续工作。

频率和幅度设置在关闭期间不会被清除。


## 8. 程序执行流程

### 8.1 上电初始化

`main.c` 依次执行：

```text
HAL_Init()
→ SystemClock_Config()
→ MX_GPIO_Init()
→ MX_TIM1_Init()
→ MX_TIM6_Init()
→ MX_TIM7_Init()
→ App_Init()
→ while (1) 调用 App_Process()
```

`App_Init()` 完成：

1. LED熄灭。
2. 初始化按键对象，设置低电平按下。
3. 启动 TIM1_CH1 PWM并让舵机回中。
4. 启动 TIM6的1 ms按键扫描中断。
5. 按初始1 Hz配置并启动 TIM7节拍中断。

### 8.2 前后台结构

- **中断后台：** TIM6执行按键采样和计时；TIM7翻转 LED并改变舵机目标角度。
- **主循环前台：** `App_Process()`读取按键事件，修改频率、幅度或启停状态。

主循环中没有阻塞等待，也没有调用 `HAL_Delay()`。


## 9. 关键源文件说明

### `Core/Src/main.c`

只负责HAL、时钟和外设初始化，并进入持续调用 `App_Process()` 的主循环。业务逻辑通过 `App_Init()` 和 `App_Process()` 接入CubeMX生成框架。

### `Core/Src/app.c`

工程的应用控制中心：

- 保存当前频率、幅度、运行状态和舵机方向。
- 根据短按、双击、长按事件切换模式。
- 运行时计算 TIM7的ARR值。
- 在 `HAL_TIM_PeriodElapsedCallback()` 中区分 TIM6和TIM7。
- 保持 LED与舵机动作同步。

### `Core/Src/button.c`

维护一个 `Button` 状态结构，按1 ms周期实现：

- 输入电平采样。
- 20 ms软件消抖。
- 按下时间累计。
- 单击等待窗口。
- 短按、双击和长按事件生成。

### `Core/Src/servo.c`

封装舵机控制：

- 启动TIM1 PWM。
- 将角度限制在0°～180°。
- 将角度线性转换成1000～2000 μs比较值。
- 提供回中和相对中位偏移接口。


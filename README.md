# 智能仓库安防系统 fire_theif_proof

**[English](./README_EN.md)** | **中文**

基于 **STM32F407VET6 + FreeRTOS + ESP8266（MQTT / 华为云 IoTDA）** 的智能仓库安防系统（防火防盗），集温湿度、烟雾、火焰、人体入侵检测于一体，实现分级报警联动控制、OLED 本地交互、阈值掉电保存与云端属性上报，并配套 PySide6 + OpenCV 人脸识别上位机用于陌生人检测。

## 功能特性

- **多传感器采集**：DHT11 温湿度、MQ-2 烟雾/可燃气体浓度、红外火焰传感器、PIR 人体红外
- **分级报警联动**（优先级：火焰 > 燃气 > 陌生人 > 温度 > 湿度 > 正常）
  - 火警：洒水泵开启 + 风扇全速排烟 + 蜂鸣器报警 + RGB 状态灯
  - 温度 / 燃气超标：风扇 PWM 排风
  - 陌生人（PIR 触发 + 上位机人脸识别确认）：门禁与声光报警联动
- **三种运行模式**：自动模式 / 手动模式 / 阈值设置模式，按键切换
- **本地交互**：OLED 双页界面（传感器数据页 / 执行器状态页）+ 阈值设置界面；阈值经 EEPROM（AT24Cxx）掉电保存
- **云端交互**：MQTT 属性上报至华为云 IoTDA；RTC 支持 SNTP 网络对时
- **人脸识别上位机**：SmartFaceApp（PySide6 + OpenCV，LBPH 模型），识别到陌生人后经串口向 MCU 发送 `STRANGER` 指令

## 系统架构

FreeRTOS 多任务架构（4 个任务，共享数据由互斥锁 `xDataMutex` 保护）：

| 任务 | 函数 | 栈（字） | 优先级 | 周期 | 职责 |
|---|---|---|---|---|---|
| Control | `Task_ControlLogic` | 256 | 4 | 50 ms | 分级报警判断与执行器联动控制 |
| Sensor | `Task_SensorRead` | 256 | 3 | 500 ms | 读取 DHT11 / MQ-2 / 火焰红外 / PIR |
| Network | `Task_Network` | 512 | 2 | 2000 ms | MQTT 连接保活与属性上报 |
| Display | `Task_Display` | 512 | 1 | 200 ms | OLED 界面刷新与状态显示 |

```
 传感器                     控制                      交互
┌────────┐   500ms   ┌──────────────┐   50ms   ┌──────────────┐
│ DHT11  │──────────▶│              │────────▶│ 蜂鸣器 / 风扇 │
│ MQ-2   │           │ Task_Control │         │ 舵机 / 水泵   │
│ 火焰IR │           │  (xDataMutex)│         │ RGB LED      │
│ PIR    │           └──────┬───────┘         └──────────────┘
└────────┘                  │                          ▲
       ▲                    ▼                          │
┌────────────┐      ┌──────────────┐           ┌──────────────┐
│ SmartFace  │─────▶│ Task_Network │──────────▶│ Task_Display │
│ 上位机(串口)│      │ ESP8266 MQTT │           │ OLED / KEY   │
└────────────┘      └──────────────┘           └──────────────┘
                       华为云 IoTDA                EEPROM / RTC
```

## 目录结构

| 目录 | 作用 |
|---|---|
| `app/` | 应用入口 `main.c`，4 个 FreeRTOS 任务的完整业务逻辑 |
| `board/` | 板级外设驱动（按外设分子目录：传感器、执行器、显示、按键等） |
| `module/` | 中断服务 `stm32f4xx_it.c`、外设配置头 `stm32f4xx_conf.h` |
| `Net/` | ESP8266 驱动 + MQTT 协议封装（WiFi / 华为云连接配置） |
| `FreeRTOS/` | FreeRTOS V10.5.1 内核源码与 ARM_CM4F 移植 |
| `libraries/` | STM32F4 标准外设库 V1.8.0 + CMSIS |
| `SmartFaceApp/` | 人脸识别上位机（PySide6 + OpenCV 打包的 exe） |
| `project/` | Keil MDK 工程文件 `fire_theif_proof.uvprojx` 及编译输出 |

## 硬件资源映射

| 外设 | 硬件资源 | 引脚 | 说明 |
|---|---|---|---|
| DHT11 温湿度 | GPIO 单总线 | PB13 | 500 ms 周期读取 |
| MQ-2 烟雾 | ADC1_IN3 | PA3 | 换算为百分比，ADC1 互斥锁 |
| 火焰红外 | ADC1_IN2 | PA2 | 读取阻值判断火源 |
| 蜂鸣器 | GPIO | PA5 | 按频率播放 |
| RGB LED | TIM1 CH1/2/3 PWM | PE9 / PE11 / PE13 | 按报警类型变色 |
| OLED | 软件 I2C | PB8(SCL) / PB9(SDA) | GRAM 刷新显示 |
| EEPROM AT24Cxx | 软件 I2C | PB8 / PB9 | 与 OLED 共用总线，存阈值 |
| 舵机（门禁） | TIM3_CH4 PWM | PC9 | 50 Hz，0°/180° 开关门 |
| 水泵 | TIM3 PWM | PC8 | 火警洒水 |
| 风扇 | TIM3 PWM | PC6 / PC7 | 正反转/刹车，火警全速(50)、超标排风(40) |
| 调试串口 USART1 | USART + DMA2_Stream7 | PA9(TX) / PA10(RX) | 115200；兼接收上位机 `STRANGER` 指令 |
| ESP8266 | USART3 | PB10(TX) / PB11(RX) | 115200，AT 指令收发 |
| 按键 KEY1–KEY4 | EXTI + TIM7 | PG2 / PG3 / PG4 / PG5 | 下降沿中断 + 1 ms 扫描（长/短按） |
| TIM6 | 定时中断 | — | 100 ms 周期，RGB / 蜂鸣器闪烁节拍 |
| TIM7 | 定时中断 | — | 1 ms 周期，按键扫描与软件计时 |
| RTC | Alarm / WKUP 中断 | — | 实时时钟，支持 SNTP 网络对时 |

## 核心控制逻辑

`Task_ControlLogic` 以 50 ms 周期执行分级报警判断，优先级为：

```
火焰 > 燃气 > 陌生人 > 温度 > 湿度 > 正常
```

- **火警**：水泵开启 + 风扇全速（PWM=50）+ 蜂鸣器 + RGB 红色闪烁
- **温度 / 燃气超标**：仅风扇排风（PWM=40）
- **陌生人**：门禁联动 + 声光报警（PIR 触发，人脸识别上位机经串口确认）
- 三种运行状态：**自动模式**（按阈值自动联动）/ **手动模式**（按键直接控制）/ **阈值设置模式**（按键加减并保存至 EEPROM）

## 云端交互（华为云 IoTDA）

- WiFi 与 MQTT 连接参数配置于 `Net/esp8266.h` 与 `Net/mqtt.h`（含 WiFi SSID/密码、设备三元组）
- MQTT Broker：华为云 IoTDA（TCP 1883），SNTP 对时同步 RTC
- 上报 Topic：`$oc/devices/{device_id}/sys/properties/report`
- 订阅 Topic：`$oc/devices/{device_id}/sys/messages/down`
- 上报属性（service_id = `jifengdaxunlin`）：

| 属性 | 含义 | 属性 | 含义 |
|---|---|---|---|
| `temp` | 温度 | `fan` | 风扇状态 |
| `humi` | 湿度 | `gate` | 门禁状态 |
| `fire` | 火焰报警 | `water` | 洒水状态 |
| `gas` | 燃气浓度 | `led` | LED 状态 |
| `stranger` | 陌生人报警 | | |

## 人脸识别上位机 SmartFaceApp

- 独立打包的 Windows 可执行程序（PyInstaller，内含 Python 3.10、OpenCV、numpy、PySide6）
- 采用 LBPH 人脸识别模型（`model/trainer.yml`），OpenCV Haar 级联检测人脸
- 内置已录入人脸 `fyf`（`face/fyf/1~20.jpg` 训练样本）作为已知人员
- 运行后通过串口（USART1）与下位机通信，识别到陌生人发送 `STRANGER` 指令触发报警

## 构建与烧录

1. 使用 Keil MDK 打开 `project/fire_theif_proof.uvprojx`
2. 目标芯片：STM32F407VETx（Target 名 `F407VE`）
3. 编译生成 `fire_theif_proof.axf` / `fire_theif_proof.hex`
4. 通过 J-Link 下载烧录
5. 运行 `SmartFaceApp/SmartFaceApp.exe` 启动人脸识别上位机（需串口连接至上位机）

## 使用说明

- **KEY1–KEY4**：切换自动/手动模式、OLED 翻页、阈值加减与保存（支持长/短按）
- **OLED 双页界面**：页 1 显示传感器实时数据，页 2 显示执行器状态；阈值设置模式下可修改温湿度/气体/火焰阈值
- 阈值保存后写入 EEPROM，掉电不丢失

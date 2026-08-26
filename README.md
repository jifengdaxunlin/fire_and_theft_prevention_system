# 智能仓库安防系统 fire_theif_proof
**[English](./README_EN.md)** | **中文**

> 📌 STM32F407VET6 + FreeRTOS + ESP8266(MQTT/华为云IoTDA) 实现仓库防火防盗综合安防系统
> 项目包含底层驱动、RTOS多任务业务逻辑、云端通信、人脸识别上位机完整代码；可作为嵌入式软件开发简历项目。

## 📋 项目简介
本项目针对仓库安防场景开发，实现多源传感器数据采集、分级消防防盗联动、本地按键+OLED人机交互、EEPROM阈值掉电存储、MQTT物联网云端上报；配套Python上位机实现人脸识别，通过串口与STM32联动触发防盗告警。
项目重点解决RTOS多任务资源竞争、时序敏感外设驱动、传感器噪声滤波、弱网下云端通信可靠性等嵌入式工程实际问题。

## ✨ 功能特性
- **多传感器采集**：DHT11温湿度、MQ‑2烟雾/可燃气体浓度、红外火焰传感器、PIR人体红外检测
- **分级报警联动**（告警优先级：`火焰 > 燃气 > 陌生人 > 温度 > 湿度 > 正常`）
  - 火警：洒水泵开启 + 风扇全速排烟 + 蜂鸣器高频报警 + RGB红色闪烁告警
  - 温度/燃气超标：风扇PWM自动排风，预警提示
  - 陌生人入侵：PIR检测 + PC人脸识别双重确认，触发舵机关闭隔离闸门、声光报警
- **三种运行模式**：自动模式 / 手动模式 / 阈值设置模式，按键短按、长按区分操作
- **本地人机交互**：OLED双页UI（传感器数据页 / 执行器状态页）+阈值设置页面；告警阈值存储至AT24C02‑EEPROM，断电数据不丢失
- **物联网云端**：ESP8266 WiFi模块，手写MQTT3.1.1协议对接华为云IoTDA；支持SNTP网络对时同步RTC时钟
- **PC上位机**：PySide6 + OpenCV‑Haar级联人脸识别；识别陌生人通过串口下发`STRANGER`指令给下位机触发告警

## ⚙️ FreeRTOS多任务系统架构
4个业务任务分离采集、控制、网络、显示；使用互斥锁解决总线、ADC硬件资源竞争，`vTaskDelayUntil`保证任务严格周期调度，避免调度漂移。

| 任务 | 函数名 | 栈大小(words) | 优先级 | 运行周期 | 职责说明 |
|---|---|---|---|---|---|
| Control | `Task_ControlLogic` | 256 | 4 | 50 ms | 分级告警逻辑判断、执行器联动控制 |
| Sensor | `Task_SensorRead` | 256 | 3 | 500 ms | 各传感器周期数据读取与预处理滤波 |
| Network | `Task_Network` | 512 | 2 | 2000 ms | MQTT保活、设备属性上报、SNTP时间同步 |
| Display | `Task_Display` | 512 | 1 | 200 ms | OLED屏幕刷新、系统状态展示 |

> 💡 `.gitignore` 已过滤Keil编译产物、exe打包二进制，仓库仅保留源码。

| 目录 | 说明 |
|---|---|
| `app/` | 应用入口`main.c`，4个FreeRTOS任务业务逻辑 |
| `board/` | 板级外设底层驱动，按传感器、执行器、通信分模块 |
| `module/` | 中断服务文件、芯片外设配置头文件 |
| `Net/` | ESP8266 AT驱动、手写MQTT协议栈，华为云连接配置 |
| `FreeRTOS/` | FreeRTOS V10.5.1内核源码 + Cortex‑M4F移植层 |
| `libraries/` | STM32F4标准外设库V1.8.0、CMSIS库文件 |
| `SmartFaceApp/` | 人脸识别上位机Python源码（**不包含打包exe产物**） |
| `project/` | Keil‑MDK工程文件 |

## 🔌 硬件引脚资源映射
| 外设模块 | 硬件资源 | 引脚 | 备注 |
|---|---|---|---|
| DHT11温湿度 | GPIO单总线 | PB13 | 500ms周期读取 |
| MQ‑2烟雾传感器 | ADC1_IN3 | PA3 | ADC互斥锁，防止通道抢占 |
| 火焰红外传感器 | ADC1_IN2 | PA2 | 通过分压阻值判断火源 |
| 蜂鸣器 | GPIO | PA5 | 不同告警输出不同频率音调 |
| RGB状态灯 | TIM1 CH1/2/3 PWM | PE9 / PE11 / PE13 | 不同告警等级对应不同颜色 |
| OLED显示屏 | 软件I2C | PB8(SCL) / PB9(SDA) | 显存页模式刷新 |
| EEPROM AT24C02 | 软件I2C | PB8(SCL) / PB9(SDA) | 与OLED共享I2C总线，互斥锁保护 |
| 隔离闸门舵机 | TIM3_CH4 PWM | PC9 | 50Hz PWM，0°关闭闸门，180°打开 |
| 消防水泵 | TIM3 PWM | PC8 | 仅火警触发开启 |
| 排烟风扇 | TIM3 PWM | PC6 / PC7 | 支持正转、反转、刹车调速 |
| USART1调试串口 | USART1+DMA | PA9(TX)/PA10(RX) | 115200波特，接收上位机STRANGER指令 |
| ESP8266 WiFi | USART3 | PB10(TX)/PB11(RX) | AT指令交互，对接华为云IoT |
| 4个功能按键 | EXTI外部中断 + TIM7扫描 | PG2 / PG3 / PG4 / PG5 | 区分短按、长按1.5s |
| TIM6 | 定时器中断 | - | 100ms节拍，控制蜂鸣器、LED闪烁 |
| TIM7 | 定时器中断 | - | 1ms节拍，按键软件消抖扫描 |
| RTC实时时钟 | RTC外设 + WKUP | - | 支持SNTP网络同步北京时间 |

## 🧠 核心技术与工程难点
1. **RTOS多任务并发安全**
    - 使用三把互斥锁分别保护：共享业务数据、I2C总线、ADC外设；拿锁设置超时时间，获取锁失败直接放弃本次传输，避免任务死锁。
    - 严格控制临界区代码长度；DHT11微秒时序只将4ms应答读取放进临界区，起始信号放在临界区外，防止长时间关中断。
2. **时序敏感外设驱动**
    - 使用DWT内核计数器实现微级延时，规避FreeRTOS环境下SysTick延时导致调度卡死的经典bug。
3. **传感器多级数字滤波**
    - MQ‑2烟雾：中值平均滤波 + 突变限幅 + 一阶低通滤波，抑制电源、环境带来的尖峰噪声。
4. **手写MQTT协议栈**
    - 不依赖第三方MQTT库，完整实现CONNECT / PUBLISH / SUBSCRIBE报文，处理剩余长度变长编码；校验CONNACK、SUBACK应答，增加缓冲区溢出检测。
5. **EEPROM可靠性存储**
    - MAGIC魔字标记首次上电；8字节分页写；写完成后ACK轮询等待芯片内部烧写完成，防止假写入。
6. **上位机‑下位机串口通信**
    - USART中断解析上位机人脸报警指令，增加超时复位，防止脏数据持续误触发告警。

## ☁️ 华为云IoTDA云端交互
- 修改配置文件`Net/esp8266.h`、`Net/mqtt.h`填入WiFi信息与华为云设备三元组。
- MQTT端口：TCP 1883；RTC通过SNTP同步网络时间。
- 上报Topic：`$oc/devices/{device_id}/sys/properties/report`
- 下发订阅Topic：`$oc/devices/{device_id}/sys/messages/down`

上报设备属性列表：
| 属性 | 含义 | 属性 | 含义 |
|---|---|---|---|
| `temp` | 温度 | `fan` | 风扇工作状态 |
| `humi` | 湿度 | `gate` | 隔离闸门状态 |
| `fire` | 火焰告警标志 | `water` | 消防水泵状态 |
| `gas` | 燃气烟雾浓度 | `led` | RGB告警灯状态 |
| `stranger` | 陌生人入侵告警 | | |

## 🖥️ 人脸识别上位机 SmartFaceApp
> 源码在`SmartFaceApp/`目录，exe打包产物不纳入git版本管理。
- 技术栈：Python + PySide6图形界面 + OpenCV Haar级联人脸检测 + LBPH人脸识别模型。
- 逻辑：摄像头识别人脸；区分已知人员与陌生人；识别陌生人后串口发送`STRANGER`字符串，通知STM32触发防盗报警。

## 🔨 编译、烧录运行
1. 使用Keil‑MDK5打开工程文件：`project/fire_theif_proof.uvprojx`
2. 选择目标芯片：`STM32F407VETx`，编译工程。
3. 使用J‑Link下载hex/axf固件至开发板。
4. 修改ESP8266的WiFi账号密码、华为云IoT设备三元组配置。
5. PC串口连接开发板USART1，运行上位机Python源码。

## 🎮 使用操作说明
- **KEY1**：短按切换自动/手动模式；长按进入阈值设置界面
- **KEY2**：短按OLED切换显示页面；阈值界面切换修改项
- **KEY3**：短按手动控制LED；阈值界面数值+
- **KEY4**：短按手动控制水泵；阈值界面数值‑；长按保存阈值到EEPROM
- OLED共两页显示：第一页传感器实时数据；第二页执行器设备状态；阈值设置页修改各项告警阈值，保存后断电不丢失。

## ⚠️ 注意事项
1. 工程编译产物（Objects、Listings、axf、hex）、exe二进制被`.gitignore`过滤，不会提交仓库；需要exe请自行pyinstaller打包。
2. 华为云设备三元组、WiFi密码不要直接提交公开仓库，实际部署请本地修改。
3. FreeRTOS堆大小有限，RAM占用较高，新增功能注意监控内存。

## License
Apache‑2.0
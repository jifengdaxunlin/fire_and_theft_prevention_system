# Smart Warehouse Security System (fire_theif_proof)

**English** | **[中文](./README.md)**

A smart warehouse fire & intrusion prevention system based on **STM32F407VET6 + FreeRTOS + ESP8266 (MQTT / Huawei Cloud IoTDA)**. It integrates temperature/humidity, smoke, flame and human-presence detection with tiered alarm-linked actuator control, OLED local interaction, power-fail-safe threshold storage, and cloud property reporting — plus a PySide6 + OpenCV face-recognition host application for stranger detection.

## Features

- **Multi-sensor acquisition**: DHT11 temperature/humidity, MQ-2 smoke/combustible gas, infrared flame sensor, PIR motion sensor
- **Tiered alarm linkage** (priority: Flame > Gas > Stranger > Temperature > Humidity > Normal)
  - Fire alarm: water spray on + fan at full speed + buzzer + RGB status LED
  - Temperature / gas threshold exceeded: PWM fan exhaust only
  - Stranger (PIR trigger + face-recognition confirmation from host app): gate and audio-visual alarm linkage
- **Three operating modes**: Auto / Manual / Threshold-Setting, switched by keys
- **Local interaction**: two-page OLED UI (sensor data page / actuator status page) plus threshold-setting screen; thresholds persisted in EEPROM (AT24Cxx)
- **Cloud connectivity**: MQTT property reporting to Huawei Cloud IoTDA; RTC synced via SNTP
- **Face-recognition host app**: SmartFaceApp (PySide6 + OpenCV, LBPH model) sends a `STRANGER` command to the MCU over serial upon detecting an unknown face

## System Architecture

FreeRTOS multi-task design (4 tasks; shared data protected by the `xDataMutex` mutex):

| Task | Function | Stack (words) | Priority | Period | Responsibility |
|---|---|---|---|---|---|
| Control | `Task_ControlLogic` | 256 | 4 | 50 ms | Tiered alarm evaluation and actuator linkage |
| Sensor | `Task_SensorRead` | 256 | 3 | 500 ms | Read DHT11 / MQ-2 / flame IR / PIR |
| Network | `Task_Network` | 512 | 2 | 2000 ms | MQTT keep-alive and property reporting |
| Display | `Task_Display` | 512 | 1 | 200 ms | OLED UI refresh and status display |

```
 Sensors                    Control                    Interaction
┌────────┐   500ms   ┌──────────────┐   50ms   ┌──────────────┐
│ DHT11  │──────────▶│              │────────▶│ Buzzer / Fan  │
│ MQ-2   │           │ Task_Control │         │ Servo / Pump  │
│ Flame  │           │  (xDataMutex)│         │ RGB LED       │
│ PIR    │           └──────┬───────┘         └──────────────┘
└────────┘                  │                          ▲
       ▲                    ▼                          │
┌────────────┐      ┌──────────────┐           ┌──────────────┐
│ SmartFace  │─────▶│ Task_Network │──────────▶│ Task_Display │
│ Host (UART)│      │ ESP8266 MQTT │           │ OLED / KEY   │
└────────────┘      └──────────────┘           └──────────────┘
                    Huawei Cloud IoTDA          EEPROM / RTC
```

## Directory Structure

| Directory | Purpose |
|---|---|
| `app/` | Application entry `main.c`; full business logic of the 4 FreeRTOS tasks |
| `board/` | Board-level peripheral drivers (organized in per-peripheral subdirectories) |
| `module/` | Interrupt service `stm32f4xx_it.c`, peripheral config header `stm32f4xx_conf.h` |
| `Net/` | ESP8266 driver + MQTT protocol wrapper (WiFi / Huawei Cloud connection config) |
| `FreeRTOS/` | FreeRTOS V10.5.1 kernel sources and ARM_CM4F port |
| `libraries/` | STM32F4 StdPeriph Library V1.8.0 + CMSIS |
| `SmartFaceApp/` | Face-recognition host application (PySide6 + OpenCV, packaged exe) |
| `project/` | Keil MDK project `fire_theif_proof.uvprojx` and build outputs |

## Hardware Resource Mapping

| Peripheral | Resource | Pin | Notes |
|---|---|---|---|
| DHT11 temp/humidity | GPIO 1-wire | PB13 | Read every 500 ms |
| MQ-2 smoke | ADC1_IN3 | PA3 | Converted to percentage; ADC1 mutex |
| Flame IR | ADC1_IN2 | PA2 | Resistance-based flame detection |
| Buzzer | GPIO | PA5 | Plays note frequencies |
| RGB LED | TIM1 CH1/2/3 PWM | PE9 / PE11 / PE13 | Color indicates alarm type |
| OLED | Software I2C | PB8(SCL) / PB9(SDA) | GRAM-based refresh |
| EEPROM AT24Cxx | Software I2C | PB8 / PB9 | Shared bus with OLED; stores thresholds |
| Servo (gate) | TIM3_CH4 PWM | PC9 | 50 Hz; 0°/180° gate open/close |
| Water pump / valve | TIM3 PWM | PC8 | Activated on fire alarm |
| Fan | TIM3 PWM | PC6 / PC7 | Forward/reverse/brake; full speed (50) on fire, exhaust (40) on threshold breach |
| Debug UART USART1 | USART + DMA2_Stream7 | PA9(TX) / PA10(RX) | 115200; also receives `STRANGER` command from host app |
| ESP8266 | USART3 | PB10(TX) / PB11(RX) | 115200, AT command interface |
| Keys KEY1–KEY4 | EXTI + TIM7 | PG2 / PG3 / PG4 / PG5 | Falling-edge interrupt + 1 ms scan (short/long press) |
| TIM6 | Timer interrupt | — | 100 ms tick for RGB / buzzer blinking |
| TIM7 | Timer interrupt | — | 1 ms tick for key scanning and software timing |
| RTC | Alarm / WKUP interrupt | — | Real-time clock, SNTP-synced |

## Core Control Logic

`Task_ControlLogic` runs every 50 ms and evaluates alarms with the following priority:

```
Flame > Gas > Stranger > Temperature > Humidity > Normal
```

- **Fire alarm**: pump on + fan at full speed (PWM = 50) + buzzer + blinking red RGB
- **Temperature / gas exceeded**: fan exhaust only (PWM = 40)
- **Stranger**: gate linkage + audio-visual alarm (PIR trigger, confirmed by the face-recognition host app over serial)
- Three operating states: **Auto mode** (threshold-driven automatic linkage), **Manual mode** (direct key control), **Threshold-setting mode** (adjust and save thresholds to EEPROM)

## Cloud Interaction (Huawei Cloud IoTDA)

- WiFi and MQTT connection parameters are configured in `Net/esp8266.h` and `Net/mqtt.h` (WiFi SSID/password, device triad)
- MQTT broker: Huawei Cloud IoTDA (TCP 1883); SNTP time sync for the RTC
- Publish topic: `$oc/devices/{device_id}/sys/properties/report`
- Subscribe topic: `$oc/devices/{device_id}/sys/messages/down`
- Reported properties (service_id = `jifengdaxunlin`):

| Property | Meaning | Property | Meaning |
|---|---|---|---|
| `temp` | Temperature | `fan` | Fan state |
| `humi` | Humidity | `gate` | Gate state |
| `fire` | Flame alarm | `water` | Water spray state |
| `gas` | Gas concentration | `led` | LED state |
| `stranger` | Stranger alarm | | |

## Face-Recognition Host App (SmartFaceApp)

- Standalone packaged Windows executable (PyInstaller; bundles Python 3.10, OpenCV, numpy, PySide6)
- LBPH face-recognition model (`model/trainer.yml`) with OpenCV Haar cascade face detection
- Ships with one enrolled person `fyf` (training samples `face/fyf/1~20.jpg`) as a known face
- Communicates with the MCU over serial (USART1) and sends a `STRANGER` command when an unknown face is recognized

## Build & Flash

1. Open `project/fire_theif_proof.uvprojx` in Keil MDK
2. Target device: STM32F407VETx (target name `F407VE`)
3. Build to generate `fire_theif_proof.axf` / `fire_theif_proof.hex`
4. Flash via J-Link
5. Run `SmartFaceApp/SmartFaceApp.exe` to start the face-recognition host app (requires a serial connection to the host PC)

## Usage

- **KEY1–KEY4**: switch Auto/Manual mode, page through the OLED, adjust and save thresholds (short/long press supported)
- **Two-page OLED UI**: page 1 shows live sensor data, page 2 shows actuator status; threshold-setting mode allows adjusting temperature/humidity/gas/flame thresholds
- Thresholds are written to EEPROM on save and survive power cycles

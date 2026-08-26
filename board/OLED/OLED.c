#include "oled.h"
#include <stdio.h>

// 显存缓存区 (8页 * 128列 = 1024字节)
// 按【页优先】存储：GRAM[页][列]，保证每一页的 128 个字节在内存中连续，
// 这样 OLED_Refresh_Gram 才能一次把整页发出去。
static uint8_t OLED_GRAM[8][128];

// OLED 从机地址 (0x3C << 1)。若实测 NACK 会自动切到 0x7A (SA0 拉高)
static uint8_t OLED_SlaveAddr = 0x78;

// 探测指定地址能否收到 ACK，用于自动适配 0x78/0x7A 两种 OLED 地址
static uint8_t OLED_Probe_Address(uint8_t addr)
{
    uint8_t ack;
    IIC_Start();
    IIC_SendByte(addr);
    ack = IIC_SlaveAck();
    IIC_Stop();
    return (ack == 0) ? 1 : 0; // ack==0 表示从机应答正常
}

// 统一的 OLED 底层写入接口（直接复用总线）
static void OLED_Write_Packet(uint8_t addr_type, uint8_t *p_data, uint16_t len)
{
    // addr_type: 0x00 代表接下来发命令 (Co=0, D/C#=0)
    //            0x40 代表接下来发数据 (Co=0, D/C#=1)

    // 与 EEPROM 共用同一条 I2C 总线，必须持有互斥锁，
    // 否则 RTOS 任务里 OLED 刷新会与 EEPROM 读写竞争总线。
    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return; // 拿不到锁直接放弃本次传输，防止总线时序错乱
    }

    IIC_Start();
    IIC_SendByte(OLED_SlaveAddr); // OLED 写地址 (0x3C << 1)
    IIC_SlaveAck();
    IIC_SendByte(addr_type);
    IIC_SlaveAck();

    for(uint16_t i = 0; i < len; i++) {
        IIC_SendByte(p_data[i]);
        IIC_SlaveAck();
    }
    IIC_Stop();

    xSemaphoreGive(xI2C_Mutex);
}

// 写单条命令
void OLED_WriteCmd(uint8_t cmd) {
    uint8_t temp = cmd;
    OLED_Write_Packet(0x00, &temp, 1);
}

// 写单字节数据
void OLED_WriteData(uint8_t data) {
    uint8_t temp = data;
    OLED_Write_Packet(0x40, &temp, 1);
}

// 连续写入数据缓冲区
static void OLED_WriteDataBuffer(const uint8_t *buffer, uint16_t len) {
    if (len == 0 || buffer == NULL) return;
    
    // 为了防止过长导致总线卡死，可分包或直接整体发送
    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }

    IIC_Start();
    IIC_SendByte(OLED_SlaveAddr);
    IIC_SlaveAck();
    IIC_SendByte(0x40); // 连续数据模式
    IIC_SlaveAck();

    for (uint16_t i = 0; i < len; i++) {
        IIC_SendByte(buffer[i]);
        IIC_SlaveAck();
    }
    IIC_Stop();

    xSemaphoreGive(xI2C_Mutex);
}

// GPIO 及 I2C 引脚初始化
void OLED_Init_GPIO(void) {
    IIC_config();
}

// 设置硬件坐标（针对 SH1106 增加 +2 偏移量）
void OLED_SetPos(uint8_t x, uint8_t y) {
    x += 2; // SH1106 1.3寸屏硬件列地址偏移
    OLED_WriteCmd(0xB0 + (y & 0x07));         // 设置页地址 (0~7)
    OLED_WriteCmd(x & 0x0F);                    // 设置列低 4 位
    OLED_WriteCmd(0x10 | ((x >> 4) & 0x0F));    // 设置列高 4 位
}

// 清空显存缓冲区
void OLED_Clear_Gram(void) {
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 128; x++) {
            OLED_GRAM[y][x] = 0x00;
        }
    }
}

// 将显存缓冲区全部刷新到屏幕硬件
void OLED_Refresh_Gram(void) {
    for (uint8_t y = 0; y < 8; y++) {
        OLED_SetPos(0, y);
        // GRAM 页优先布局，&GRAM[y][0] 起就是这一页连续的 128 列字节
        OLED_WriteDataBuffer(&OLED_GRAM[y][0], 128);
    }
}

// 清除指定区域的显存
void OLED_ClearArea_Gram(uint8_t start_x, uint8_t end_x, uint8_t start_page, uint8_t end_page) {
    if (start_x > 127) start_x = 127;
    if (end_x > 127) end_x = 127;
    if (start_x > end_x) return;
    if (start_page > 7) start_page = 7;
    if (end_page > 7) end_page = 7;
    if (start_page > end_page) return;

    for (uint8_t y = start_page; y <= end_page; y++) {
        for (uint8_t x = start_x; x <= end_x; x++) {
            OLED_GRAM[y][x] = 0x00;
        }
    }
}

// SH1106 屏初始化序列
void OLED_Init(void) {
    printf("[OLED] >>> OLED_Init Start (Standard Bus) <<<\r\n");

    // 【关键】IIC 引脚必须先初始化为开漏输出，否则此前的所有 GPIO_SetBits/ResetBits 都是无效的
    IIC_config();

    // 上电稳压延时
    for (volatile uint32_t i = 0; i < 500000; i++);

    // 自动探测 OLED 从机地址：先试 0x78，NACK 则切 0x7A（部分 1.3 寸屏 SA0 默认拉高）
    OLED_SlaveAddr = 0x78;
    if (!OLED_Probe_Address(0x78)) {
        if (OLED_Probe_Address(0x7A)) {
            OLED_SlaveAddr = 0x7A;
            printf("[OLED] Address probe -> 0x7A\r\n");
        } else {
            printf("[OLED] WARNING: No ACK at 0x78 or 0x7A, check wiring/address!\r\n");
        }
    }

    OLED_WriteCmd(0xAE); // 关闭显示
    OLED_WriteCmd(0xD5); // 设置时钟分频
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); // 设置复用率
    OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3); // 设置显示偏移
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40); // 设置起始行
    OLED_WriteCmd(0x8D); // 电荷泵设置
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xA1); // 段重映射 (左右反转)
    OLED_WriteCmd(0xC8); // 行扫描顺序 (上下反转)
    OLED_WriteCmd(0xDA); // COM 引脚硬件配置
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81); // 对比度控制
    OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); // 预充电周期
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB); // VCOMH 脱扣电平
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4); // 全屏显示开启 (跟随 RAM)
    OLED_WriteCmd(0xA6); // 正常显示 (非反相)
    OLED_WriteCmd(0xAF); // 开启显示

    OLED_Clear_Gram();
    OLED_Refresh_Gram();
    
    printf("[OLED] >>> OLED_Init Finished successfully! <<<\r\n");
}

// 写入 8x8 字符到显存
void OLED_ShowStr_8x8(uint8_t x, uint8_t page, const char *str) {
    while (*str) {
        if (x > 120) { 
            x = 0;
            page += 1;
            if (page > 7) break;
        }

        uint8_t c = *str - ' ';
        for (uint8_t i = 0; i < 8; i++) {
            if ((x + i) < 128) {
                OLED_GRAM[page][x + i] = ascii_font_8x8[c][i];
            }
        }
        x += 8;
        str++;
    }
}

void OLED_ShowStr_5x8(uint8_t x, uint8_t page, const char *str) {
    while (*str) {
        if (x > 122) break; 
        uint8_t index = OLED_GetFont5x8Index(*str);
        for (uint8_t i = 0; i < 5; i++) {
            if ((x + i) < 128) {
                OLED_GRAM[page][x + i] = font_5x8_dateTime[index][i];
            }
        }
        if ((x + 5) < 128) {
            OLED_GRAM[page][x + 5] = 0x00;
        }
        x += 6;
        str++;
    }
}

void OLED_Display_Off(void) {
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x10);
    OLED_WriteCmd(0xAE);
}

void OLED_Display_On(void) {
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);
}

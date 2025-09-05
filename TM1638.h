#include "stm32l4xx_hal.h"

#define TM1638_CMD1 0x40
#define TM1638_CMD2 0xC0
#define TM1638_CMD3 0x80
#define TM1638_DSP_ON 0x08

class TM1638 {
private:
    GPIO_TypeDef* stbPort;
    uint16_t stbPin;
    GPIO_TypeDef* clkPort;
    uint16_t clkPin;
    GPIO_TypeDef* dioPort;
    uint16_t dioPin;
    uint8_t brightness;
    bool displayOn;

    void writeByte(uint8_t data) {
        for (int i = 0; i < 8; i++) {
            HAL_GPIO_WritePin(clkPort, clkPin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(dioPort, dioPin, (data >> i) & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(clkPort, clkPin, GPIO_PIN_SET);
        }
    }

    void command(uint8_t cmd) {
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_RESET);
        writeByte(cmd);
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_SET);
    }

    void writeDataCommand() {
        command(TM1638_CMD1);
    }

    void writeDisplayControl() {
        command(TM1638_CMD3 | (displayOn ? TM1638_DSP_ON : 0) | brightness);
    }

public:
    TM1638(GPIO_TypeDef* stbPort, uint16_t stbPin,
           GPIO_TypeDef* clkPort, uint16_t clkPin,
           GPIO_TypeDef* dioPort, uint16_t dioPin,
           uint8_t brightness = 7)
        : stbPort(stbPort), stbPin(stbPin),
          clkPort(clkPort), clkPin(clkPin),
          dioPort(dioPort), dioPin(dioPin),
          brightness(brightness), displayOn(true) {
        writeDisplayControl();
        clear();
    }

    void clear() {
        writeDataCommand();
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_RESET);
        writeByte(TM1638_CMD2);
        for (int i = 0; i < 16; i++) {
            writeByte(0x00);
        }
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_SET);
    }

    void showString(const char* str) {
        writeDataCommand();
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_RESET);
        for (int i = 0; i < 8 && str[i]; i++) {
            writeByte(TM1638_CMD2 | (i << 1));
            writeByte(encodeChar(str[i]));
        }
        HAL_GPIO_WritePin(stbPort, stbPin, GPIO_PIN_SET);
    }

    uint8_t encodeChar(char c) {
        const uint8_t seg[] = {
            0x3F, 0x06, 0x5B, 0x4F, 0x66,
            0x6D, 0x7D, 0x07, 0x7F, 0x6F
        };
        if (c >= '0' && c <= '9') return seg[c - '0'];
        return 0x00;
    }

    void setBrightness(uint8_t val) {
        if (val > 7) return;
        brightness = val;
        writeDisplayControl();
    }

    void power(bool on) {
        displayOn = on;
        writeDisplayControl();
    }
};

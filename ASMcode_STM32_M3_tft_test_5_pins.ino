//**********************************************************************************************

                                             //Asm STM32 Cortex M3

//**********************************************************************************************

#include <Arduino_GFX_Library.h>
#include "stm32f1xx.h"  // STM32F103 HAL definitions

// Pin definitions
const uint8_t pins[] = {PA1, PA2, PA3, PA4, PA5};
const char *pinNames[] = {"PA1", "PA2", "PA3", "PA4", "PA5"};

Arduino_GFX *gfx = nullptr;
int attemptCount = 0;

// ======== 🕒 FAST DELAY (150ms) USING ASSEMBLY ========
void fastDelay(uint32_t cycles) {
    asm volatile (
        "1: subs %0, #1 \n" // Subtract 1 from cycles
        "bne 1b \n"         // If not zero, branch to label 1
        : "=r" (cycles) 
        : "0" (cycles) 
    );
}

// ======== 🖨️ USART SERIAL PRINT FUNCTIONS ========
void serialPrintChar(char c) {
    while (!(USART1->SR & USART_SR_TXE)); // Wait for TX buffer to be empty
    USART1->DR = c; // Send character
}

void serialPrint(const char *str) {
    while (*str) {
        serialPrintChar(*str++);
    }
}

// ======== 🔍 TEST PIN COMBINATIONS ========
void tryCombination(uint8_t rst, uint8_t dc, uint8_t cs, uint8_t sck, uint8_t mosi,
                    const char *rstName, const char *dcName, const char *csName,
                    const char *sckName, const char *mosiName) 
{
    attemptCount++;

    serialPrint("Attempt ");
    char numStr[10];
    sprintf(numStr, "%d\n", attemptCount);
    serialPrint(numStr);

    serialPrint("RST: "); serialPrint(rstName); serialPrint(" ");
    serialPrint("DC: "); serialPrint(dcName); serialPrint(" ");
    serialPrint("CS: "); serialPrint(csName); serialPrint(" ");
    serialPrint("SCK: "); serialPrint(sckName); serialPrint(" ");
    serialPrint("MOSI: "); serialPrint(mosiName); serialPrint("\n");

    Arduino_DataBus *bus = new Arduino_SWSPI(dc, cs, sck, mosi, GFX_NOT_DEFINED);
    gfx = new Arduino_ST7735(bus, rst, 0);

    if (gfx->begin()) {
        serialPrint("✅ Success: Display initialized!\n");
        gfx->fillScreen(0xFFE0);
        fastDelay(3600000); // 🔥 150ms delay
    } else {
        serialPrint("❌ Failed: Display init error\n");
    }

    delete gfx;
    delete bus;
}

// ======== 🔄 OPTIMIZED PIN TESTING LOOP ========
void testPinCombinations() {
    size_t numPins = sizeof(pins) / sizeof(pins[0]);

    for (size_t i = 0; i < numPins; i++) {
        for (size_t j = 0; j < numPins; j++) {
            if (j == i) continue;
            for (size_t k = 0; k < numPins; k++) {
                if (k == i || k == j) continue;
                for (size_t l = 0; l < numPins; l++) {
                    if (l == i || l == j || l == k) continue;
                    for (size_t m = 0; m < numPins; m++) {
                        if (m == i || m == j || m == k || m == l) continue;

                        tryCombination(
                            pins[i], pins[j], pins[k], pins[l], pins[m],
                            pinNames[i], pinNames[j], pinNames[k], pinNames[l], pinNames[m]);
                    }
                }
            }
        }
    }
}

// ======== 🏁 STM32 SETUP FUNCTION ========
void setup() {
    // Enable GPIOA and USART1 clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    // Configure PA9 (TX) as Alternate Function Push-Pull
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1); // 50MHz, AF Push-Pull

    // Configure PA10 (RX) as Floating Input
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0; // Floating input

    // Set USART1 Baud Rate (38400 for 72MHz clock)
    USART1->BRR = 72000000 / 38400;  

    // Enable USART1 (TX & RX)
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    serialPrint("🚀 Optimized STM32F103 Pin Testing\n");
    testPinCombinations();
    serialPrint("✅ Testing Complete!\n");
}

// ======== 🔁 EMPTY LOOP (TESTING IN SETUP) ========
void loop() {}




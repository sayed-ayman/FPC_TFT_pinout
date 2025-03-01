//**********************************************************************************************

                                             //Asm STM32 Cortex M3

//**********************************************************************************************



#include <Arduino_GFX_Library.h>
#include "stm32f1xx.h"  // STM32F103 HAL definitions

#define GFX_BL DF_GFX_BL // Default backlight pin

// Pin definitions
const uint8_t pins[] = {PA1, PA2, PA3, PA4, PA5, PA7};
const char *pinNames[] = {"PA1", "PA2", "PA3", "PA4", "PA5", "PA7"};

Arduino_GFX *gfx = nullptr;
int attemptCount = 0;
int successCount = 0; // Track successful attempts

// Fast delay using ARM assembly (~500ms at 72MHz)
void fastDelay(uint32_t cycles) {
    asm volatile (
        "1: subs %0, #1 \n"
        "bne 1b \n"
        : "=r" (cycles) 
        : "0" (cycles) 
    );
}

// Optimized Serial Print (Direct USART)
void serialPrintChar(char c) {
    while (!(USART1->SR & USART_SR_TXE)); // Wait for TX buffer to be empty
    USART1->DR = c; // Send character
}

void serialPrint(const char *str) {
    while (*str) {
        serialPrintChar(*str++);
    }
}

void tryCombination(uint8_t rst, uint8_t dc, uint8_t cs, uint8_t sck, uint8_t mosi,
                    const char *rstName, const char *dcName, const char *csName,
                    const char *sckName, const char *mosiName) 
{
    attemptCount++;

    serialPrint("Attempt ");
    char numStr[10];
    sprintf(numStr, "%d\n", attemptCount);
    serialPrint(numStr);

    Arduino_DataBus *bus = new Arduino_SWSPI(dc, cs, sck, mosi, GFX_NOT_DEFINED);
    gfx = new Arduino_ST7735(bus, rst, 0);

    if (gfx->begin()) {
        successCount++;
        serialPrint("Success! Used Pins: \n");
        
        serialPrint("RST: "); serialPrint(rstName); serialPrint("\n");
        serialPrint("DC: "); serialPrint(dcName); serialPrint("\n");
        serialPrint("CS: "); serialPrint(csName); serialPrint("\n");
        serialPrint("SCK: "); serialPrint(sckName); serialPrint("\n");
        serialPrint("MOSI: "); serialPrint(mosiName); serialPrint("\n");
        
        gfx->fillScreen(0xFFE0);
        fastDelay(3600000); // ~500ms delay
    } else {
        serialPrint("Failed: gfx->begin() error\n");
    }

    delete gfx;
    delete bus;
}

// Optimized Pin Testing Loop
void testPinCombinations() {
    size_t numPins = sizeof(pins) / sizeof(pins[0]);

    for (size_t exclude = 0; exclude < numPins; exclude++) {
        for (size_t i = 0; i < numPins; i++) {
            if (i == exclude) continue;
            for (size_t j = 0; j < numPins; j++) {
                if (j == exclude || j == i) continue;
                for (size_t k = 0; k < numPins; k++) {
                    if (k == exclude || k == i || k == j) continue;
                    for (size_t l = 0; l < numPins; l++) {
                        if (l == exclude || l == i || l == j || l == k) continue;
                        for (size_t m = 0; m < numPins; m++) {
                            if (m == exclude || m == i || m == j || m == k || m == l) continue;

                            tryCombination(
                                pins[i], pins[j], pins[k], pins[l], pins[m],
                                pinNames[i], pinNames[j], pinNames[k], pinNames[l], pinNames[m]);
                        }
                    }
                }
            }
        }
    }
}

void setup() {
    // Enable USART1 and GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

    // Configure PA9 (TX) as Alternate Function Push-Pull
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1);

    // Configure PA10 (RX) as Floating Input
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // Set USART1 Baud Rate (38400 for 72MHz clock)
    USART1->BRR = 72000000 / 38400;  
    USART1->CR1 |= USART_CR1_TE | USART_CR1_UE; // Enable TX

    serialPrint("STM32F103 Pin Testing Started\n");
    testPinCombinations();

    // Final Summary
    serialPrint("Testing Complete\n");
    serialPrint("Total Attempts: ");
    char totalStr[10];
    sprintf(totalStr, "%d\n", attemptCount);
    serialPrint(totalStr);

    serialPrint("Successful Attempts: ");
    sprintf(totalStr, "%d\n", successCount);
    serialPrint(totalStr);
}

void loop() {
    // No need for a loop, testing is done in setup
}




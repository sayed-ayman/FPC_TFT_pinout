#include <SPI.h>  // Include SPI library

// Define possible pin configurations
const int pins[] = {PA1, PA2, PA3, PA4, PA5, PA7};
const char* pinNames[] = {"PA1", "PA2", "PA3", "PA4", "PA5", "PA7"};

// SPI speed: 27 MHz, MSB First, SPI Mode 0
SPISettings spiSettings(27000000, MSBFIRST, SPI_MODE0);

// List of display controllers
const char* controllers[] = {
    "ILI9341",   // دقة 240×320، شائع جدًا، يدعم SPI و 8080
    "ST7735",    // دقة 128×160، مناسب للشاشات الصغيرة
    "ILI9163",   // مشابه لـ ST7735 لكن مع دعم ألوان أفضل
    "S6D02A1",   // يستخدم في بعض شاشات 128×160، غير شائع
    "HX8357D",   // دقة 320×480، يدعم SPI و 8080، مناسب لشاشات 3.5 بوصة
    "ILI9481",   // دقة 320×480، يدعم 16/18 بت لون
    "ILI9486",   // دقة 320×480، يعمل عبر SPI ولكن أبطأ بسبب 18-bit لون
    "ILI9488",   // دقة 320×480، يشبه ILI9486 ولكن مع تحسينات طفيفة
    "ST7789",    // دقة 240×240، شائع في الشاشات الدائرية والمربعة
    "R61581",    // دقة 320×480، يستخدم في بعض الشاشات الأكبر
    "RM68140",   // مشابه لـ R61581 لكنه أقل انتشارًا
    "ST7796",    // دقة 320×480، يدعم SPI و RGB
    "SSD1351",   // شاشة OLED، دقة 128×128، تدعم SPI
    "SSD1963",   // دقة 800×480، تستخدم في الشاشات الكبيرة (5 بوصة أو أكثر)
    "ILI9225",   // دقة 176×220، نادر الاستخدام
    "GC9A01",    // دقة 240×240، مشهور في الشاشات الدائرية
    "GC9106",    // نادر الاستخدام
    "GC9107",    // مشابه لـ GC9106 ولكن مع اختلافات طفيفة
    "HX8347C",   // دقة 240×320، يدعم 8/16-bit 8080
    "HX8347D",   // مشابه لـ HX8347C مع تحسينات
    "HX8352C",   // دقة 240×400، قليل الاستخدام
    "HX8357A",   // دقة 320×480، مشابه لـ HX8357D
    "HX8357B",   // نسخة مختلفة من HX8357A
    "HX8369A",   // دقة 480×800، يستخدم في بعض الهواتف القديمة
    "ILI9342",   // نسخة محسنة من ILI9341
    "JBT6K71",   // نادر جدًا
    "NT35310",   // دقة 720×1280، غير شائع في المشاريع الصغيرة
    "NT35510",   // دقة 480×800، يستخدم في بعض الشاشات الكبيرة
    "NT39125",   // نادر جدًا
    "NV3041A",   // غير معروف جيدًا
    "R61529",    // دقة 320×480، مشابه لـ R61581
    "SEPS525",   // شاشة OLED، دقة 160×128
    "SSD1283A",  // شاشة OLED صغيرة
    "SSD1331",   // شاشة OLED، دقة 96×64، تدعم SPI
    "SSD1351"    // شاشة OLED، دقة 128×128، ألوان جميلة جدًا
};
bool enabledControllers[sizeof(controllers) / sizeof(controllers[0])];

// Variables for pin assignment
int MOSI_PIN, SCK_PIN, DC_PIN, CS_PIN, RESET_PIN, TE_PIN;
volatile bool TE_Flag = false;

// Interrupt handler for TE signal
void TE_ISR() {
    TE_Flag = true;
}

// Function to send 9-bit SPI data
void sendSPI9(uint16_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));  // Wait for SPI ready
    SPI1->DR = data & 0x01FF;  // Send only 9-bit data within 16-bit register
}

// Send command (9-bit: DC=0)
void LCD_SendCommand(uint8_t cmd) {
    digitalWrite(CS_PIN, LOW);
    sendSPI9(cmd & 0xFF);  // DC = 0 (Command)
    digitalWrite(CS_PIN, HIGH);
}

// Send data (9-bit: DC=1)
void LCD_SendData(uint8_t data) {
    digitalWrite(CS_PIN, LOW);
    sendSPI9(0x100 | (data & 0xFF));  // DC = 1 (Data)
    digitalWrite(CS_PIN, HIGH);
}

// Reset the LCD
void LCD_Reset() {
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    digitalWrite(RESET_PIN, HIGH);
    delay(100);
}

// Initialize LCD Controller
void LCD_Init(const char* controller) {
    LCD_Reset();

    if (strcmp(controller, "ILI9341") == 0) {
        LCD_SendCommand(0x11); delay(120);  // Sleep Out
        LCD_SendCommand(0x3A); LCD_SendData(0x55);  // 16-bit color
        LCD_SendCommand(0x36); LCD_SendData(0x48);
        LCD_SendCommand(0x29);  // Display ON
    } 
    else if (strcmp(controller, "ST7789V3") == 0) {
        LCD_SendCommand(0x11); delay(120);
        LCD_SendCommand(0x3A); LCD_SendData(0x55);
        LCD_SendCommand(0x36); LCD_SendData(0x00);
        LCD_SendCommand(0x29);
    }

    LCD_SendCommand(0x35);  // Enable TE signal
    LCD_SendData(0x00);
}

// Wait for TE signal before refreshing the screen
bool Wait_For_TE_Signal() {
    TE_Flag = false;
    unsigned long timeout = millis() + 2000;
    while (millis() < timeout) {
        if (TE_Flag) return true;
    }
    return false;
}

// Fill screen with red using 9-bit SPI
void Fill_Screen_Red() {
    LCD_SendCommand(0x2C);
    digitalWrite(CS_PIN, LOW);
    for (int i = 0; i < (320 * 480); i++) {
        sendSPI9(0x1F0);  // Red High Byte
        sendSPI9(0x100);  // Red Low Byte
    }
    digitalWrite(CS_PIN, HIGH);
}

// Select controllers via Serial Monitor
void SelectControllers() {
    Serial.println("🔹 Available Controllers:");
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(controllers[i]);
        enabledControllers[i] = false;
    }

    Serial.println("\n✍️ Enter controller numbers (e.g., 0 2 4):");
    while (!Serial.available());

    String input = Serial.readStringUntil('\n');
    input.trim();

    char* token = strtok((char*)input.c_str(), " ");
    while (token != NULL) {
        int index = atoi(token);
        if (index >= 0 && index < sizeof(controllers) / sizeof(controllers[0])) {
            enabledControllers[index] = true;
        }
        token = strtok(NULL, " ");
    }

    Serial.println("\n✅ Enabled Controllers:");
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        if (enabledControllers[i]) {
            Serial.println(controllers[i]);
        }
    }
}

// Try different pin combinations
void Find_Correct_Pins() {
    for (int c = 0; c < sizeof(controllers) / sizeof(controllers[0]); c++) {
        if (!enabledControllers[c]) continue;

        Serial.print("🖥️ Testing: ");
        Serial.println(controllers[c]);

        for (int m = 0; m < 6; m++) {
            for (int s = 0; s < 6; s++) {
                if (s == m) continue;
                for (int d = 0; d < 6; d++) {
                    if (d == m || d == s) continue;
                    for (int cs = 0; cs < 6; cs++) {
                        if (cs == m || cs == s || cs == d) continue;
                        for (int r = 0; r < 6; r++) {
                            if (r == m || r == s || r == d || r == cs) continue;
                            for (int t = 0; t < 6; t++) {
                                if (t == m || t == s || t == d || t == r || t == cs) continue;

                                MOSI_PIN = pins[m];
                                SCK_PIN = pins[s];
                                DC_PIN = pins[d];
                                CS_PIN = pins[cs];
                                RESET_PIN = pins[r];
                                TE_PIN = pins[t];

                                Serial.print("MOSI="); Serial.print(pinNames[m]);
                                Serial.print(", SCK="); Serial.print(pinNames[s]);
                                Serial.print(", DC="); Serial.print(pinNames[d]);
                                Serial.print(", CS="); Serial.print(pinNames[cs]);
                                Serial.print(", RESET="); Serial.print(pinNames[r]);
                                Serial.print(", TE="); Serial.println(pinNames[t]);

                                SPI.begin();
                                pinMode(MOSI_PIN, OUTPUT);
                                pinMode(SCK_PIN, OUTPUT);
                                pinMode(DC_PIN, OUTPUT);
                                pinMode(CS_PIN, OUTPUT);
                                pinMode(RESET_PIN, OUTPUT);
                                pinMode(TE_PIN, INPUT);

                                attachInterrupt(digitalPinToInterrupt(TE_PIN), TE_ISR, RISING);

                                LCD_Init(controllers[c]);
                                if (Wait_For_TE_Signal()) {
                                    Serial.println("✅ Correct Configuration Found!");
                                    Fill_Screen_Red();
                                }

                                detachInterrupt(digitalPinToInterrupt(TE_PIN));
                            }
                        }
                    }
                }
            }
        }
    }
}

// Setup function
void setup() {
    Serial.begin(115200);
    delay(2000);
    SelectControllers();
    Find_Correct_Pins();
}

// Loop function (not used)
void loop() {
    delay(1000);
}

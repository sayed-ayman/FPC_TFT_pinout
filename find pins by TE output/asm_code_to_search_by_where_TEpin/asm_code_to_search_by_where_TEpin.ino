#include <SPI.h>

// دالة تأخير دقيق لمدة 100 ملي ثانية باستخدام ASM
void delay_100ms() {
    __asm__ volatile (
        "mov r0, #50000 \n" /* تقسيم التأخير إلى عدة دورات أصغر */ 
        "delay_loop_A: subs r0, r0, #1 \n"
        "bne delay_loop_A \n"
        ::: "r0"
    );
}

// قائمة المنافذ الممكنة
uint8_t pins[] = {PA1, PA2, PA3, PA4, PA5, PA7};
const char* pinNames[] = {"PA1", "PA2", "PA3", "PA4", "PA5", "PA7"};

// إعداد SPI بسرعة 27MHz
SPISettings spiSettings(27000000, MSBFIRST, SPI_MODE0);

// قائمة متحكمات الشاشات مع إمكانية تفعيل أو تعطيل كل منها
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

// مصفوفة لتحديد المتحكمات المفعلة
bool enabledControllers[sizeof(controllers) / sizeof(controllers[0])];

// عدد المحاولات الإجمالي
int total_attempts = 0;
int current_attempt = 0;

// متغيرات التوصيل
uint8_t MOSI_PIN, SCK_PIN, DC_PIN, RESET_PIN, TE_PIN;
volatile bool TE_Flag = false;

// وظيفة المقاطعة عند استقبال TE Signal
void TE_ISR() {
    TE_Flag = true;
}

// إرسال أمر عبر SPI
void LCD_SendCommand(uint8_t cmd) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(DC_PIN, LOW);
    SPI.transfer(cmd);
    SPI.endTransaction();
}

// إرسال بيانات عبر SPI
void LCD_SendData(uint8_t data) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(DC_PIN, HIGH);
    SPI.transfer(data);
    SPI.endTransaction();
}

// إعادة تشغيل الشاشة
void LCD_Reset() {
    digitalWrite(RESET_PIN, LOW);
    delay_100ms();
    digitalWrite(RESET_PIN, HIGH);
    delay_100ms();
}

// تهيئة الشاشة
void LCD_Init(const char* controller) {
    LCD_Reset();
    LCD_SendCommand(0x35); // تمكين TE Signal
    LCD_SendData(0x00);
}

// انتظار TE Signal باستخدام المقاطعة
bool Wait_For_TE_Signal() {
    TE_Flag = false;
    unsigned long timeout = millis() + 2000;
    while (millis() < timeout) {
        if (TE_Flag) return true;
    }
    return false;
}

// ملء الشاشة باللون الأحمر
void Fill_Screen_Red() {
    LCD_SendCommand(0x2C);
    for (uint32_t i = 0; i < (320 * 480); i++) {
        SPI.transfer(0xF8);
        SPI.transfer(0x00);
    }
}

// اختيار المتحكمات عبر Serial Monitor
void SelectControllers() {
    Serial.println("🔹 قائمة متحكمات الشاشات المتاحة:");
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(controllers[i]);
        enabledControllers[i] = false; // تعطيل جميع المتحكمات في البداية
    }

    Serial.println("\n✍️ أدخل أرقام المتحكمات التي تريد اختبارها، مفصولة بمسافات (مثلاً: 0 1 3):");
    while (!Serial.available()) {} // انتظار الإدخال من المستخدم

    String input = Serial.readStringUntil('\n'); // قراءة الإدخال
    input.trim();

    char* token = strtok((char*)input.c_str(), " ");
    while (token != NULL) {
        int index = atoi(token);
        if (index >= 0 && index < sizeof(controllers) / sizeof(controllers[0])) {
            enabledControllers[index] = true; // تفعيل المتحكم المحدد
        }
        token = strtok(NULL, " ");
    }

    Serial.println("\n✅ المتحكمات المفعلة:");
    total_attempts = 0;
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        if (enabledControllers[i]) {
            Serial.println(controllers[i]);
            total_attempts += 6 * 5 * 4 * 3 * 2; // حساب عدد المحاولات لكل متحكم
        }
    }

    Serial.print("🔢 إجمالي المحاولات: ");
    Serial.println(total_attempts);
}

// تجربة جميع التوصيلات الممكنة
void Find_Correct_Pins() {
    current_attempt = 0;

    for (int c = 0; c < sizeof(controllers) / sizeof(controllers[0]); c++) {
        if (!enabledControllers[c]) continue;

        Serial.print("🖥️ تجربة معالج الشاشة: ");
        Serial.println(controllers[c]);

        for (int m = 0; m < 6; m++) {
            for (int s = 0; s < 6; s++) {
                if (s == m) continue;
                for (int d = 0; d < 6; d++) {
                    if (d == m || d == s) continue;
                    for (int r = 0; r < 6; r++) {
                        if (r == m || r == s || r == d) continue;
                        for (int t = 0; t < 6; t++) {
                            if (t == m || t == s || t == d || t == r) continue;

                            current_attempt++;
                            int remaining_attempts = total_attempts - current_attempt;

                            MOSI_PIN = pins[m];
                            SCK_PIN = pins[s];
                            DC_PIN = pins[d];
                            RESET_PIN = pins[r];
                            TE_PIN = pins[t];

                            Serial.println("🔍 تجربة التوصيل:");
                            Serial.print("📌 محاولة ");
                            Serial.print(current_attempt);
                            Serial.print(" من ");
                            Serial.println(total_attempts);
                            Serial.print("🟡 المحاولات المتبقية: ");
                            Serial.println(remaining_attempts);
                            Serial.print("MOSI=");
                            Serial.print(pinNames[m]);
                            Serial.print(", SCK=");
                            Serial.print(pinNames[s]);
                            Serial.print(", DC=");
                            Serial.print(pinNames[d]);
                            Serial.print(", RESET=");
                            Serial.print(pinNames[r]);
                            Serial.print(", TE=");
                            Serial.println(pinNames[t]);

                            SPI.begin();
                            pinMode(MOSI_PIN, OUTPUT);
                            pinMode(SCK_PIN, OUTPUT);
                            pinMode(DC_PIN, OUTPUT);
                            pinMode(RESET_PIN, OUTPUT);
                            pinMode(TE_PIN, INPUT);

                            attachInterrupt(digitalPinToInterrupt(TE_PIN), TE_ISR, RISING);

                            LCD_Init(controllers[c]);
                            if (Wait_For_TE_Signal()) {
                                Serial.println("✅ تم العثور على التوصيل الصحيح!");
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

void setup() {
    Serial.begin(115200);
    delay(2000);
    SelectControllers();
    Find_Correct_Pins();
}

void loop() {
    delay(1000);
}

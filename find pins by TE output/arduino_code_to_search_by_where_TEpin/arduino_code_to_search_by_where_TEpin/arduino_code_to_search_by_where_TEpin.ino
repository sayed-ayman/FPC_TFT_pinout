#include <SPI.h>  // تضمين مكتبة SPI لاستخدام بروتوكول الاتصال

// تعريف المنافذ الممكنة لاستخدامها مع الشاشة
const int pins[] = {PA1, PA2, PA3, PA4, PA5, PA7};  
const char* pinNames[] = {"PA1", "PA2", "PA3", "PA4", "PA5", "PA7"};

// إعداد سرعة SPI إلى 27 ميجاهرتز
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

// مصفوفة لتحديد المتحكمات التي سيتم تجربتها
bool enabledControllers[sizeof(controllers) / sizeof(controllers[0])];

// متغيرات لحفظ المنافذ المستخدمة
int MOSI_PIN, SCK_PIN, DC_PIN, RESET_PIN, TE_PIN;
volatile bool TE_Flag = false;  // متغير لحفظ حالة TE Signal

// دالة المقاطعة لتحديث TE_Flag عند استقبال TE Signal
void TE_ISR() {
    TE_Flag = true;
}

// دالة لإرسال الأوامر إلى الشاشة
void LCD_SendCommand(uint8_t cmd) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(DC_PIN, LOW); // وضع DC في وضع الأوامر
    SPI.transfer(cmd);
    SPI.endTransaction();
}

// دالة لإرسال البيانات إلى الشاشة
void LCD_SendData(uint8_t data) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(DC_PIN, HIGH); // وضع DC في وضع البيانات
    SPI.transfer(data);
    SPI.endTransaction();
}

// دالة إعادة تشغيل الشاشة
void LCD_Reset() {
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    digitalWrite(RESET_PIN, HIGH);
    delay(100);
}

// دالة لتهيئة الشاشة وتفعيل TE Signal
void LCD_Init(const char* controller) {
    LCD_Reset();
    LCD_SendCommand(0x35);  // تفعيل TE Signal
    LCD_SendData(0x00);
}

// دالة لانتظار TE Signal حتى يتم تحديث الشاشة بشكل متزامن
bool Wait_For_TE_Signal() {
    TE_Flag = false;
    unsigned long timeout = millis() + 2000;
    while (millis() < timeout) {
        if (TE_Flag) return true;
    }
    return false;
}

// دالة لملء الشاشة باللون الأحمر
void Fill_Screen_Red() {
    LCD_SendCommand(0x2C);
    for (int i = 0; i < (320 * 480); i++) {
        SPI.transfer(0xF8);
        SPI.transfer(0x00);
    }
}

// دالة لاختيار المتحكمات عبر Serial Monitor
void SelectControllers() {
    Serial.println("🔹 قائمة المتحكمات المتاحة:");
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(controllers[i]);
        enabledControllers[i] = false; // تعطيل جميع المتحكمات في البداية
    }

    Serial.println("\n✍️ أدخل أرقام المتحكمات المراد تجربتها (مثال: 0 2 4):");
    while (!Serial.available());  // انتظار الإدخال من Serial Monitor

    String input = Serial.readStringUntil('\n'); // قراءة الإدخال
    input.trim();  // إزالة أي مسافات زائدة

    char* token = strtok((char*)input.c_str(), " ");  // تقسيم النص
    while (token != NULL) {
        int index = atoi(token);  // تحويل النص إلى رقم
        if (index >= 0 && index < sizeof(controllers) / sizeof(controllers[0])) {
            enabledControllers[index] = true;  // تفعيل المتحكم المحدد
        }
        token = strtok(NULL, " ");
    }

    Serial.println("\n✅ المتحكمات المفعلة:");
    for (int i = 0; i < sizeof(controllers) / sizeof(controllers[0]); i++) {
        if (enabledControllers[i]) {
            Serial.println(controllers[i]);
        }
    }
}

// دالة لتجربة جميع التوصيلات الممكنة للمتحكمات المحددة
void Find_Correct_Pins() {
    for (int c = 0; c < sizeof(controllers) / sizeof(controllers[0]); c++) {
        if (!enabledControllers[c]) continue;  // تجاهل المتحكمات غير المفعلة

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

                            // تحديد المنافذ الحالية
                            MOSI_PIN = pins[m];
                            SCK_PIN = pins[s];
                            DC_PIN = pins[d];
                            RESET_PIN = pins[r];
                            TE_PIN = pins[t];

                            // طباعة تفاصيل التجربة الحالية
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

                            // تهيئة المنافذ
                            SPI.begin();
                            pinMode(MOSI_PIN, OUTPUT);
                            pinMode(SCK_PIN, OUTPUT);
                            pinMode(DC_PIN, OUTPUT);
                            pinMode(RESET_PIN, OUTPUT);
                            pinMode(TE_PIN, INPUT);

                            attachInterrupt(digitalPinToInterrupt(TE_PIN), TE_ISR, RISING);

                            // تهيئة الشاشة وتجربة التوصيل
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

// الدالة الرئيسية للتشغيل
void setup() {
    Serial.begin(115200);
    delay(2000);
    SelectControllers();  // السماح للمستخدم بتحديد المتحكمات
    Find_Correct_Pins();  // بدء تجربة التوصيلات
}

// الدالة الرئيسية للتكرار (غير مستخدمة حاليًا)
void loop() {
    delay(1000);
}

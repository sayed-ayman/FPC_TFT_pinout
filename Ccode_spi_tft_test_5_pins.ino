//**********************************************************************************************

                                             //C arduino

//**********************************************************************************************



#include <Arduino_GFX_Library.h>


// Pin group definitions
const uint8_t pins[] = {PA5, PA2, PA4, PA1, PA7}; // Symbolic pin names used directly
const char *pinNames[] = {"PA5", "PA2", "PA4", "PA1", "PA7"}; // Human-readable names

Arduino_GFX *gfx = nullptr;
int attemptCount = 0; // Variable to track the number of attempts

void tryCombination(uint8_t rst, uint8_t dc, uint8_t cs, uint8_t sck, uint8_t mosi,
                    const char *rstName, const char *dcName, const char *csName,
                    const char *sckName, const char *mosiName)
{
    attemptCount++; // Increment the attempt counter
    Serial.print("Attempt ");
    Serial.print(attemptCount);
    Serial.println(":");

    Serial.print("RST: ");
    Serial.println(rstName);
    Serial.print("DC: ");
    Serial.println(dcName);
    Serial.print("CS: ");
    Serial.println(csName);
    Serial.print("SCK: ");
    Serial.println(sckName);
    Serial.print("MOSI: ");
    Serial.println(mosiName);

    Arduino_DataBus *bus = new Arduino_SWSPI(dc, cs, sck, mosi, GFX_NOT_DEFINED /* MISO */);
    gfx = new Arduino_ST7735(bus, rst, 0 /* rotation */);

    if (gfx->begin())
    {
        Serial.println("Success: gfx->begin() succeeded!");
        gfx->fillScreen(0xFFE0);
        delay(100); // Show success color for 2 seconds
    }
    else
    {
        Serial.println("Failed: gfx->begin() failed.");
    }

    // Clean up memory
    delete gfx;
    delete bus;
}

void setup(void)
{
    Serial.begin(38400);
    Serial.println("Arduino_GFX Pin Testing");

    // Iterate over all possible combinations of pins
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++)
    {
        for (size_t j = 0; j < sizeof(pins) / sizeof(pins[0]); j++)
        {
            if (j == i) continue;
            for (size_t k = 0; k < sizeof(pins) / sizeof(pins[0]); k++)
            {
                if (k == i || k == j) continue;
                for (size_t l = 0; l < sizeof(pins) / sizeof(pins[0]); l++)
                {
                    if (l == i || l == j || l == k) continue;
                    for (size_t m = 0; m < sizeof(pins) / sizeof(pins[0]); m++)
                    {
                        if (m == i || m == j || m == k || m == l) continue;

                        // Test the current combination with corresponding names
                        tryCombination(
                            pins[i], pins[j], pins[k], pins[l], pins[m],
                            pinNames[i], pinNames[j], pinNames[k], pinNames[l], pinNames[m]);
                    }
                }
            }
        }
    }

    Serial.println("Pin testing completed!");
}

void loop()
{}


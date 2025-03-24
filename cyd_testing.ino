#include <Wire.h>
#include <test.h>
#include <static_cyd_icons.h>
#include <XPT2046_Touchscreen.h>
#include<SD.h>
#include<Ticker.h>
Ticker ticker;
// Color Definitions (RGB565 format)
#define TFT_PURPLE       0x8010  // Purple
#define TFT_YELLOW       0xFFE0  // Yellow
#define TFT_GOLD         0xFEA0  // Gold
#define TFT_DARK_PURPLE  0x4010  // Dark Purple
#define TFT_LIGHT_PURPLE 0xC018  // Light Purple
#define TFT_DARK_YELLOW  0x8C40  // Dark Yellow
#define TFT_LIGHT_YELLOW 0xFFF0  // Light Yellow
#define TFT_ORANGE       0xFD20  // Orange
#define TFT_DARK_ORANGE  0xFC00  // Dark Orange
#define TFT_LIGHT_ORANGE 0xFE60  // Light Orange
#define TFT_RED          0xF800  // Red
#define TFT_DARK_RED     0x8000  // Dark Red
#define TFT_LIGHT_RED    0xFC10  // Light Red
#define TFT_GREEN        0x07E0  // Green
#define TFT_DARK_GREEN   0x03E0  // Dark Green
#define TFT_LIGHT_GREEN  0x8FE0  // Light Green
#define TFT_BLUE         0x001F  // Blue
#define TFT_DARK_BLUE    0x0010  // Dark Blue
#define TFT_LIGHT_BLUE   0x02BF  // Light Blue
#define TFT_CYAN         0x07FF  // Cyan
#define TFT_DARK_CYAN    0x03EF  // Dark Cyan
#define TFT_LIGHT_CYAN   0x87FF  // Light Cyan
#define TFT_MAGENTA      0xF81F  // Magenta
#define TFT_DARK_MAGENTA 0x8010  // Dark Magenta
#define TFT_LIGHT_MAGENTA 0xFC1F // Light Magenta
#define TFT_PINK         0xFC18  // Pink
#define TFT_DARK_PINK    0x8010  // Dark Pink
#define TFT_LIGHT_PINK   0xFE19  // Light Pink
#define TFT_BROWN        0xA145  // Brown
#define TFT_DARK_BROWN   0x5145  // Dark Brown
#define TFT_LIGHT_BROWN  0xC185  // Light Brown
#define TFT_WHITE        0xFFFF  // White
#define TFT_BLACK        0x0000  // Black
#define TFT_GRAY         0x8410  // Gray
#define TFT_DARK_GRAY    0x4208  // Dark Gray
#define TFT_LIGHT_GRAY   0xC618  // Light Gray
// Touchscreen pins
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

int main_manue_length = 9;

bool homePressed = false;
bool backPressed = false;
bool forwardPressed = false;
volatile bool touchDetected = false; 

int counter = 0;
bool homeFlag = false;

void printTouchToSerial(void);
void handleTouchEvent(void);
void drawLayout(void);
void displaySystemMetrics(void);
void drawProgressBar(int x, int y, int width, int height, float percent, uint16_t color, const char* label, float total, float used);
void updateCounterDisplay(void);

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
int x, y, z;


void manue_selection(int a);
void manue_selection(int a){
  static int prev_a = 0;  // Stores the previous selection
switch(prev_a) {
  case 1: tft.drawRoundRect(10, 10, 70, 70, 5, TFT_BLACK); break;
  case 2: tft.drawRoundRect(85, 10, 70, 70, 5, TFT_BLACK); break;
  case 3: tft.drawRoundRect(160, 10, 70, 70, 5, TFT_BLACK); break;
  case 4: tft.drawRoundRect(10, 85, 70, 70, 5, TFT_BLACK); break;
  case 5: tft.drawRoundRect(85, 85, 70, 70, 5, TFT_BLACK); break;
  case 6: tft.drawRoundRect(160, 85, 70, 70, 5, TFT_BLACK); break;
  case 7: tft.drawRoundRect(10, 160, 70, 70, 5, TFT_BLACK); break;
  case 8: tft.drawRoundRect(85, 160, 70, 70, 5, TFT_BLACK); break;
  case 9: tft.drawRoundRect(160, 160, 70, 70, 5, TFT_BLACK); break;
}

switch(a) {
  case 1: tft.drawRoundRect(10, 10, 70, 70, 5, TFT_RED); break;
  case 2: tft.drawRoundRect(85, 10, 70, 70, 5, TFT_RED); break;
  case 3: tft.drawRoundRect(160, 10, 70, 70, 5, TFT_RED); break;
  case 4: tft.drawRoundRect(10, 85, 70, 70, 5, TFT_RED); break;
  case 5: tft.drawRoundRect(85, 85, 70, 70, 5, TFT_RED); break;
  case 6: tft.drawRoundRect(160, 85, 70, 70, 5, TFT_RED); break;
  case 7: tft.drawRoundRect(10, 160, 70, 70, 5, TFT_RED); break;
  case 8: tft.drawRoundRect(85, 160, 70, 70, 5, TFT_RED); break;
  case 9: tft.drawRoundRect(160, 160, 70, 70, 5, TFT_RED); break;
}
prev_a = a;
}

void manue(int icon_number);
void manue(int icon_number){
  laptop_static(15,15);
  gear_static(90,15);
  technology_static(165,15);
  ruler_static(15,90);
  calculator_static(90,90);
  engineering_static(165,90);
  plan_static(15,165);
  prototype_static(90,165);
  robotic_arm_static(165,165);
}
void setup() 
            {
                Serial.begin(115200);
                cyd_initialize();
                logo_cyd_static(0,0);
                delay(2000);
                tft.fillScreen(TFT_BLACK);    
                touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
                touchscreen.begin(touchscreenSPI);  
                touchscreen.setRotation(0);
                drawLayout();
                manue(1);
                //ticker.attach(0.2,displaySystemMetrics );
              }

void loop() 
{
  //manue_selection(counter);
  // Your other code here
    handleTouchEvent();
   // delay(1);  
   // printTouchToSerial();
  // updateCounterDisplay();

}
void printTouchToSerial(void) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 345, 3840, 1, 240);
    y = map(p.y, 240, 4100, 1, 360);
    z = p.z;
    Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();
}
}
void handleTouchEvent() {
    if (touchscreen.touched()) {
        // Get touch coordinates
        TS_Point p = touchscreen.getPoint();
        int x = map(p.x, 345, 3840, 1, 240);  // Calibrate X
        int y = map(p.y, 240, 4100, 1, 360);  // Calibrate Y

        // Handle Home button press
        if (x >= 104 && x <= 136 && y >= 284 && y <= 316) {  // Home button area
            tft.fillCircle(120, 300, 16, TFT_GREEN);  // Fill home button
            delay(200);
            tft.fillCircle(120, 300, 16, TFT_BLACK);
            tft.drawCircle(120, 300, 16, TFT_RED);  // Unfill home button

            // Add your home button logic here
        }

        // Handle Back (left) button press
        else if (x >= 20 && x <= 45 && y >= 285 && y <= 315) {  // Back button area
            tft.fillTriangle(45, 285, 20, 300, 45, 315, TFT_GREEN);  // Fill back button
            delay(200);
            tft.fillTriangle(45, 285, 20, 300, 45, 315, TFT_BLACK); 
            tft.drawTriangle(45, 285, 20, 300, 45, 315, TFT_RED); 
            counter --;
                    }

        // Handle Forward (right) button press
        else if (x >= 195 && x <= 220 && y >= 285 && y <= 315) {  // Forward button area
            tft.fillTriangle(195, 285, 220, 300, 195, 315, TFT_GREEN);  // Fill forward button
            delay(200);
            tft.fillTriangle(195, 285, 220, 300, 195, 315, TFT_BLACK);
            tft.drawTriangle(195, 285, 220, 300, 195, 315, TFT_RED);
            counter ++;
        }
        if(counter>main_manue_length){counter = main_manue_length ;}
        else if (counter < 1 ){counter = 1;}
        manue_selection(counter);
    } 

}

// Function to display RAM, ROM, and CPU usage
void displaySystemMetrics() {
    // Calculate RAM usage
    uint32_t freeRAM = esp_get_free_heap_size()/1024;  // Free RAM in bytes
    uint32_t totalRAM = ESP.getHeapSize()/1024;        // Total RAM in bytes
    uint32_t usedRAM = totalRAM - freeRAM;        // Used RAM in bytes
    float ramUsagePercent = (usedRAM * 100.0) / totalRAM;  // RAM usage in percentage

    // Calculate ROM usage
    uint32_t freeROM = ESP.getFreeSketchSpace()/1024;  // Free ROM in bytes
    uint32_t totalROM = ESP.getFlashChipSize()/1024;   // Total ROM in bytes
    uint32_t usedROM = totalROM - freeROM;        // Used ROM in bytes
    float romUsagePercent = (usedROM * 100.0) / totalROM;  // ROM usage in percentage

    // Estimate CPU usage (alternative method)
    static uint32_t lastTime = millis();
    static uint32_t lastWorkTime = 0;
    uint32_t currentTime = millis();
    uint32_t workTime = currentTime - lastTime;  // Time spent doing work
    float cpuUsagePercent = (workTime * 100.0) / (currentTime - lastWorkTime);  // CPU usage in percentage
    lastTime = currentTime;
    lastWorkTime = workTime;

    // Get CPU frequency
    uint32_t cpuFreq = getCpuFrequencyMhz();  // Get CPU frequency in MHz

    // Check if SD card is detected
    bool sdCardDetected = SD.begin();  // Initialize SD card and check if detected
    float sdCardTotalMB = 0;
    float sdCardUsedMB = 0;
    float sdCardUsagePercent = 0;

    if (sdCardDetected) {
        // Calculate SD card storage
        uint32_t sdCardTotalBytes = SD.totalBytes();  // Total SD card size in bytes
        uint32_t sdCardUsedBytes = SD.usedBytes();    // Used SD card size in bytes
        sdCardTotalMB = sdCardTotalBytes / (1024.0 * 1024.0);  // Convert to MB
        sdCardUsedMB = sdCardUsedBytes / (1024.0 * 1024.0);    // Convert to MB
        sdCardUsagePercent = (sdCardUsedBytes * 100.0) / sdCardTotalBytes;  // SD card usage in percentage
    }

    // Calculate program memory usage
    uint32_t programSize = ESP.getSketchSize();  // Program size in bytes
    uint32_t freeProgramSpace = ESP.getFreeSketchSpace();  // Free program space in bytes
    float programUsagePercent = (programSize * 100.0) / (programSize + freeProgramSpace);  // Program memory usage in percentage

    // Clear the display area
    tft.fillRect(10, 10, 220, 150, TFT_BLACK);  // Clear the metrics display area

    // Draw progress bars with percentage on the left and values on the right
    drawProgressBar(10, 10, 220, 20, ramUsagePercent, TFT_LIGHT_RED, "RAM", totalRAM / 1024.0, usedRAM / 1024.0);  // RAM bar (in KB)
    drawProgressBar(10, 50, 220, 20, romUsagePercent, TFT_LIGHT_PURPLE, "ROM", totalROM / 1024.0, usedROM / 1024.0);  // ROM bar (in KB)
    drawProgressBar(10, 90, 220, 20, cpuUsagePercent, TFT_LIGHT_GREEN, "CPU", 240, cpuFreq);  // CPU bar (percentage)

    // Display SD card information
    if (sdCardDetected) {
        drawProgressBar(10, 170, 220, 20, sdCardUsagePercent, TFT_LIGHT_BLUE, "SD Card", sdCardTotalMB, sdCardUsedMB);  // SD card bar (in MB)
    } 
  

    // Display program memory usage
    drawProgressBar(10, 130, 220, 20, programUsagePercent, TFT_LIGHT_CYAN, "Program", (programSize + freeProgramSpace) / 1024.0, programSize / 1024.0);  // Program memory bar (in KB)

    // Display counter value
    tft.fillRect(60,240, 120, 20, TFT_BLACK);  // Clear the counter value area
    tft.setTextColor(TFT_WHITE, TFT_BLACK);     // Set text color to white on black
    tft.setTextSize(2);                         // Set text size
    tft.setCursor(70, 260);                     // Set cursor position
    tft.print("Count: ");                       // Display label
    tft.print(counter);                         // Display counter value
}
// Function to draw a progress bar
void drawProgressBar(int x, int y, int width, int height, float percent, uint16_t color, const char* label, float total, float used) {
    // Draw the background of the progress bar
    tft.drawRoundRect(x, y, width, height, 10, TFT_WHITE);

    // Calculate the filled width based on the percentage
    int filledWidth = (percent / 100.0) * width;

    // Draw the filled portion of the progress bar
    tft.fillRoundRect(x + 1, y + 1, filledWidth - 2, height - 2, 9, color);

    // Display the label and percentage on the left
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(x + 5, y + height + 5);  // Left side
    tft.print(label);
    tft.print(": ");
    tft.print(percent, 1);  // Display percentage
    tft.print(" %");

    // Display remaining/total values on the right
    tft.setCursor(x + width - 80, y + height + 5);  // Right side
    tft.print(used, 1);  // Display used value
    tft.print(" / ");
    tft.print(total, 1);  // Display total value
}
// Function to draw the initial layout
void drawLayout() {
    // Draw layout
    tft.drawRoundRect(0, 0, 240, 320, 5, TFT_GREEN);
    tft.drawRoundRect(0, 280, 240, 40, 5, TFT_GREEN);

    // Draw buttons
    tft.drawTriangle(45, 285, 20, 300, 45, 315, TFT_RED);  // Back button
    tft.drawCircle(120, 300, 16, TFT_RED);                 // Home button
    tft.drawTriangle(195, 285, 220, 300, 195, 315, TFT_RED);  // Forward button
}

void updateCounterDisplay() {
    tft.fillRect(60, 60, 120, 20, TFT_BLACK);  // Clear the counter value area
    tft.setTextColor(TFT_WHITE, TFT_BLACK);    // Set text color to white on black
    tft.setTextSize(2);                        // Set text size
    tft.setCursor(70, 65);                     // Set cursor position
    tft.print("Count: ");                      // Display label
    tft.print(counter);                        // Display counter value
}
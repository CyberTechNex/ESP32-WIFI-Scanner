#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCROLL_BUTTON_PIN 23 // Pin D23 for scrolling pages
#define RESCAN_BUTTON_PIN 15 // Pin D15 for triggering rescan
#define BUTTON_DEBOUNCE_DELAY 200 // Adjust this value as needed
#define COUNTDOWN_DURATION 5000 // Countdown duration in milliseconds

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int currentNetwork = 0;
int numNetworks = 0;
int currentPage = 1; // Initialize the page number to 1
unsigned long lastButtonPressTime = 0;
bool rescanRequested = false;
unsigned long rescanStartTime = 0;

void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(SCROLL_BUTTON_PIN, INPUT_PULLUP); // Set the scroll button pin as input with pull-up resistor
  pinMode(RESCAN_BUTTON_PIN, INPUT_PULLUP); // Set the rescan button pin as input with pull-up resistor
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("Display failed to load."));
    for (;;);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();
  displayIntroScreen(); // Display intro screen on startup
  scanNetworks(); // Initial WiFi network scan
}

void loop() {
  if (millis() - lastButtonPressTime > BUTTON_DEBOUNCE_DELAY) {
    if (digitalRead(SCROLL_BUTTON_PIN) == LOW) { // Check if the scroll button is pressed
      lastButtonPressTime = millis(); // Record the time of button press
      currentNetwork = (currentNetwork + 1) % numNetworks; // Move to the next network
      currentPage = (currentPage % numNetworks) + 1; // Update the page number
    }

    if (digitalRead(RESCAN_BUTTON_PIN) == LOW && !rescanRequested) { // Check if the rescan button is pressed and a rescan is not already requested
      rescanRequested = true; // Set the flag to indicate rescan request
      rescanStartTime = millis(); // Record the start time of rescan
    }
  }

  if (rescanRequested) {
    unsigned long elapsedTime = millis() - rescanStartTime;
    if (elapsedTime < COUNTDOWN_DURATION) {
      // Display countdown during the first COUNTDOWN_DURATION milliseconds
      displayCountdown();
    } else {
      // Rescan networks after the countdown
      scanNetworks(); // Perform the network rescan
      rescanRequested = false; // Reset the rescan flag
    }
  } else {
    // Display network information if rescan is not requested
    displayNetworkInfo();
  }
}

void displayIntroScreen() {
  display.clearDisplay();
  display.setTextSize(1); // Increased font size
  display.setTextColor(SSD1306_WHITE);

// Position the text
  // Print bold text
  display.setCursor(1, 1);
  display.println("Esp-32 WIFI Scanner");
  display.setCursor(1, 10);
  display.println("");
  display.setCursor(1, 20);
  display.println("By A1D3N");

  // Draw the text again with a slight offset
  display.setCursor(2, 2);
  display.println("Esp-32 WIFI Scanner");
  display.setCursor(2, 11);
  display.println("");
  display.setCursor(2, 21);
  display.println("By A1D3N");

  // Position the ASCII cat
  display.setCursor(0, 36); // Adjust Y position for the cat
  display.println(" /\\_/\\");
  display.println("( o.o )");
  display.println(" > ^ <");
  display.println("  ^ ^  ");


  display.display();
}


void scanNetworks() {
  Serial.println("Scanning networks..."); // Debug message
  numNetworks = WiFi.scanNetworks();
  currentNetwork = 0;
  currentPage = 1;
}

void displayNetworkInfo() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("SSID: ");
  display.println(WiFi.SSID(currentNetwork));
  display.print("MAC:");
  display.println(WiFi.BSSIDstr(currentNetwork));
  display.print("Security: ");
  display.println(getEncryptionTypeText(WiFi.encryptionType(currentNetwork)));
  display.print("Channel: ");
  display.println(WiFi.channel(currentNetwork));
  display.print("RSSI: ");
  display.println(WiFi.RSSI(currentNetwork));
  // Estimate distance based on RSSI
  double distance = estimateDistance(WiFi.RSSI(currentNetwork));
  display.print("Distance: ");
  display.print(distance);
  display.println(" meters");
 // Calculate the width of the page number text
  int pageNumTextWidth = 6 * 2; // Assuming 2 digits for the page number
  // Center-align the page number and shift four characters to the right
  int pageNumX = (SCREEN_WIDTH - pageNumTextWidth) / 2 + 24; // Shift four characters to the right
  display.setCursor(pageNumX, SCREEN_HEIGHT - 8);
  display.print("Page ");
  display.print(currentPage);
  display.display();
}


void displayCountdown() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Scanning networks...");
  int secondsRemaining = (COUNTDOWN_DURATION - (millis() - rescanStartTime)) / 1000;
  display.println("Time remaining: ");
  display.print(secondsRemaining);
  display.println(" seconds");
  display.println("");
  display.setCursor(38, 26); // Adjust Y position for the cat
  display.println(" /\\_/\\");
   display.setCursor(38, 36);
  display.println("( o.o )");
   display.setCursor(38, 46);
  display.println(" > ^ <");
   display.setCursor(38, 56);
  display.println("  ^ ^  ");
  display.display();
}

String getEncryptionTypeText(uint8_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "None";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2 Enterprise";
    default:
      return "Unknown";
  }
}

double estimateDistance(int rssi) {
  // Apply the path-loss model to estimate distance
  const double A = -69; // Reference signal strength in dBm at 1 meter
  const double n = 2.0; // Path-loss exponent, typically varies between 2 and 4
  return pow(10, (A - rssi) / (10 * n));
}

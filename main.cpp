#include "mbed.h"
#include "ESP8266.h"
#include "MFRC522.h"
#include "TextLCD.h"

// Initialize SPI and RFID module
MFRC522 rfid(PA_7, PA_6, PA_5, PA_4, PA_8); // MOSI, MISO, SCK, CS, RST

// Serial monitor for output
Serial pc(USBTX, USBRX, 115200);

// Wi-Fi UART port and baud rate
ESP8266 wifi(PA_9, PA_10, 115200);

// Buzzer setup
DigitalOut buzzer(PC_0); // Initialize PC0 as a digital output for the buzzer

// Initialize LEDs
DigitalOut greenLED(PC_8); // Green LED on when connected to Wi-Fi
DigitalOut redLED(PC_6); // Red LED on when not connected to Wi-Fi

// I2C Communication
I2C i2c_lcd(PB_9, PB_8); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2, TextLCD::HD44780); // I2C bus, PCF8574 address, LCD Type

// Updated Google Apps Script URL - extract only the ID part
const char *scriptId = "AKfycbzQjdMC-xjgmzROoYNPoUZsWxLviQ9n56DaZB0ajN99lCE0_2Drt-peRlaeb35NhiIM0Q";

const char *name;
const char *student_ID;
int knownCard;

char snd[512], rcv[1000];

// Function to display LCD
void displayLCD (const char *line1, const char *line2)
{
    lcd.cls(); // Clear the display
    lcd.locate(0, 0); // Set cursor to row 0, column 0
    lcd.printf(line1);

    lcd.locate(0, 1); // Set cursor to row 1, column 0
    lcd.printf(line2);
    wait_ms (500);
}

// Function to connect to Wi-Fi
void connectWifi (char ssid[], char password[])
{   

    // Set ESP8266 mode to Station
    pc.printf("Setting mode to Station...\r\n");
    wifi.SetMode(1); // Mode 1 = Station mode
    wifi.RcvReply(rcv, sizeof(rcv)); // Fetch response into rcv buffer
    pc.printf("Response: %s\r\n", rcv);

    // Retry mechanism for connecting to Wi-Fi
    while (true) {
        pc.printf("Connecting to Wi-Fi (%s)...\r\n", ssid);
        wifi.Join(ssid, password); // Attempt to join Wi-Fi
        wifi.RcvReply(rcv, sizeof(rcv)); // Fetch response into rcv buffer

        displayLCD ("Connecting...", "");

        wait (3);

        // Check if connection is successful
        if (strstr(rcv, "OK") != NULL || strstr(rcv, "WIFI CONNECTED") != NULL) 
        {
            pc.printf("Wi-Fi Connected: %s\r\n", rcv);
            displayLCD ("Connected!", "");

            //Show Wi-Fi connection status
            redLED = 0; // Red LED off when connected to Wi-Fi
            greenLED = 1; // Green LED on when connected to Wi-Fi

            wait (2);
            break; // Exit loop if connection succeeds
        } 

        else 
        {
            pc.printf("Error connecting to Wi-Fi. Trying again...\r\n");
        }
        wait(5); // Wait 5 seconds before retrying
    }
}

// Function to send data to Google Sheets
void sendToGoogleSheets(int no, const char* studentId, const char* name) 
{
    // Create JSON data
    char jsonData[200];
    sprintf(jsonData, "{\"no\":\"%d\",\"studentId\":\"%s\",\"name\":\"%s\"}", 
            no, studentId, name);
    
    // Enable SSL
    wifi.SendCMD("AT+CIPSSL=1\r\n");
    wifi.RcvReply(rcv, 1000);

    // Start SSL connection to Google Scripts
    wifi.SendCMD("AT+CIPSTART=\"SSL\",\"script.google.com\",443\r\n");
    wifi.RcvReply(rcv, 1000);

    // Wait for connection
    wait(2);
    
    // Prepare HTTP POST request
    sprintf(snd, 
        "POST /macros/s/%s/exec HTTP/1.1\r\n"
        "Host: script.google.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        scriptId,
        strlen(jsonData),
        jsonData);

    // Inform ESP8266 of data length
    sprintf(rcv, "AT+CIPSEND=%d\r\n", strlen(snd));
    wifi.SendCMD(rcv);
    wifi.RcvReply(rcv, 1000);

    // Send HTTP POST request
    wifi.SendCMD(snd);
    wifi.RcvReply(rcv, 2000);

    // Close connection
    wifi.SendCMD("AT+CIPCLOSE\r\n");
    wifi.RcvReply(rcv, 1000);

    pc.printf("Data sent to Google Sheets.\r\n");

    // Add delay before next transmission
    wait(1);
}

// Function to beep once when known card is scanned
void knownCardSound ()
{
    buzzer = 1; // Turn the buzzer ON
    wait_ms(400);    // Wait for 1 second
    buzzer = 0; // Turn the buzzer OFF
}

// Function to beep once when unknown card is scanned
void unknownCardSound ()
{
    buzzer = 1; // Turn the buzzer ON
    wait_ms(300);    // Wait for 1 second
    buzzer = 0; // Turn the buzzer OFF
    wait_ms (200);    // Wait for 1 second
    buzzer = 1; // Turn the buzzer ON
    wait_ms(300);    // Wait for 1 second
    buzzer = 0; // Turn the buzzer OFF  
}


int main() {

    lcd.setMode(TextLCD::DispOn);            // Turn display ON
    lcd.setBacklight(TextLCD::LightOn);      // Turn backlight ON
    lcd.setCursor(TextLCD::CurOff_BlkOff);  // Turn cursor OFF

    //Show Wi-Fi connection status
    redLED = 1; // Red LED on when not connected to Wi-Fi
    greenLED = 0; // Green LED off when not connected to Wi-Fi

    pc.baud(115200);
    pc.printf("Initializing ESP8266...\r\n");
    
    connectWifi ("tharshen", "roticanai"); //Edit wifi name and password here

    pc.printf("Initializing RC522 RFID module...\r\n");

    // Initialize RFID reader
    rfid.PCD_Init();
    pc.printf("RFID module initialized.\r\n");

    int recordNumber = 1;

    while (true) {
    displayLCD ("Tap your card", "");

    redLED = 0;
    greenLED = 1;
    
    // Look for a new card
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) 
    {
        wait_ms(200); // Wait and retry
        continue;
    }

    // Print UID of the card
    pc.printf("\nCard UID: ");
    char card_uid[16] = ""; // Buffer to store UID as a string
    for (uint8_t i = 0; i < rfid.uid.size; i++) 
    {
        pc.printf("%02X", rfid.uid.uidByte[i]);       // Print each byte in HEX
        sprintf(card_uid + (i * 2), "%02X", rfid.uid.uidByte[i]); // Convert UID to string
    }
    pc.printf("\r\n");

    // Check if the UID matches the predefined value
    if (strcmp(card_uid, "4166EE26") == 0) 
    { 
        knownCard = 1;
        name = "Tharshen";
        student_ID = "20001074";   
    } 

    else if (strcmp(card_uid, "E8CFB60D") == 0) 
    { 
        knownCard = 1;
        name = "Jin Hang";
        student_ID = "20001366";   
    } 

    else
    {
        knownCard = 0;
    }

    // Perform when known card is scanned
    if (knownCard == 1)
    {
        
        pc.printf("Name: %s\r\n", name);
        pc.printf("Student ID: %s\r\n", student_ID);

        displayLCD (name, student_ID);
        knownCardSound ();

        redLED = 1;
        greenLED = 0;

        sendToGoogleSheets(recordNumber, student_ID, name);
        recordNumber++;
    }

    // Perform when unknown card is scanned  
    else 
    {
        pc.printf("Unknown Card UID.\r\n");

        displayLCD ("Unknown ID.", "Please retry!");
        unknownCardSound ();

        redLED = 1;
        greenLED = 0;
    }

    // Halt the card to prepare for the next detection
    rfid.PICC_HaltA();

    wait_ms(500); // Wait 500ms before next read
    }
}
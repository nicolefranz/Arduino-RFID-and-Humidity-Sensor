#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define SS_PIN 10 //SDA connected to Arduino pin 10
#define RST_PIN 9 //RST connect to Arduino pin 9
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const int buzzer_pin = 8;
const int red_led = 5;
const int blue_led = 7;
const int green_led = 6;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

int acc = 2;
String accounts[] = {" 69 B8 2B 16", " 2A C3 65 81"};
String names[] = {"Alvin", "Nicole"};
String logged = "";
String name = "";
bool isLogin = false;
int count = 0;

void setup() {
  Serial.begin(9600); // Initiate a serial communication
  SPI.begin(); // Initiate SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(buzzer_pin, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  lcd.print("Tap your card.");
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) { //Returns 1 if that card's UID can be read
    return;
  }
  
  if (isLogin) {
    if (logged.equals(getRfidUID(mfrc522.uid.size))) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank you!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Logging out...");
      isLogin = false;
      setColor(0, 0, 0);
      delay(5000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tap your card.");
      return;
    } 
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error: Another");
      lcd.setCursor(0, 1);
      lcd.print("Acc online");
      setColor(0, 255, 0); // Set LED color to red
      tone(buzzer_pin, 1000); // Set higher pitch
      delay(2000);
      noTone(buzzer_pin); // Stop the tone
    }
  }
  validate(getRfidUID(mfrc522.uid.size));
  mfrc522.PICC_HaltA(); // This call must be made to prevent repeated re-detection of our card
}

// Extract UID from mfrc522 class variable
String getRfidUID(byte bufferSize) {
  String sUID = "";
  for (byte i = 0; i < bufferSize; i++) {
    sUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); // add zero if hexadecimal is single digit
    sUID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  sUID.toUpperCase();
  return sUID;
}

void setColor(int redValue, int blueValue, int greenValue)
{
  analogWrite(red_led, redValue);
  analogWrite(blue_led, blueValue);
  analogWrite(green_led, greenValue);
}

void validate(String uid) {
  if (!isLogin) {
    for (int i = 0; i < acc; i++) {
      if (uid.equals(accounts[i])) {
        logged = accounts[i];
        name = names[i];
        isLogin = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Welcome ");
        lcd.setCursor(0, 1);
        lcd.print(name);
        setColor(0, 0, 255); // Set LED color to red
        delay(1000);
        tone(buzzer_pin, 1000); // Set higher pitch
        delay(1000);
        noTone(buzzer_pin); // Stop the tone
        delay(1000);
        heatIndex();
        return; // ID found and validated
      }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid unique ID!");
    setColor(255, 0, 0); // Set LED color to red
    tone(buzzer_pin, 1000); // Set higher pitch
    delay(2000);
    setColor(0, 0, 0);
    noTone(buzzer_pin); // Stop the tone
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tap your card.");
  }
}

void heatIndex(){
  float h = dht.readHumidity();
  float f = dht.readTemperature();
  float t = dht.readTemperature(true);

  if(isnan(h) || isnan(t) || isnan(f)){
    Serial.print("Failed");
    return;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hum:  ");
  lcd.print(h);
  lcd.print(" %");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(f);
  lcd.print(" C");
  delay(3000);
  lcd.clear();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(f);
  Serial.print(" C");
  Serial.println();
  if(f>0 && f<=26){
    lcd.setCursor(0, 0);
    lcd.print("HEAT INDEX:");
    lcd.setCursor(0, 1);
    lcd.print("NORMAL");
    setColor(0, 255, 255); 
  }
  else if(f>27 && f<=32){
    lcd.setCursor(0, 0);
    lcd.print("HEAT INDEX:");
    lcd.setCursor(0, 1);
    lcd.print("CAUTION");
    setColor(255, 255, 0);
  }
  else if(f>32 && f<=41){
    lcd.setCursor(0, 0);
    lcd.print("HEAT INDEX:");
    lcd.setCursor(0, 1);
    lcd.print("EXTR. CAUTION");
    setColor(255, 0, 255);
  }
  else if(f>41 && f<=51){
    lcd.setCursor(0, 0);
    lcd.print("HEAT INDEX:");
    lcd.setCursor(0, 1);
    lcd.print("DANGER");
    setColor(255, 0, 0); // Set LED color to red
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print("HEAT INDEX:");
    lcd.setCursor(0, 1);
    lcd.print("EXTR. DANGER!");
    setColor(255, 0, 0); // Set LED color to red
    tone(buzzer_pin, 1000); // Set higher pitch
    delay(2000);
    noTone(buzzer_pin); // Stop the tone
    setColor(255, 0, 0); // Set LED color to red
  }
   // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    delay(2000);
    heatIndex();
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) { //Returns 1 if that card's UID can be read
    return;
  }
  
  if (isLogin) {
    if (logged.equals(getRfidUID(mfrc522.uid.size))) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank you!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Logging out...");
      isLogin = false;
      setColor(0, 0, 0);
      delay(5000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tap your card.");
      return;
    } 
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error: Another");
      lcd.setCursor(0, 1);
      lcd.print("Acc online");
      setColor(0, 255, 0); // Set LED color to red
      tone(buzzer_pin, 1000); // Set higher pitch
      delay(2000);
      noTone(buzzer_pin); // Stop the tone
      delay(1000);
      heatIndex();
    }
  }
}
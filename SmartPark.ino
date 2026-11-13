#include <NewPing.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// -------- LCD --------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// -------- SETTINGS --------
#define DISTANCE_LIMIT 4
#define MAX_DISTANCE 200

// -------- BUZZER --------
#define BUZZER 2

// -------- SERVO & GATE IR --------
#define SERVO_PIN 9
#define GATE_IR   6
Servo gateServo;

// -------- RFID --------
#define SS_PIN   53
#define RST_PIN  5
#define RFID_SERVO_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN);
Servo rfidServo;

// -------- ULTRASONIC --------
NewPing sonar1(22, 23, MAX_DISTANCE);
NewPing sonar2(24, 25, MAX_DISTANCE);
NewPing sonar3(26, 27, MAX_DISTANCE);
NewPing sonar4(28, 29, MAX_DISTANCE);

// -------- IR SENSORS --------
// A1 A2 A3 A4 B1 B2 B3 B4
int irPins[8]    = {30, 31, 32, 33, 34, 35, 36, 37};
int greenLEDs[8] = {38, 40, 42, 44, 46, 48, 11, 3};
int redLEDs[8]   = {39, 41, 43, 45, 47, 49, 10, 4};

void setup() {
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // -------- IR + LEDs --------
  for (int i = 0; i < 8; i++) {
    pinMode(irPins[i], INPUT);
    pinMode(greenLEDs[i], OUTPUT);
    pinMode(redLEDs[i], OUTPUT);

    digitalWrite(greenLEDs[i], HIGH);
    digitalWrite(redLEDs[i], LOW);
  }

  // -------- SERVO (IR GATE) --------
  pinMode(GATE_IR, INPUT);
  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  // -------- RFID --------
  SPI.begin();
  rfid.PCD_Init();

  rfidServo.attach(RFID_SERVO_PIN);
  rfidServo.write(0);

  // -------- LCD --------
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.begin(9600);
  Serial.println("Smart Parking System with RFID Started");
}

void loop() {
  // -------- ULTRASONIC READINGS --------
  int d1 = sonar1.ping_cm();
  int d2 = sonar2.ping_cm();
  int d3 = sonar3.ping_cm();
  int d4 = sonar4.ping_cm();

  // -------- SERIAL DISPLAY (1–4 CM ONLY) --------
  Serial.print("Distances (cm): ");
Serial.println(d1);
 Serial.println(d2);
 Serial.println(d3);
 Serial.println(d4);


  // -------- BUZZER LOGIC --------
  if ((d1 > 0 && d1 < 7) ||
      (d2 > 0 && d2 < 7) ||
      (d3 > 0 && d3 < 7) ||
      (d4 > 0 && d4 < 7)) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  // -------- IR + LED STATUS --------
  for (int i = 0; i < 8; i++) {
    if (digitalRead(irPins[i]) == LOW) {
      digitalWrite(redLEDs[i], HIGH);
      digitalWrite(greenLEDs[i], LOW);
    } else {
      digitalWrite(redLEDs[i], LOW);
      digitalWrite(greenLEDs[i], HIGH);
    }
  }

  // -------- LCD UPDATE --------
  lcd.setCursor(0, 0);
  lcd.print("A1 A2 A3 A4     ");

  lcd.setCursor(0, 1);
  lcd.print(digitalRead(irPins[0]) == LOW ? "X " : "O ");
  lcd.print(digitalRead(irPins[1]) == LOW ? " X " : " O ");
  lcd.print(digitalRead(irPins[2]) == LOW ? " X " : " O ");
  lcd.print(digitalRead(irPins[3]) == LOW ? " X" : " O");

  lcd.setCursor(0, 2);
  lcd.print("B1 B2 B3 B4     ");

  lcd.setCursor(0, 3);
  lcd.print(digitalRead(irPins[7]) == LOW ? "X " : "O ");
  lcd.print(digitalRead(irPins[6]) == LOW ? " X " : " O ");
  lcd.print(digitalRead(irPins[5]) == LOW ? " X " : " O ");
  lcd.print(digitalRead(irPins[4]) == LOW ? " X" : " O");

  // -------- IR GATE SERVO --------
  if (digitalRead(GATE_IR) == LOW) {
    gateServo.write(90);
    delay(2000);
    gateServo.write(0);
    delay(1000);
  }

  // -------- RFID SERVO --------
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    Serial.print("RFID UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    rfidServo.write(90);
    delay(2000);
    rfidServo.write(0);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  delay(200);
}

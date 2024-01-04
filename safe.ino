#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define Password_Length 8 //constant
char Data[Password_Length]; //password input
char Master[Password_Length] = "123A456"; //actual password

Servo myServo; //pin connected to door lock input

byte data_count = 0; // counter for character entries

char customKey; //character to hold key input

//constants
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12,11,9,8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7,6,5,4}; //connect to the column pinouts of the keypad

//initialize an object of Keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool doorOpen = false;  // Variable to track the state of the door

void setup() {
  Serial.begin(9600);
  myServo.attach(10);
  myServo.write(170);
  lcd.backlight(); //Setup LCD
  lcd.init();
  Serial.println("fingertest");

  // pinMode(myServo, OUTPUT); //set lockOutput as an OUTPUT

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for a valid finger...");
}

void loop() {
  // Check if the door is open and wait for 'C' key to close it
  if (doorOpen) {
    // lcd.clear();
    // lcd.print("Close the Safe");
    customKey = customKeypad.getKey();

    if (customKey == 'C') {
      lcd.clear();
      lcd.print("Closing Door");
      myServo.write(170);
      doorOpen = false;  // Set the door state to closed
      delay(3000);
      lcd.clear();
    }
  } else {
    //initialize LCD
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");

    //Look for keypress
    customKey = customKeypad.getKey();

    if (customKey) {
      //Enter keypress into array and increment counter
      Data[data_count] = customKey; //Data array
      lcd.setCursor(data_count, 1);
      lcd.print(Data[data_count]);
      data_count++;
    }

    //see if we have reached the password length
    if (data_count == Password_Length - 1) {
      lcd.clear();

      // compare password
      if (!strcmp(Data, Master)) {
        // Password is correct
        lcd.print("Give Fingerprint");
        delay(3000);

        int attempts = 0;
        bool fingerprintMatched = false;

        while (attempts < 3 && !fingerprintMatched) {
          fingerprintMatched = getFingerprintID();

          if (!fingerprintMatched) {
            lcd.clear();
            lcd.print("Fingerprint Failed");
            delay(1000);
          }

          attempts++;
        }

        if (fingerprintMatched) {
          lcd.clear();
          lcd.print("It is a Match");
          delay(1000);
          lcd.clear();
          lcd.print("Press C to Close");
          myServo.write(80);
          doorOpen = true;  // Set the door state to open
          delay(3000);
          
        } else {
          lcd.clear();
          lcd.print("Access Denied");
          delay(1000);
        }
      } else {
        // password is incorrect
        lcd.print("Incorrect");
        delay(1000);
      }

      // clear data and LCD display
      lcd.clear();
      clearData();
    }
  }
}

void clearData() {
  //clear array
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
  return;
}

bool getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }

  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    // myServo.write(90);
    // lcd.print("Matched");
    // delay(3000);
    // myServo.write(4);
    // Serial.print("Matched");
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return false;
  } else {
    Serial.println("Unknown error");
    return false;
  }
}

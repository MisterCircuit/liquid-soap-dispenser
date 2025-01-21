#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define keypad configuration
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {5, 4, 3, 10}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 LCD

// Water flow sensor variables
volatile unsigned long pulse_count = 0; // Pulse count from the flow sensor
float total_volume = 0;                // Total volume in liters
float target_volume = 0;         // User-defined volume target

// Relay setup
const int relay_pin = 11;

void setup() {
  pinMode(2, INPUT_PULLUP);      // YF-S201 sensor input pin
  pinMode(relay_pin, OUTPUT);    // Relay control pin
  digitalWrite(relay_pin, LOW);  // Turn off the pump initially

  // Initialize serial communication and LCD
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Litres:");
  
  // Attach interrupt for the flow sensor
  attachInterrupt(digitalPinToInterrupt(2), pulse, RISING);
}

void loop() {
  // User input handling
  char key = keypad.getKey();
  if (key) { // If a key is pressed
    static String input = ""; // To store the user's input
    if (key == '#') { // Confirm the input
      target_volume = input.toInt(); // Convert input to an integer
//      target_volume=target_volume*0.07;
      input = ""; // Clear the input string

      if (target_volume>0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Target: ");
        lcd.print(target_volume);
        lcd.print("L");
        delay(2000);
        
        startPumping(); // Start the pumping process
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid Input");
        delay(2000);
        lcd.clear();
        lcd.print("Enter Litres:");
      }
    } else if (key == '*') { // Cancel the input
      input = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Litres:");
    } else { // Append the digit
      input += key;
      lcd.setCursor(0, 1);
      lcd.print(input);
    }
  }
}

void startPumping() {
  target_volume=target_volume*0.07;  //for panel A
//  target_volume=target_volume*0.017; //for panel B
  total_volume = 0; // Reset total volume
  pulse_count = 0;  // Reset pulse count
  digitalWrite(relay_pin, HIGH); // Turn on the pump
  Serial.println("Pump started.");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing...");
  delay(500);
  const double correction_factor = 1.07; // Adjust for 10% error margin  
  
  while (total_volume < target_volume) {
    // Calculate total volume
    total_volume = 0.00225 * pulse_count* correction_factor;; // Convert pulse count to liters
        // Debugging log
    Serial.print("Pulses: ");
    Serial.print(pulse_count);
    Serial.print(", Dispensed: ");
    Serial.print(total_volume, 3);
    Serial.println(" L");

    // Update LCD with current volume
    lcd.setCursor(0, 1);
    lcd.print("Dispensed: ");
    lcd.print(total_volume, 2); // Show 2 decimal places
    lcd.print("L");
  }

  // Pumping complete
  digitalWrite(relay_pin, LOW); // Turn off the pump
   Serial.println("Target volume reached. Pump stopped.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensed ");
  lcd.print(target_volume);
  lcd.print("L");
  lcd.setCursor(0, 1);
  lcd.print("Complete!");
  delay(5000);

  // Reset to initial state
  lcd.clear();
  lcd.print("Enter Litres:");
}

void pulse() {
  pulse_count++; // Increment pulse count on each pulse
}

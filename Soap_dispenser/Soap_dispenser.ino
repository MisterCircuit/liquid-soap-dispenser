#include <Keypad.h>                // Library for interacting with a keypad
#include <Wire.h>                  // Library for I2C communication (used by the LCD)
#include <LiquidCrystal_I2C.h>     // Library for controlling an I2C LCD

// Define keypad configuration
const byte ROWS = 4; // Keypad has four rows
const byte COLS = 4; // Keypad has four columns
// Mapping the keys on the keypad to their respective characters
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Pin connections for the keypad rows and columns
byte rowPins[ROWS] = {5, 4, 3, 10}; 
byte colPins[COLS] = {9, 8, 7, 6};

// Create a keypad object for managing key inputs
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD setup: address 0x27, 16 characters, and 2 lines
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Variables for the water flow sensor
volatile unsigned long pulse_count = 0; // Counts pulses from the flow sensor
float total_volume = 0;                // Tracks the total volume dispensed
float target_volume = 0;               // The volume the user wants to dispense

// Relay pin to control the pump
const int relay_pin = 11;

void setup() {
  pinMode(2, INPUT_PULLUP);      // Set pin 2 as input for the flow sensor
  pinMode(relay_pin, OUTPUT);    // Set the relay pin as output
  digitalWrite(relay_pin, LOW);  // Ensure the pump is off initially

  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();               // Turn on the LCD backlight
  lcd.setCursor(0, 0);
  lcd.print("Enter Litres:");    // Prompt the user to input a target volume

  // Attach an interrupt to count pulses from the flow sensor
  attachInterrupt(digitalPinToInterrupt(2), pulse, RISING);
}

void loop() {
  // Check if a key on the keypad is pressed
  char key = keypad.getKey();
  if (key) { // If a key is pressed
    static String input = ""; // Temporarily store the user's input
    if (key == '#') { // '#' acts as the confirmation key
      target_volume = input.toInt(); // Convert the input string to an integer
      input = ""; // Clear the input for the next entry

      if (target_volume > 0) { // Validate the input
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Target: ");
        lcd.print(target_volume); // Display the target volume on the LCD
        lcd.print("L");
        delay(2000);
        
        startPumping(); // Begin the pumping process
      } else {
        // Handle invalid input
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid Input");
        delay(2000);
        lcd.clear();
        lcd.print("Enter Litres:");
      }
    } else if (key == '*') { // '*' acts as the cancel key
      input = ""; // Clear the input
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Litres:");
    } else { // For numeric keys, append the character to the input
      input += key;
      lcd.setCursor(0, 1);
      lcd.print(input); // Display the input on the LCD
    }
  }
}

void startPumping() {
  // Scaling factors for calibration (specific to your setup)
  target_volume = target_volume * 0.07; // Adjust target volume for panel A
  // target_volume = target_volume * 0.017; // Adjust target volume for panel B (if needed)
  
  total_volume = 0; // Reset the dispensed volume
  pulse_count = 0;  // Reset the pulse count
  digitalWrite(relay_pin, HIGH); // Turn on the pump
  Serial.println("Pump started."); // Debug message

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensing...");
  delay(500);

  const double correction_factor = 1.07;  // Adjust for a 7% error margin
  const double output_factor = 14.29;     // Calibration factor for display accuracy

  while (total_volume < target_volume) {
    // Calculate the total volume dispensed
    total_volume = 0.00225 * pulse_count * correction_factor;

    // Print debugging information to the serial monitor
    Serial.print("Pulses: ");
    Serial.print(pulse_count);
    Serial.print(", Dispensed: ");
    Serial.print(total_volume * output_factor, 3);
    Serial.println(" L");

    // Update the LCD with the dispensed volume
    lcd.setCursor(0, 1);
    lcd.print("Dispensed: ");
    lcd.print(total_volume * output_factor, 2); // Show 2 decimal places
    lcd.print("L");
  }

  // Pumping process complete
  digitalWrite(relay_pin, LOW); // Turn off the pump
  Serial.println("Target volume reached. Pump stopped."); // Debug message

  // Display completion message on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensed ");
  lcd.print(target_volume);
  lcd.print("L");
  lcd.setCursor(0, 1);
  lcd.print("Complete!");
  delay(5000);

  // Reset LCD to initial state
  lcd.clear();
  lcd.print("Enter Litres:");
}

void pulse() {
  pulse_count++; // Increment the pulse count for each pulse detected
}

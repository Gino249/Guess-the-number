#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_DRV2605.h>


// Forward declarations
void showLines(const char *line1, const char *line2);
void showGuessPrompt();
void vibrateMiss();
void showResults();
void startNewGame();
void acceptEntry();
void handleKey(char key);

constexpr byte LCD_ADDRESS = 0x27;
constexpr byte LCD_COLUMNS = 16;
constexpr byte LCD_ROWS = 2;
constexpr byte KEYPAD_ROWS = 4;
constexpr byte KEYPAD_COLS = 4;
constexpr byte GUESS_COUNT = 3;
constexpr int MIN_NUMBER = 1;
constexpr int MAX_NUMBER = 100;
constexpr unsigned long RESULT_VIEW_MS = 1800;
constexpr unsigned long VIBRATION_MS = 900;

const byte rowPins[KEYPAD_ROWS] = {2, 3, 4, 5};
const byte colPins[KEYPAD_COLS] = {6, 7, 8, 9};
char keyMap[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
Adafruit_DRV2605 drv;

int guesses[GUESS_COUNT] = {0, 0, 0};
byte guessIndex = 0;
int entryValue = 0;
byte entryDigits = 0;
bool hapticReady = false;

void showLines(const char *line1, const char *line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void showGuessPrompt() {
  char prompt[17];
  char valueLine[17];
  snprintf(prompt, sizeof(prompt), "Guess number %u:", guessIndex + 1);
  if (entryDigits == 0) {
    snprintf(valueLine, sizeof(valueLine), "1-100  #=Enter");
  } else {
    snprintf(valueLine, sizeof(valueLine), "%d #Enter *=Clr", entryValue);
  }
  showLines(prompt, valueLine);
}

void vibrateMiss() {
  if (!hapticReady) return;
  drv.setWaveform(0, 47);
  drv.setWaveform(1, 0);
  drv.go();
  delay(VIBRATION_MS);
  drv.stop();
}

const char *comparisonFor(int guess, int target) {
  if (guess < target) return "LOW";
  if (guess > target) return "HIGH";
  return "MATCH";
}

void showResults() {
  const int target = random(MIN_NUMBER, MAX_NUMBER + 1);
  const bool matched = guesses[0] == target || guesses[1] == target || guesses[2] == target;
  char line1[17];
  char line2[17];

  snprintf(line1, sizeof(line1), "Number: %d", target);
  snprintf(line2, sizeof(line2), "G1:%s G2:%s", comparisonFor(guesses[0], target), comparisonFor(guesses[1], target));
  showLines(line1, line2);
  delay(RESULT_VIEW_MS);

  snprintf(line1, sizeof(line1), "Number: %d", target);
  snprintf(line2, sizeof(line2), "G3: %s", comparisonFor(guesses[2], target));
  showLines(line1, line2);
  delay(RESULT_VIEW_MS);

  if (matched) {
    showLines("You matched it!", "New game soon...");
    delay(RESULT_VIEW_MS);
  } else {
    showLines("No match!", "Vibration ON");
    vibrateMiss();
    delay(RESULT_VIEW_MS);
  }
}

void startNewGame() {
  guessIndex = 0;
  entryValue = 0;
  entryDigits = 0;
  showGuessPrompt();
}

void acceptEntry() {
  if (entryDigits == 0 || entryValue < MIN_NUMBER || entryValue > MAX_NUMBER) {
    showLines("Enter 1 to 100", "Digits then #");
    delay(1200);
    showGuessPrompt();
    return;
  }

  guesses[guessIndex++] = entryValue;
  entryValue = 0;
  entryDigits = 0;
  if (guessIndex < GUESS_COUNT) {
    showGuessPrompt();
  } else {
    showResults();
    startNewGame();
  }
}

void handleKey(char key) {
  if (key >= '0' && key <= '9') {
    if (entryDigits < 3) {
      int proposed = entryValue * 10 + (key - '0');
      if (proposed <= MAX_NUMBER) {
        entryValue = proposed;
        entryDigits++;
      }
    }
    showGuessPrompt();
  } else if (key == '*') {
    entryValue = 0;
    entryDigits = 0;
    showGuessPrompt();
  } else if (key == '#') {
    acceptEntry();
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  showLines("Guess the Number", "Starting...");
  Wire.begin();
  hapticReady = drv.begin();
  if (hapticReady) {
    drv.selectLibrary(6);
    drv.setMode(DRV2605_MODE_INTTRIG);
  }
  randomSeed(analogRead(A0) ^ micros());
  delay(1000);
  startNewGame();
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) handleKey(key);
}

#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

// ──────────────────────────────────────────
//            تعريف الأطراف
// ──────────────────────────────────────────
#define SERVO_PIN     9
#define LED_RED_PIN   8    // LED أحمر ← خطأ / إنذار
#define LED_GREEN_PIN 13   // LED أخضر ← دخول صحيح
#define LOCKOUT_SEC   10

// ──────────────────────────────────────────
//            إعداد الـ LCD
//            RS, EN, D4, D5, D6, D7
// ──────────────────────────────────────────
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ──────────────────────────────────────────
//            إعداد لوحة المفاتيح 4×3
// ──────────────────────────────────────────
const byte ROWS = 4;
const byte COLS = 3;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {6,  7,  10};

Keypad keypad = Keypad(
  makeKeymap(hexaKeys),
  rowPins, colPins,
  ROWS, COLS
);

// ──────────────────────────────────────────
//            إعداد السيرفو
// ──────────────────────────────────────────
Servo lockServo;

// ──────────────────────────────────────────
//            ثوابت النظام
// ──────────────────────────────────────────
const String STUDENT_NAME = "Ahmed Osrof";
const String STUDENT_ID   = "120212439";
const String CORRECT_PASS = "1234";
const int    MAX_ATTEMPTS  = 3;

// ──────────────────────────────────────────
//            متغيرات الحالة
// ──────────────────────────────────────────
int    attemptCount = 0;
bool   systemLocked = false;
String enteredPass  = "";


void setup() {
  pinMode(LED_RED_PIN,   OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN,   LOW);
  digitalWrite(LED_GREEN_PIN, LOW);

  lockServo.attach(SERVO_PIN);
  lockDoor();

  lcd.begin(16, 2);
  showWelcomeScreen();
  delay(3000);
  showEnterPassword();
}


void loop() {
  if (systemLocked) {
    runLockout();
    return;
  }

  char key = keypad.getKey();
  if (!key) return;

  if      (key == '#') checkPassword();
  else if (key == '*') clearEntry();
  else                 appendKey(key);
}


//         دوال الشاشة (LCD)


void showWelcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(STUDENT_NAME);
  lcd.setCursor(0, 1);
  lcd.print(STUDENT_ID);
}

void showEnterPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  enteredPass = "";
}

void updatePasswordDisplay() {
  lcd.setCursor(0, 1);
  for (int i = 0; i < (int)enteredPass.length(); i++)
    lcd.print("*");
  for (int i = enteredPass.length(); i < 16; i++)
    lcd.print(" ");
}

void showAccessGranted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" ACCESS GRANTED ");
  lcd.setCursor(0, 1);
  lcd.print("  Door Opened!");
}

void showAccessDenied(int remaining) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" ACCESS DENIED! ");
  lcd.setCursor(0, 1);
  if (remaining > 0) {
    lcd.print("Tries left: ");
    lcd.print(remaining);
    lcd.print("   ");
  } else {
    lcd.print("SYSTEM LOCKED!  ");
  }
}

void showLockoutScreen(int secondsLeft) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!!! ALARM  !!!");
  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(secondsLeft);
  lcd.print(" sec    ");
}

//         دوال التحكم (سيرفو + LED)

void unlockDoor() { lockServo.write(90); }
void lockDoor()   { lockServo.write(0);  }

void allLedsOff() {
  digitalWrite(LED_RED_PIN,   LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
}

//         دوال كلمة المرور

void appendKey(char key) {
  if (enteredPass.length() < 16) {
    enteredPass += key;
    updatePasswordDisplay();
  }
}

void clearEntry() {
  enteredPass = "";
  showEnterPassword();
}

void checkPassword() {
  if (enteredPass == CORRECT_PASS)
    handleCorrectPassword();
  else
    handleWrongPassword();
}

//     معالجة كلمة المرور الصحيحة 
void handleCorrectPassword() {
  attemptCount = 0;
  allLedsOff();

  digitalWrite(LED_GREEN_PIN, HIGH);  //  إضاءة أخضر
  showAccessGranted();
  unlockDoor();
  delay(5000);                         // الباب مفتوح 5 ثوان

  digitalWrite(LED_GREEN_PIN, LOW);   // إطفاء أخضر
  lockDoor();
  showEnterPassword();
}

//     معالجة كلمة المرور الخاطئة 

void handleWrongPassword() {
  attemptCount++;
  int remaining = MAX_ATTEMPTS - attemptCount;

  showAccessDenied(remaining);
  delay(2000);

  if (attemptCount >= MAX_ATTEMPTS)
    systemLocked = true;
  else
    showEnterPassword();
}

//     دالة الإغلاق      

void runLockout() {
  for (int s = LOCKOUT_SEC; s > 0; s--) {
    showLockoutScreen(s);
    digitalWrite(LED_RED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_RED_PIN, LOW);
    delay(500);
  }

  systemLocked = false;
  attemptCount = 0;
  allLedsOff();
  showEnterPassword();
}

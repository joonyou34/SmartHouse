#include <Servo.h>
#include <Keypad.h>

const byte trig = 3, echo = 4, servo_pin = 5;
const byte buzzer = 2;
const byte rows = 4, cols = 3;
const byte ldr = A0, leds = 6;
byte rowPins[rows] = {13, 12, 11, 10},
     colPins[cols] = {9, 8, 7};
const float timeToDist = 1 / 58.f;

const char hexa[rows][cols] = { { '1', '2', '3'},
                          		{ '4', '5', '6'},
                          		{ '7', '8', '9'},
                          		{ '*', '0', '#'} };

Keypad pad(makeKeymap(hexa), rowPins, colPins, rows, cols);
Servo servo;


const byte minLen = 4, maxLen = 16;
const float detectionRange = 25.f;

String password = "0000";
String buffer = "";

bool open = false;
int trialCtr = 0;
byte buzzct = 0;
unsigned long lastBuzz = 0;
unsigned long buzzTime = 0;
bool buzzState = 0;
unsigned long buzzDelay = 0, buzzLength = 0;

bool frozen = 0;

float sensorValue = 0.f;

long freezeStart = 0;

bool isNum(char c) {
  return (c >= '0' && c <= '9');
}

//-1 : IGNORED
// 0 : num
// 1 : accepted
// 2 : change password
// 3 : clear
// 4 : delete last character
// 5 : password hint
int state(char c) {
  if (isNum(c))
    return 0;
  if (c == '*')
    return 5;
  if (c == '#')
    return 2;

  return -1;
}

void buzz(int ct = 1, long bdelay = 250, long length = 15) {
  buzzct = ct;
  digitalWrite(buzzer, HIGH);
  buzzTime = millis();
  buzzDelay = bdelay;
  buzzLength = length;
}

void pop_back(String& s) {
  if(s.length() == 0) return;
  s.remove(s.length() - 1);
}

void clearRecieved() {
  while(Serial.available() > 0)
    Serial.read();
}

void sendSignal() {
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
}

float getDist() {
  float time = pulseIn(echo, HIGH);
  return time * timeToDist;
}

bool detectNear() {
  sendSignal();

  return getDist() <= detectionRange;
}

void hint() {
  Serial.print(password[0]);
  for (int i = 1; i < password.length(); i++)
    Serial.print('-');
  Serial.print('\n');
}

void addChar(char c) {
  if(buffer.length() == maxLen)
    return;
  buzz();
  buffer += c;
  Serial.println(buffer);
}

bool checkPassword(String pw) {
  return (pw == password);
}

void checkBuzz() {
  if(buzzct > 0) {
    if(buzzState) {
      if(millis() - buzzTime >= buzzLength) {
        buzzState = 0;
        digitalWrite(buzzer, 0);
        lastBuzz = millis();
        buzzct--;
      }
    }
    else if(millis() - lastBuzz >= buzzDelay) {
      buzzState = 1;
      digitalWrite(buzzer, 1);
      buzzTime = millis();
    }

  }
}

void accepted(String pw) {
  if(checkPassword(pw)) {
    buzz(1, 0, 50);
    servo.write(90);
    open = true;
    trialCtr = 0;
  }
  else {
    trialCtr++;
    buzz(3, 250, 50);
    if(trialCtr == 3) {
      trialCtr = 0;
      Serial.println("Too Many Tries");
      frozen = 1;
      freezeStart = millis();
    }
  }
  buffer = "";
}


// * = confirm password
// # = go back
//TODO: Edit to make use of the state function and add delete and clear functionalities with 4x4 keypad
void changePw() {
  if(!open) return;
  Serial.println("Enter New PW:");
  String newPw = "";
  char inp = '\0';
  while(inp != '#') {
    inp = pad.waitForKey();
    if(inp == '*' && newPw.length() >= minLen) {
      Serial.flush();
      password = newPw;
      Serial.println("PASSWORD CHANGED");
      break;
    }
    else if(isNum(inp) && newPw.length() != maxLen) {
      Serial.flush();
      newPw += inp;
      Serial.println(newPw);
    }
  }
}

void clr() {
  buffer = "";
}

void deleteLast() {
  pop_back(buffer);
}

int isLit()
{
  sensorValue = analogRead(ldr);
  int ret = map(sensorValue, 0, 1023, 0, 255);

  return (ret > 15 ? 0 : 255 - ret);
}

void lighting()
{
  int calc = isLit();
  
  analogWrite(leds , calc);
}

void keypadPasswordSystem() {
  char inp = pad.getKey();
    if(inp != NO_KEY) {
      Serial.flush();
      
      // * = hint
      // # = change password
      switch(state(inp)) {
        case 0:
        	addChar(inp);
        	if(buffer.length() == password.length())
               accepted(buffer);
          break;
        case 1:
          accepted(buffer);
          break;
        case 2:
          clr();
          changePw();
          break;
        case 3:
          clr();
          break;
        case 4:
          deleteLast();
          break;
        case 5:
          hint();
          break;
      }
    }
}

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(leds, OUTPUT);
  pinMode(ldr, INPUT);

  servo.attach(servo_pin);
  servo.write(0);

  Serial.begin(115200);
}

void loop() {
  
  checkBuzz();
  lighting();

  if(!frozen) {
    if (detectNear()) {
      if(buzzct == 0) {
        keypadPasswordSystem();
        if(Serial.available()) {
          String enteredPw = Serial.readString();
          accepted(enteredPw);
          Serial.println(enteredPw);
          clearRecieved();
        }
      }
    }
    else {
      clearRecieved();
      if (buffer.length() > 0)
        clr();
      if(open) {
        servo.write(0);
        open = false;
      }
    }
  }
  else if(millis() - freezeStart >= 60000){
    frozen = 0;
    clearRecieved();
    Serial.flush();
  }


  delay(15);
}
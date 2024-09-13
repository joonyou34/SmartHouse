#include <Servo.h>
#include <Keypad.h>

const byte trig = 7, echo = 6, servo_pin = 5;
const byte buzzer = 8;
const byte rows = 4, cols = 4;
byte rowPins[rows] = {A0, A1, A2, A3},
     colPins[cols] = {A4, A5, 2, 3};
const float timeToDist = 1 / 58.f;

const byte dongBuzzer = 9;
bool dongButtonPressed = false;

const char hexa[rows][cols] =
{
  { '1', '2', '3', 'A'},
  { '4', '5', '6', 'B'},
  { '7', '8', '9', 'C'},
  { '*', '0', '#', 'D'}
};

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

bool changingPw = false;
String newPw = "";

bool pwRecieved = false;
String recievedPw = "";

const int dingDongNotes[2] = {932, 740};
bool secondDong = false;
unsigned long lastDong = 0;
const unsigned long dingDelay = 200;
const unsigned long dongDelay = 400;

void pop_back(String& s) {
  if(s.length() == 0) return;
  s.remove(s.length() - 1);
}


void clr(String& buffer) {
  buffer = "";
  Serial.flush();
  Serial.println(buffer);
}

void deleteLast(String& buffer) {
  pop_back(buffer);
  Serial.flush();
  Serial.println(buffer);
}

void handleDingDong() {
  if(dongButtonPressed) {
    dongButtonPressed = false;
    secondDong = true;
    tone(dongBuzzer, dingDongNotes[0]);
    lastDong = millis();
  }

  if(secondDong && millis() - lastDong >= dongDelay) {
    secondDong = false;
    lastDong = millis();
    tone(dongBuzzer, dingDongNotes[1]);
  }

  if(!secondDong && millis() - lastDong >= dongDelay)
    noTone(dongBuzzer);
}

bool isNum(char c) {
  return (c >= '0' && c <= '9');
}

//-1 : IGNORED
// 0 : num
// 1 : accepted / confirm changed pw
// 2 : change password / go_back
// 3 : clear
// 4 : delete last character
// 5 : password hint
int state(char c) {
  if (isNum(c))
    return 0;
  if (c == '*') {
    dongButtonPressed = true;
    return -1;
  }
  if (c == '#')
    return 5;

  return c - 'A' + 1;
}

void buzz(int ct = 1, long bdelay = 250, long length = 30) {
  buzzct = ct;
  digitalWrite(buzzer, HIGH);
  buzzTime = millis();
  buzzDelay = bdelay;
  buzzLength = length;
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
  float time = pulseIn(echo, HIGH, 100000);
  return time * timeToDist;
}

bool detectNear() {
  sendSignal();
  int dst = getDist();
  return (dst <= detectionRange && dst != 0);
}

void hint() {
  Serial.print(password[0]);
  for (int i = 1; i < password.length(); i++)
    Serial.print('-');
  Serial.print('\n');
}

void addChar(char c, String& buffer) {
  if(buffer.length() == maxLen)
    return;
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

void accepted(String& pw) {
  if(checkPassword(pw)) {
    buzz(1, 0, 75);
    servo.write(90);
    open = true;
    trialCtr = 0;
    Serial.println("/V1");
  }
  else {
    trialCtr++;
    Serial.println("/V0");
    buzz(3, 250, 75);
    if(trialCtr == 3) {
      trialCtr = 0;
      Serial.println("Too Many Tries");
      Serial.println("/E1Someone tried to rob you (probably Samir) >:(");
      frozen = 1;
      freezeStart = millis();
    }
  }
  buffer = "";
}




// A = confirm password
// B = go back
//TODO: Edit to make use of the state function and add delete and clear functionalities with 4x4 keypad
void changePw() {
  if(!open) return;
  Serial.println("Enter New PW:");
  newPw = "";
  changingPw = true;
}

void changePwHelper() {
  if(!changingPw) return;

  char inp = pad.getKey();
  if(inp == NO_KEY) return;

  buzz();
  switch(state(inp)) {
    case 0:
      addChar(inp, newPw);
      break;
    case 1:
      if(newPw.length() >= minLen) {
        Serial.flush();
        password = newPw;
        Serial.println("PASSWORD CHANGED");
        Serial.println("/E0Someone changed your password, if it wasn't you it's probably Samir uwu");

        changingPw = false;
        return;
      }
      break;

    case 2:
      changingPw = false;
      return;

    case 3:
      clr(newPw);
      break;
    
    case 4:
      deleteLast(newPw);
      break;
  }
}



void keypadPasswordSystem() {
  char inp = pad.getKey();
    if(inp != NO_KEY) {
      int st = state(inp);
      Serial.flush();
      if(~st)
        buzz();
      
      // # = hint
      // B = change password
      if(st != 2 && open) return;
      switch(st) {
        case 0:
        	addChar(inp, buffer);
          break;
        case 1:
          accepted(buffer);
          break;
        case 2:
          changePw();
          clr(buffer);
          break;
        case 3:
          clr(buffer);
          break;
        case 4:
          deleteLast(buffer);
          break;
        case 5:
          hint();
          break;
      }
    }
}
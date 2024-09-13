#include "GameEngine.h"
#include "PasswordSystem.h"


const byte ledMatrixPin = 10;
Window gameWindow(8, 8, ledMatrixPin);
unsigned long prevGameTime = 0;

bool gameEngineOn = false;

bool directInputRecieved = false;
byte inputPlayer = 0;
bool directInputType = 0; //0 is release, 1 is click
char inputButton = '\0';

void serialHandler() {
  if(Serial.available()) {
    String command =  Serial.readStringUntil('\n');
    pop_back(command);
    if(command.length() < 2) return;
    if(command[0] == '/') {
      switch(command[1]) {
        case 'P': {
          if(command.length() < 3)
            return;
          
          pwRecieved = true;
          recievedPw = command.substring(2);
          break;
        }

        case '$': {
          gameEngineOn = true;
          break;
        }

        case 'D': {
          if(command.length() < 5) return;
          directInputRecieved = true;
          inputPlayer = command[2]-'0';
          inputButton = command[3];
          directInputType = command[4]-'0';
          break;
        }
          
      }
    }
  }
}


void passwordSystemHandler() {
  if(!frozen) {
    if (detectNear()) {
      if(buzzct == 0) {
        if(!changingPw)
          keypadPasswordSystem();
        if(pwRecieved) {
          accepted(recievedPw);
          Serial.println(recievedPw);
          pwRecieved = false;
          recievedPw = "";
        }
        changePwHelper();
      }
    }
    else {
      changingPw = false;
      recievedPw = "";
      pwRecieved = false;
      if (buffer.length() > 0)
        clr(buffer);
      if(open) {
        servo.write(180);
        open = false;
      }
    }
  }
  else if(millis() - freezeStart >= 60000){
    frozen = 0;
    pwRecieved = false;
    recievedPw = "";
    changingPw = "";
    Serial.flush();
  }
}

Box<float> ground(8, -6, 9, 16), leftBound(0, -6, 9, -5), rightBound(0, 15, 9, 16);

class Player {
  private:


  bool canJump = false;

  const float moveSpeed = 10.f;
  const float jumpVelocity = 35.f;

  void gravity() {
    ChineseOBJ.vx += g*dt;
  }

    Color ChineseElMlwn[3][3];
    bool ChineseElAlphy[3][3];
    Box<float> ChineseElBoxy;
    Sprite ChineseElSpritey;
    gameObject ChineseOBJ;

  public:
    Player()
        : ChineseElMlwn{
            {{255,0,255}, {255,0,0}, {255,0,255}},
            {{255,0,255}, {255,0,255}, {255,0,255}},
            {{255,0,255}, {255,0,255}, {255,0,255}}
          },
          ChineseElAlphy{
            {0, 1, 0},
            {1, 1, 1},
            {1, 0, 1}
          },
          ChineseElBoxy(0, 0, 2, 2),
          ChineseElSpritey(ChineseElMlwn, ChineseElAlphy),
          ChineseOBJ(ChineseElBoxy, ChineseElSpritey, 0, 1)
    {}

  bool collision(Box<float> hitbox) {
    bool ret = false;
    ChineseOBJ.y += ChineseOBJ.vy*dt;
    if(ChineseOBJ.intersects(hitbox)) {
      ChineseOBJ.y -= ChineseOBJ.vy*dt;
      ChineseOBJ.vy = 0;
      ret = true;
    }
    ChineseOBJ.y -= ChineseOBJ.vy*dt;

    ChineseOBJ.x += ChineseOBJ.vx*dt;
    if(ChineseOBJ.intersects(hitbox)) {
      ChineseOBJ.x -= ChineseOBJ.vx*dt;
      ChineseOBJ.vx = 0;
      ret = true;
    }
    ChineseOBJ.x -= ChineseOBJ.vx*dt;

    return ret;
  }

  void render() {
    ChineseOBJ.render(gameWindow);
  }
  const byte JUMP = 13, LEFT = 13, RIGHT = 13; //REMOVE THIS

  void update() {
    gravity();

    canJump = collision(ground);
    if(canJump && digitalRead(JUMP))
      ChineseOBJ.vx = -jumpVelocity;

    ChineseOBJ.vy = 0;

    if(digitalRead(LEFT))
      ChineseOBJ.vy -= moveSpeed;
    if(digitalRead(RIGHT))
      ChineseOBJ.vy += moveSpeed;

    collision(leftBound);
    collision(rightBound);
    ChineseOBJ.update();
  }


} Chinese;

void gameSystem() {
  if(millis() - prevGameTime >= SPF*1000) {
    gameWindow.clear();
    calcDT();

    //game loop here
    Chinese.update(); //ADD ME
    Chinese.render();

    gameWindow.render();
    prevGameTime = millis();
  }
}

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(dongBuzzer, OUTPUT);

  servo.attach(servo_pin);
  servo.write(180);

  gameScreen.clear();
  gameScreen.render();


















  Serial.begin(115200);
}

void loop() {
  serialHandler();
  checkBuzz();
  handleDingDong();

  passwordSystemHandler();

  if(gameEngineOn)
    gameSystem();
  
  
}
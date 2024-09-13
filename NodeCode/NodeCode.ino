#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EMailSender.h>
#include <LiquidCrystal.h>
#include "SoundSystem.h"

#define senderEmail "chineseelnoody@gmail.com"
#define senderPw "mxcohvnzmectlwoh"
#define recieverEmail "yaheahaha@gmail.com"
#define mailServer "smtp.gmail.com"
#define mailPort 465

const char* wifi_ssid = "Redmi Note 11";
const char* wifi_password = "123456789";

const char* ssid = "ChineseElNodeyy";
const char* APpassword = "chinesesnode";

bool pwCommandState = false;
bool pwSuccess = false;


IPAddress staticIP(192, 168, 1, 69), gateway(192,168,1,1), subnet(255,255,255,0);

String htmlPage;

EMailSender sender(senderEmail, senderPw, recieverEmail, mailServer, mailPort);



ESP8266WebServer server(80);

const byte WiFiPin = D8;

const String subjects[] = {"Password Changed", "Too Many Unsuccessful Attempts"};

const byte ldr = A0, leds = D1;

int lightState = 2;

const byte lcdPins[6] = {D7,D6,D5,D4,D3,D2};

LiquidCrystal lcd(lcdPins[0], lcdPins[1], lcdPins[2], lcdPins[3], lcdPins[4], lcdPins[5]);

void pop_back(String& s) {
  if(s.length() == 0) return;
  s.remove(s.length() - 1);
}

void writeOnLCD(String message) {
  lcd.clear();
  lcd.print(message);
}

void sendEmail(String subject, String message) {
  EMailSender::EMailMessage msg;
  msg.subject = subject;
  msg.message = message;
  EMailSender::Response response = sender.send(recieverEmail, msg);
  Serial.println(response.desc);
}

void serialHandler() {
  if(Serial.available()) {
    String command =  Serial.readStringUntil('\n');
    pop_back(command);
    if(command.length() < 2) return;
    if(command[0] == '/') {
      switch(command[1]) {
        case 'E': {
          if(command.length() < 4)
            return;
          byte subject = command[2] - '0';
          
          sendEmail(subjects[subject], command.substring(3));
          break;
        }

        case 'S': {
          if(command.length() < 5) return;

          onloop = command[4]-'0';
          songIdx = (command[2]-'0')*10 + (command[3]-'0');
          playingMusic = true;
          Serial.println(songIdx);
          break;
        }

        case 'V': {
          if(command.length() < 3) return;
          pwSuccess = command[2]-'0';
          pwCommandState = true;
          break;
        }

        case 'L': {
          if(command.length() < 3) return;
          writeOnLCD(command.substring(2));
          break;
        }
          
        default:
          Serial.readStringUntil('\n');
      }
    }
  }
}

// void initializePage() {
//   htmlPage = "<!DOCTYPE html>\n";
//     htmlPage += "<html lang=\"en\">\n";
//     htmlPage += "<head>\n";
//     htmlPage += "    <meta charset=\"UTF-8\">\n";
//     htmlPage += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
//     htmlPage += "    <title>Password Submission</title>\n";
//     htmlPage += "    <style>\n";
//     htmlPage += "        body, html {\n";
//     htmlPage += "            height: 100%;\n";
//     htmlPage += "            margin: 0;\n";
//     htmlPage += "            display: flex;\n";
//     htmlPage += "            justify-content: center;\n";
//     htmlPage += "            align-items: center;\n";
//     htmlPage += "            background-color: #f0f0f0;\n";
//     htmlPage += "            font-family: Arial, sans-serif;\n";
//     htmlPage += "        }\n";
//     htmlPage += "        .container {\n";
//     htmlPage += "            text-align: center;\n";
//     htmlPage += "        }\n";
//     htmlPage += "        input[type=\"password\"] {\n";
//     htmlPage += "            padding: 10px;\n";
//     htmlPage += "            font-size: 16px;\n";
//     htmlPage += "            border: 1px solid #ccc;\n";
//     htmlPage += "            border-radius: 5px;\n";
//     htmlPage += "            width: 300px;\n";
//     htmlPage += "            box-sizing: border-box;\n";
//     htmlPage += "            margin-bottom: 20px;\n";
//     htmlPage += "        }\n";
//     htmlPage += "        button {\n";
//     htmlPage += "            padding: 10px 20px;\n";
//     htmlPage += "            font-size: 16px;\n";
//     htmlPage += "            background-color: #007BFF;\n";
//     htmlPage += "            color: white;\n";
//     htmlPage += "            border: none;\n";
//     htmlPage += "            border-radius: 5px;\n";
//     htmlPage += "            cursor: pointer;\n";
//     htmlPage += "        }\n";
//     htmlPage += "        button:hover {\n";
//     htmlPage += "            background-color: #0056b3;\n";
//     htmlPage += "        }\n";
//     htmlPage += "    </style>\n";
//     htmlPage += "</head>\n";
//     htmlPage += "<body>\n";
//     htmlPage += "    <div class=\"container\">\n";
//     htmlPage += "        <form id=\"passwordForm\">\n";
//     htmlPage += "            <input type=\"password\" name=\"password\" id=\"password\" placeholder=\"Enter your password\" required>\n";
//     htmlPage += "            <br>\n";
//     htmlPage += "            <button type=\"submit\">Submit</button>\n";
//     htmlPage += "        </form>\n";
//     htmlPage += "    </div>\n";
//     htmlPage += "    <script>\n";
//     htmlPage += "        document.getElementById('passwordForm').addEventListener('submit', function(event) {\n";
//     htmlPage += "            event.preventDefault();\n";
//     htmlPage += "            const password = document.getElementById('password').value;\n";
//     htmlPage += "            fetch('http://192.168.1.69/submit-password', {\n";
//     htmlPage += "                method: 'POST',\n";
//     htmlPage += "                headers: {\n";
//     htmlPage += "                    'Content-Type': 'text/plain'\n";
//     htmlPage += "                },\n";
//     htmlPage += "                body: password\n";
//     htmlPage += "            })\n";
//     htmlPage += "            .then(response => response.text())\n";
//     htmlPage += "            .then(data => {\n";
//     htmlPage += "                document.getElementById('password').value = '';\n";
//     htmlPage += "            })\n";
//     htmlPage += "            .catch(error => console.error('Error:', error));\n";
//     htmlPage += "        });\n";
//     htmlPage += "    </script>\n";
//     htmlPage += "</body>\n";
//     htmlPage += "</html>";
// }

int isLit()
{
  int sensorValue = analogRead(ldr);
  int ret = map(sensorValue, 0, 1023, 0, 255);
  // Serial.println(ret);

  return (ret > 15 ? 0 : 255 - ret);
}

void lighting()
{
  int calc = 0;
  if(lightState == 1) {
    calc = 255;
  }
  else if(lightState == 2)
    calc = isLit();
  // analogWrite(leds , calc);
}

void startHotspot() {
  WiFi.softAPConfig(staticIP, gateway, subnet);
  WiFi.softAP(ssid, APpassword);
}

void startWiFi() {
  digitalWrite(WiFiPin, LOW);

  WiFi.begin(wifi_ssid, wifi_password);
  while(WiFi.status() != WL_CONNECTED) {

    delay(500);
  }
  
  digitalWrite(WiFiPin, HIGH);
  Serial.println("Connected");
}

void handleRoot() {
  server.send(200, "text/plain", "Welcome");
}

void setup() {
  Serial.begin(115200);

  pinMode(ldr, INPUT);

  pinMode(WiFiPin, OUTPUT);

  pinMode(buzzer, OUTPUT);

  pinMode(leds, OUTPUT);
  shenanigans();
  
  // initializePage();

  startWiFi();
  startHotspot();

  //HTTPS requests
  server.on("/", handleRoot);
  server.on("/submit_password", HTTP_POST, handlePasswordSubmit);
  server.on("/lightState", HTTP_POST, handleLightState); 
  server.on("/nextTrack", HTTP_POST, handleNextTrack);
  server.on("/gameEngine", HTTP_POST, handleGameEngine);
  server.on("/prevState", HTTP_POST, handlePrevState);
  server.on("/musicState", HTTP_POST, handleMusicState); 
  server.on("/startEngine", HTTP_POST, handleStartEngine); 

  server.begin();
}

void handleLightState() {
  lightState++;
  lightState%=3;
  server.send(200, "text/plain", String((char)('0'+lightState)));
}
void handleNextTrack() {
  songIdx++;
  songIdx%=songsNum;
  server.send(200);
}
void handleGameEngine() {
  if (server.hasArg("player") && server.hasArg("Button") && server.hasArg("State")) {
    String player = server.arg("player");
    String button = server.arg("Button");
    String state = server.arg("state");

    Serial.print("/D");
    Serial.print(player);
    Serial.print(button);
    Serial.println(state);

    server.send(200);
  }
  else {
    server.send(400, "text/plain", "No Input");
  }
}
void handlePrevState() {
  server.send(200, "text/plain", String((char)('0'+lightState)) + String((char)('0'+playingMusic)));
}
void handleMusicState() {
  playingMusic = !playingMusic;
  server.send(200, "text/plain", String((char)('0'+playingMusic)));
}

void handleStartEngine() {
  Serial.println("/$");
  server.send(200);
}

long ctr = 0;

void loop() {
  server.handleClient();
  serialHandler();
  lighting();
  // Serial.println("meow");

  if(playingMusic) {
    playMusic();
  }
  else
    noTone(buzzer);
}

void handlePasswordSubmit() {
  if (server.hasArg("password")) {
    String pw = server.arg("password");

    Serial.print("/P");
    Serial.println(pw);

    while(!pwCommandState) {
      serialHandler();
      yield();
    }
    pwCommandState = false;
    String response = pwSuccess ? "1" : "0";

    server.send(200, "text/plain", response);
  }
  else {
    server.send(400, "text/plain", "No password received");
  }
}

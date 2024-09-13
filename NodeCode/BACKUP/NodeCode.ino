#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ChineseElNodey";
const char* password = "samirbig";

IPAddress staticIP(192, 168, 1, 69), gateway(192,168,1,1), subnet(255,255,255,0);

String htmlPage;



ESP8266WebServer server(80);

void initializePage() {
  htmlPage = "<!DOCTYPE html>\n";
    htmlPage += "<html lang=\"en\">\n";
    htmlPage += "<head>\n";
    htmlPage += "    <meta charset=\"UTF-8\">\n";
    htmlPage += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    htmlPage += "    <title>Password Submission</title>\n";
    htmlPage += "    <style>\n";
    htmlPage += "        body, html {\n";
    htmlPage += "            height: 100%;\n";
    htmlPage += "            margin: 0;\n";
    htmlPage += "            display: flex;\n";
    htmlPage += "            justify-content: center;\n";
    htmlPage += "            align-items: center;\n";
    htmlPage += "            background-color: #f0f0f0;\n";
    htmlPage += "            font-family: Arial, sans-serif;\n";
    htmlPage += "        }\n";
    htmlPage += "        .container {\n";
    htmlPage += "            text-align: center;\n";
    htmlPage += "        }\n";
    htmlPage += "        input[type=\"password\"] {\n";
    htmlPage += "            padding: 10px;\n";
    htmlPage += "            font-size: 16px;\n";
    htmlPage += "            border: 1px solid #ccc;\n";
    htmlPage += "            border-radius: 5px;\n";
    htmlPage += "            width: 300px;\n";
    htmlPage += "            box-sizing: border-box;\n";
    htmlPage += "            margin-bottom: 20px;\n";
    htmlPage += "        }\n";
    htmlPage += "        button {\n";
    htmlPage += "            padding: 10px 20px;\n";
    htmlPage += "            font-size: 16px;\n";
    htmlPage += "            background-color: #007BFF;\n";
    htmlPage += "            color: white;\n";
    htmlPage += "            border: none;\n";
    htmlPage += "            border-radius: 5px;\n";
    htmlPage += "            cursor: pointer;\n";
    htmlPage += "        }\n";
    htmlPage += "        button:hover {\n";
    htmlPage += "            background-color: #0056b3;\n";
    htmlPage += "        }\n";
    htmlPage += "    </style>\n";
    htmlPage += "</head>\n";
    htmlPage += "<body>\n";
    htmlPage += "    <div class=\"container\">\n";
    htmlPage += "        <form id=\"passwordForm\">\n";
    htmlPage += "            <input type=\"password\" name=\"password\" id=\"password\" placeholder=\"Enter your password\" required>\n";
    htmlPage += "            <br>\n";
    htmlPage += "            <button type=\"submit\">Submit</button>\n";
    htmlPage += "        </form>\n";
    htmlPage += "    </div>\n";
    htmlPage += "    <script>\n";
    htmlPage += "        document.getElementById('passwordForm').addEventListener('submit', function(event) {\n";
    htmlPage += "            event.preventDefault();\n";
    htmlPage += "            const password = document.getElementById('password').value;\n";
    htmlPage += "            fetch('http://192.168.1.69/submit-password', {\n";
    htmlPage += "                method: 'POST',\n";
    htmlPage += "                headers: {\n";
    htmlPage += "                    'Content-Type': 'text/plain'\n";
    htmlPage += "                },\n";
    htmlPage += "                body: password\n";
    htmlPage += "            })\n";
    htmlPage += "            .then(response => response.text())\n";
    htmlPage += "            .then(data => {\n";
    htmlPage += "                document.getElementById('password').value = '';\n";
    htmlPage += "            })\n";
    htmlPage += "            .catch(error => console.error('Error:', error));\n";
    htmlPage += "        });\n";
    htmlPage += "    </script>\n";
    htmlPage += "</body>\n";
    htmlPage += "</html>";
}

void startHotspot() {
  WiFi.softAPConfig(staticIP, gateway, subnet);
  WiFi.softAP(ssid, password);
}

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void setup() {
  Serial.begin(115200);
  
  initializePage();
  startHotspot();

  //HTTPS requests
  server.on("/", handleRoot);
  server.on("/submit-password", HTTP_POST, handlePasswordSubmit);

  server.begin();
}

void loop() {
  server.handleClient();
}

void handlePasswordSubmit() {
  if (server.hasArg("plain")) {
    String pw = server.arg("plain");

    Serial.print(pw);

    server.send(200, "text/plain", "Password received");
  }
  else {
    server.send(400, "text/plain", "No password received");
  }
}

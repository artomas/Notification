#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "TEST"; 
const char* password = "12345678"; 

ESP8266WebServer server(80);

int currentAnim = 1;
int currentBrightness = 3;
int currentSpeed = 1;

// HTML страница
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Управление</title>
</head>
<body>
  <h2>Управление LED матрицей и звуком</h2>
  <form action="/set" method="POST">
    <button type="submit" name="anim" value="1">Градиент</button>
    <button type="submit" name="anim" value="2">Синий</button>
    <button type="submit" name="anim" value="3">Красный</button><br><br>
    
    Яркость: <input type="number" name="brightness" min="0" max="255" value="10"><br><br>
    Скорость: <input type="number" name="speed" min="1" max="20" value="1"><br><br>

    <label for="track">Номер трека:</label>
    <input type="number" id="track" name="track" min="1" max="999" value="1"><br><br>
    <button type="submit" formaction="/order" name="track" value="1">Запустить трек</button>
  </form>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSet() {
  if (server.hasArg("anim")) {
    int anim = server.arg("anim").toInt();
    Serial.println("ANIM:" + String(anim));
    server.send(200, "text/plain", "Анимация: " + String(anim));
  } 
  else if (server.hasArg("brightness")) {
    int brightness = server.arg("brightness").toInt();
    Serial.println("BRIGHTNESS:" + String(brightness));
    server.send(200, "text/plain", "Яркость: " + String(brightness));
  } 
  else if (server.hasArg("speed")) {
    int speed = server.arg("speed").toInt();
    Serial.println("SPEED:" + String(speed));
    server.send(200, "text/plain", "Скорость: " + String(speed));
  } 
  else {
    server.send(400, "text/plain", "Неизвестная команда");
  }
}

void handleOrder() {
  if (server.hasArg("track")) {
    int track = server.arg("track").toInt();
    Serial.println("TRACK:" + String(track));
    server.send(200, "text/plain", "Трек: " + String(track));
  } else {
    server.send(400, "text/plain", "Не указан номер трека");
  }
}

void handleStatus() {
  DynamicJsonDocument doc(256);
  doc["anim"] = currentAnim;
  doc["brightness"] = currentBrightness;
  doc["speed"] = currentSpeed;
  
  String json;
  serializeJson(doc, json);
  
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/order", HTTP_POST, handleOrder);
  server.begin();
}

void loop() {
  server.handleClient();
}

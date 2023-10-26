#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

ESP8266WebServer server(80);

const int ledPin = D6;
DHT dht14(D4, DHT11);
const char* ssid = "ffx";
const char* password = "thanaporn";

void init_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  Serial.begin(115200);
  init_wifi();

  pinMode(ledPin, OUTPUT);

  dht14.begin();

  server.on("/", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();

    String html = "<html><head>";
    html += "<style>";
    html += "body { font-family: Arial, Helvetica, sans-serif; background-color: #f2f2f2; }";
    html += "div { background-color: #fff; border-radius: 5px; padding: 20px; margin: 20px; text-align: center; }";
    html += "h1 { color: #333; font-size: 36px; text-decoration: underline; }";
    html += "p { color: #666; font-size: 24px; font-weight: bold; }";
    html += "input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; width: 150px; margin: 10px auto; }"; // เพิ่มความกว้างและทำให้ปุ่มอยู่กลางจอ
    html += "input[type='submit']:hover { background-color: #45a049; }";
    html += "#turnOffButton { background-color: black; }";
    html += "</style>";
    html += "</head>";
    html += "<body><div>";
    html += "<h1>Web Server</h1>";
    html += "<p>Temperature: <span id='temperature'>" + String(temperature, 2) + " Celsius</span></p>";
    html += "<p>Humidity: <span id='humidity'>" + String(humidity, 2) + " percent</span></p>";
    html += "<p>LED Status: ";
    if (digitalRead(ledPin) == HIGH) {
      html += "<span style='color: green;'>On</span>";
    } else {
      html += "<span style='color: red;'>Off</span>";
    }
    html += "</p>";
    html += "<form action='/TurnOn' method='get'><input type='submit' value='Turn On'></form>";
    html += "<form action='/TurnOff' method='get'><input type='submit' id='turnOffButton' value='Turn Off'></form>";
    html += "</div></body></html>";

    html += "<script>";
    html += "function updateData() {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', '/data', true);";
    html += "  xhr.onreadystatechange = function() {";
    html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "      var data = JSON.parse(xhr.responseText);";
    html += "      document.getElementById('temperature').textContent = data.temperature.toFixed(2) + ' Celsius';";
    html += "      document.getElementById('humidity').textContent = data.humidity.toFixed(2) + ' percent';";
    html += "    }";
    html += "  };";
    html += "  xhr.send();";
    html += "}";
    html += "setInterval(updateData, 500);";
    html += "</script>";

    server.send(200, "text/html", html);
  });

  server.on("/TurnOn", HTTP_GET, []() {
    digitalWrite(ledPin, HIGH);
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/TurnOff", HTTP_GET, []() {
    digitalWrite(ledPin, LOW);
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/data", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();

    String data = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
    server.send(200, "application/json", data);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

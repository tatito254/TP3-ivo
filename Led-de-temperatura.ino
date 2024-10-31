#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

#define DHTPIN 4         
#define DHTTYPE DHT11    

#define LED_VERDE_1 34
#define LED_VERDE_2 33
#define LED_AMARILLO 26
#define LED_ROJO_1 14
#define LED_ROJO_2 13

#define BOTON_INCREMENTAR 23
#define BOTON_DISMINUIR 22

DHT dht(DHTPIN, DHTTYPE);

int umbralVerde1 = 5;
int umbralVerde2 = 10;
int umbralAmarillo = 15;
int umbralRojo1 = 20;
int umbralRojo2 = 25;

const char* ssid = "IoTB";          
const char* password = "inventaronelVAR";  

WebServer server(80);

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

void setup() {
    Serial.begin(115200);
    dht.begin();

    pinMode(LED_VERDE_1, OUTPUT);
    pinMode(LED_VERDE_2, OUTPUT);
    pinMode(LED_AMARILLO, OUTPUT);
    pinMode(LED_ROJO_1, OUTPUT);
    pinMode(LED_ROJO_2, OUTPUT);

    pinMode(BOTON_INCREMENTAR, INPUT_PULLUP);
    pinMode(BOTON_DISMINUIR, INPUT_PULLUP);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

void loop() {
    server.handleClient();

    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
        controlarLeds(temperature);
    }

    unsigned long currentMillis = millis();
    if (digitalRead(BOTON_DISMINUIR) == LOW && (currentMillis - lastDebounceTime > debounceDelay)) {
        disminuirUmbrales();
        lastDebounceTime = currentMillis;
    }
    if (digitalRead(BOTON_INCREMENTAR) == LOW && (currentMillis - lastDebounceTime > debounceDelay)) {
        incrementarUmbrales();
        lastDebounceTime = currentMillis;
    }
}

void handleRoot() {
    String html = "<html><head><title>Monitor de Temperatura</title>";
    html += "<style>body { font-family: Arial; text-align: center; background-color: #f2f2f2; }";
    html += ".container { max-width: 600px; margin: auto; padding: 20px; border: 1px solid #ddd; background-color: white; }";
    html += ".status { font-size: 1.2em; margin: 10px 0; }";
    html += "</style></head><body><div class='container'><h1>Monitor de Temperatura</h1>";
    html += "<p class='status'>Temperatura actual: <span id='temperature'>Cargando...</span> °C</p>";
    html += "<p>Umbral Verde 1: <span id='umbralVerde1'>" + String(umbralVerde1) + "</span> °C</p>";
    html += "<p>Umbral Verde 2: <span id='umbralVerde2'>" + String(umbralVerde2) + "</span> °C</p>";
    html += "<p>Umbral Amarillo: <span id='umbralAmarillo'>" + String(umbralAmarillo) + "</span> °C</p>";
    html += "<p>Umbral Rojo 1: <span id='umbralRojo1'>" + String(umbralRojo1) + "</span> °C</p>";
    html += "<p>Umbral Rojo 2: <span id='umbralRojo2'>" + String(umbralRojo2) + "</span> °C</p>";
    html += "<h2>Estado de los LEDs:</h2>";
    html += "<p>LED Verde 1: <span id='ledVerde1'>0</span></p>";
    html += "<p>LED Verde 2: <span id='ledVerde2'>0</span></p>";
    html += "<p>LED Amarillo: <span id='ledAmarillo'>0</span></p>";
    html += "<p>LED Rojo 1: <span id='ledRojo1'>0</span></p>";
    html += "<p>LED Rojo 2: <span id='ledRojo2'>0</span></p>";
    html += "</div>";
    
    // JavaScript para actualización automática
    html += "<script>setInterval(async function() {";
    html += "let response = await fetch('/data');";
    html += "let data = await response.json();";
    html += "document.getElementById('temperature').innerText = data.temperature;";
    html += "document.getElementById('ledVerde1').innerText = data.ledVerde1;";
    html += "document.getElementById('ledVerde2').innerText = data.ledVerde2;";
    html += "document.getElementById('ledAmarillo').innerText = data.ledAmarillo;";
    html += "document.getElementById('ledRojo1').innerText = data.ledRojo1;";
    html += "document.getElementById('ledRojo2').innerText = data.ledRojo2;";
    html += "}, 5000);</script></body></html>";

    server.send(200, "text/html", html);
}

void handleData() {
    float temperature = dht.readTemperature();

    String data = "{";
    data += "\"temperature\":" + String(temperature) + ",";
    data += "\"ledVerde1\":" + String(digitalRead(LED_VERDE_1)) + ",";
    data += "\"ledVerde2\":" + String(digitalRead(LED_VERDE_2)) + ",";
    data += "\"ledAmarillo\":" + String(digitalRead(LED_AMARILLO)) + ",";
    data += "\"ledRojo1\":" + String(digitalRead(LED_ROJO_1)) + ",";
    data += "\"ledRojo2\":" + String(digitalRead(LED_ROJO_2));
    data += "}";

    server.send(200, "application/json", data);
}

void controlarLeds(float temperature) {
    apagarLeds();
    if (temperature >= umbralVerde1) digitalWrite(LED_VERDE_1, HIGH);
    if (temperature >= umbralVerde2) digitalWrite(LED_VERDE_2, HIGH);
    if (temperature >= umbralAmarillo) digitalWrite(LED_AMARILLO, HIGH);
    if (temperature >= umbralRojo1) digitalWrite(LED_ROJO_1, HIGH);
    if (temperature >= umbralRojo2) digitalWrite(LED_ROJO_2, HIGH);
}

void apagarLeds() {
    digitalWrite(LED_VERDE_1, LOW);
    digitalWrite(LED_VERDE_2, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_ROJO_1, LOW);
    digitalWrite(LED_ROJO_2, LOW);
}

void incrementarUmbrales() {
    umbralVerde1 += 5;
    umbralVerde2 += 5;
    umbralAmarillo += 5;
    umbralRojo1 += 5;
    umbralRojo2 += 5;
    Serial.println("Umbrales incrementados en 5°C");
}

void disminuirUmbrales() {
    umbralVerde1 = max(0, umbralVerde1 - 5);
    umbralVerde2 = max(0, umbralVerde2 - 5);
    umbralAmarillo = max(0, umbralAmarillo - 5);
    umbralRojo1 = max(0, umbralRojo1 - 5);
    umbralRojo2 = max(0, umbralRojo2 - 5);
    Serial.println("Umbrales disminuidos en 5°C");
}
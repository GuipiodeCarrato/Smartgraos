
// Adiciona as bibliotecas utilizadas
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "LiquidCrystal.h"

// Define os pinos usados no sensor e no display
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(4,0,14,12,13,15);

// Cria as variáveis globais
const char* ssid = "Nome da rede";
const char* password = "Senha da rede";

// Broker utilizado, mas existem outros também gratuitos
const char* mqtt_server = "broker.mqtt-dashboard.com";
String inString = "";
char mqtt_t[20];
char mqtt_u[20];

WiFiClient espClient;
PubSubClient client(espClient);

// Método responsável pela conexão da do hardware à rede
void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  inString="";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inString+=(char)payload[i];
  }
  Serial.println();
}

// Método responsável por reconectar ao client MQTT
void reconnect() {

  while (!client.connected()) {
    
    Serial.print("Tentando conexão MQTT...");
    // Cria um clientID aleatório
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Tenta conexão
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado ao MQTT");
      // Ao conectar, publica no tópico, que deve ser substituido pelo nome do seu projeto e suas medições...
      client.publish("SmartGraos/conectado", "ESP Conectado");
      client.subscribe("SmartGraos/conectado");

    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println("Tentando novamente em 3 segundos...");
      // Espera 3 segundos para tentar novamente
      delay(3000);
    }
  }
}

void setup() {

  // Aqui são chamados os métodos que iniciam e/ou executam cada função
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  lcd.begin(16, 2);
  // Configura o server e a porta utilizada
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

// Método responsável pela execução contínua no hardware
void loop() {

  // váriáveis usadas para medição
  float t = 0.0;
  float u = 0.0;

  // verifica a conexão e reconecta se necessário
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // A partir daqui começam as medições, apresentando no display e publicando nos tópicos

  // Limpa o display
  lcd.clear();

  // Mede a temperatura
  t = dht.readTemperature();

  // Valida se a medição retornou algum valor
  if (!isnan(t)) {
    
    Serial.println("temperatura: ");
    Serial.println(t);

    // Transforma float para char e atribui a mqtt_t
    dtostrf(t,4,1,mqtt_t);
    
    // Apresenta no display
    lcd.printf("Temp: ");
    lcd.printf(mqtt_t);
    lcd.write(B11011111);
    lcd.printf("C");

  }
  else {
    
    // Se a medição falhar, apresenta a mensagem:
    Serial.println("Falha no sensor!");
    lcd.printf("Falha no sensor!");

    // Transforma float para char e atribui a mqtt_t
    t = 0.0;
    dtostrf(t,4,1,mqtt_t);
     
  }

  // Muda de linha no display
  lcd.setCursor(0,1);

  // Mede a umidade
  u = dht.readHumidity();

  // Valida se a medição retornou algum valor
  if (!isnan(u)) {
    
    Serial.println("umidade: ");
    Serial.println(u);

    // Transforma float para char e atribui a mqtt_u
    dtostrf(u,3,0,mqtt_u);

   // Apresenta no display 
    lcd.printf("umidade: ");
    lcd.printf(mqtt_u);
    lcd.write(B00100101);

  }
  else {

    // Se a medição falhar, apresenta a mensagem:
    Serial.println("Falha no sensor!");
    lcd.printf("Falha no sensor!");

    // Transforma float para char e atribui a mqtt_t
    u = 0.0;
    dtostrf(u,4,1,mqtt_u);

  }
  
  // Publica as medições para cada tópico, que deve ser substituido pelo nome do seu projeto e suas medições...
  client.publish("SmartGraos/temperatura", mqtt_t);
  client.publish("SmartGraos/umidade", mqtt_u);
    
  delay(3000);
}

// Incluimos librería e instanciamos el objeto servo
#include <ESP32Servo.h>
Servo myservo;

// Librerias para Telegram
#include <UniversalTelegramBot.h>

// Librerias para AWS
#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>  

//Credenciales de red Wifi
const char* ssid = "iPhone de Rodrigo";//modificar
const char* password = "12345678";//modificar 

//Token de Telegram BOT se obtenienen desde Botfather en telegram
#define token_Bot "7229764680:AAFY4sXh7aCCdf0Q2wmPE7Ti_fBaM8JFIIE"
// El ID se obtiene de (IDBot) en telegram no olvidar hacer click en iniciar
#define ID_Chat "1968250737"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(token_Bot, secured_client);
String mensaje = "";

//Servidor MQTT
const char* mqtt_server = "a2tulmi9b492z-ats.iot.us-east-2.amazonaws.com";  //modificar (Settings > end point) END POINT
const int mqtt_port = 8883;
String Read_rootca;
String Read_cert;
String Read_privatekey;
//********************************
#define BUFFER_LEN  256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];
int count = 1;
//********************************

//Configuración de cliente MQTT
WiFiClientSecure espClient;
PubSubClient client(espClient);

//Conectar a red Wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando.. ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

//Callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//Conectar a broker MQTT
void reconnect() {
  
  // Loop para reconección
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Creando un ID como ramdon
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    
    // Intentando conectarse
    if (client.connect(clientId.c_str())) {
      Serial.println("conectada");
      
    // Conectado, publicando un payload...
      client.publish("ei_out", "hello world");
    
    // ... y suscribiendo
      client.subscribe("ei_in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Esperando 5 segundos");
      
      // Tiempo muerto de 5 segundos
      delay(5000);
    }
  }
}



//***************************************************************
// Termina TELEGRAM

// Definición pines EnA y EnB para el control de la velocidad (en el puente H)
int VelocidadMotor1 = 5; 
int VelocidadMotor2 = 18;

// Definición pines sensor distancia y variables para el cálculo de la distancia
int echoPin = 2;  //pines ultrasonido
int trigPin = 3;   //pines ultrasonido
// Variables de apoyo para calcular distancia de un objeto
long duration; 
int distance; 
int delayVal;

// Definición de los pines de control de giro de los motores In1, In2, In3 e In4
// Primer motor
int Motor1A = 13; 
int Motor1B = 12;  
// Segundo motor
int Motor2C = 33; 
int Motor2D = 32; 

// Variable para el control de la posición del servo y observaciones de distancia (izquierda derecha)
int servoPos = 0;
int servoReadLeft = 0;
int servoReadRight = 0;

// Configuración inicial
void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);

  // Vinculamos el pin digital 4 al servo instanciado arriba
  myservo.attach(4);

  // Establecemos modo de los pines del sensor de ultrasonidos
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  // Establecemos modo de los pines del control de motores
  pinMode(Motor1A,OUTPUT);
  pinMode(Motor1B,OUTPUT);
  pinMode(Motor2C,OUTPUT);
  pinMode(Motor2D,OUTPUT);
  pinMode(VelocidadMotor1, OUTPUT);
  pinMode(VelocidadMotor2, OUTPUT);

  // Configuración
  Serial.print("Conectando a la red wifi... ");
  Serial.println(ssid);
  //Seteo de la red Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Conectado a la red wifi!!!");
  Serial.print("Dirección ip: ");
  Serial.println(WiFi.localIP());//Imprimimos la direción ip local
  bot.sendMessage(ID_Chat, "Sistema preparado para entradas!!!", ""); //Enviamos un mensaje a telegram para informar que el sistema está listo

  setup_wifi();
  delay(1000);

  // Código AWS
  //*********************
  if (!SPIFFS.begin(true)) {
    Serial.println("Se ha producido un error al montar SPIFFS");
    return;
  }
  //**********************
  //Root CA leer archivo.
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if (!file2) {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Root CA File Content:");
  while (file2.available()) {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //*****************************
  // Cert leer archivo
  File file4 = SPIFFS.open("/fb7fbec4-certificate.pem.crt", "r"); //modificar
  if (!file4)  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Cert File Content:");
  while (file4.available()) {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }
  //***************************************
  //Privatekey leer archivo
  File file6 = SPIFFS.open("/fb7fbec4-private.pem.key", "r");//modificar
  if (!file6) {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("privateKey contenido:");
  while (file6.available()) {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //=====================================================

  char* pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char* pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char* pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("================================================================================================");
  Serial.println("Certificados que pasan adjuntan al espClient");
  Serial.println();
  Serial.println("Root CA:");
  Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");

  espClient.setCACert(pRead_rootca);
  espClient.setCertificate(pRead_cert);
  espClient.setPrivateKey(pRead_privatekey);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //******************************************
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(mac_Id);
  //****************************************
  delay(2000);
  //Termina configuración

  // Configuramos velocidad de los dos motores
  analogWrite(VelocidadMotor1, 255); 
  analogWrite(VelocidadMotor2, 255);  

  myservo.write(60);
}


void loop() {
  delay(50); 

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  distance = medirDistancia();
  Serial.println(distance);

  if(distance < 15){
    stopCar();
    
    // Miramos a la derecha    
    myservo.write(0);  
    delay(600);  // es importante hacer un tiempo de espera mientras el servo mide
    servoReadRight = medirDistancia();

    // Miramos a la izquierda                             
    myservo.write(170);  
    delay(600); 
    servoReadLeft = medirDistancia();

    // Miramos de frente 
    myservo.write(60);  
    delay(600);

    if(servoReadLeft > servoReadRight){
      Serial.println("Giro izquierda");
      turnLeftCar();
    }

    if(servoReadRight >= servoReadLeft){
      Serial.println("Giro derecha");
      turnRightCar(); 
    }

    //AWS
    //=============================================================================================
    String macIdStr = mac_Id;
    String valorIzquierda = String(servoReadLeft);
    String valorDerecha = String(servoReadRight);
    snprintf (msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"Izquierda\" : %s, \"Derecha\" : %s}", macIdStr.c_str(), valorIzquierda.c_str(), valorDerecha.c_str());
    Serial.print("Publicando mensaje: ");
    Serial.print(count);
    Serial.println(msg);
    client.publish("laboratorio", msg);//se puede modificar el nombre del topic
    count = count + 1;
    //================================================================================================

    // TELEGRAM
    //================================================================
    String mensajeTelegram = "¡Obstáculo detectado!" ".\n";
      mensajeTelegram += "Distancias: .\n\n";
      mensajeTelegram += "Distancia izquierda: " + valorIzquierda + "\n";
      mensajeTelegram += "Distancia derecha: " + valorDerecha + "\n";
      bot.sendMessage(ID_Chat, mensajeTelegram, "");
  }
  
  if(distance > 15){
    Serial.println("Recto");
    moveForwardCar();
  }
  
}

void stopCar(){
  // Paramos el carrito
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor2D, LOW);                       
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor2D,LOW); 
}

void turnRightCar(){
  // Configuramos sentido de giro para dirar a la derecha
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor2D,LOW);
  digitalWrite(Motor1A, HIGH);
  digitalWrite(Motor2D,LOW);
  delay(250);
}

void turnLeftCar(){
  // Configuramos sentido de giro para dirar a la izquierda
  digitalWrite(Motor1A,LOW);
  digitalWrite(Motor2D, LOW);
  digitalWrite(Motor1A,LOW);
  digitalWrite(Motor2D, HIGH);
  delay(250);
}

void moveForwardCar(){
  // Configuramos sentido de giro para avanzar
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor2D, LOW);                       
  digitalWrite(Motor1A, HIGH);
  digitalWrite(Motor2D,HIGH); 
}

int medirDistancia(){
  // Lanzamos pulso de sonido
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Leemos lo que tarda el pulso en llegar al sensor y calculamos distancia
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Devolver distancia calculada
  return distance;    
}

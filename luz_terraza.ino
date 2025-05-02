//#include <WiFi.h>
#include <Preferences.h>
#include <NTPClient.h> // Consultar servidor hora
#include <WiFiUdp.h> // Dependencia NTPClient
#include <ESP8266WiFi.h>  // Correct WiFi library for ESP8266
//#include <EEPROM.h>       // Replace Preferences.h



//#include <WiFiClient.h>;

//const char* ssid = "MOVISTAR-WIFI6-9C20";
//const char* password = "rTt2NTM9XqW8xdzgxa2B";
const char* ssid = "Taberna de Moe";
const char* password = "demasiadolarga";
const long TIMEOUT = 1000000;
const int ledPin = 16;
const int fotoresistorPin = 4; // Pin FOTORRESISTOR
const int movimientoPin = 2; // Pin SENSOR MOVIMIENTO


// VARIABLES GLOBALES DISCO:
int VALOR_NOCTURNIDAD = 0;
int NUMERO_SEGUNDOS_DELAY = 0;
bool ESTADO_ON = false;
bool ESTADO_OFF = true;
bool ESTADO_AUTO = false;
bool lamparasEncendidasEnAuto = false;

unsigned long lastOnAutoMode = 0;
unsigned long tiempoTranscurrido = 0;
bool debug = true;
String buffer = "";
WiFiServer serverTCP(1996); //Defino el servidor leyendo en el puerto 1996
Preferences preferences;
bool estadoRecuperado = false;

// Configuración NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600);

void setup(void) {
  // Configurar el pin como salida
  pinMode(ledPin, OUTPUT);
  pinMode(movimientoPin , INPUT);
  pinMode(fotoresistorPin , INPUT);

  digitalWrite(ledPin, HIGH);
  getPreferences();
  savePreferences();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(1500); //tiempo conexión wifi
  if (debug) {
    Serial.begin(115200);
    delay (2000);
    printPreferences();
    Serial.printf("\nEstado de la conexión: %d\n", WiFi.status());
    Serial.printf("Conectando a %s\n", ssid);
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(4000);
    if (debug) Serial.printf("Estado de la conexión: %d\n", WiFi.status());
    reconnectWiFi();
  }
  if (debug) valores_conexion();

  //Arranco el servidor TCP
  serverTCP.begin();
  if (debug) Serial.printf("Server TCP started, open %s in port 1996\n", WiFi.localIP().toString().c_str());
  digitalWrite(ledPin, LOW);

  // Inicialización del cliente NTP
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(1000);
  }
}

void loop() {
  reconnectWiFi();
  WiFiClient clienteTCP = serverTCP.available(); // Escucha a los clientes entrantes
  String comandoTCP, respuesta = "";
  if (!estadoRecuperado) {
    restaurarEstado();
  }
  while (clienteTCP && clienteTCP.connected()) { // Si se conecta un nuevo cliente
    if (clienteTCP.available()) {
      if (debug) Serial.println("[Cliente available]"); // Si hay bytes para leer desde el cliente
      tratarRespuesta(clienteTCP);
      clienteTCP.stop();
    }
  }
  autoMode();
}

void tratarRespuesta(WiFiClient clienteTCP) {
  buffer = clienteTCP.readStringUntil('\n'); //Lee mensaje hasta un salto   
  if (buffer.indexOf("<MODE>ON<MODE/>") != -1) {
    ESTADO_ON = true;
    ESTADO_OFF = false;
    ESTADO_AUTO = false;
    if (debug) Serial.println("Dentro de estado ON");
    lastOnAutoMode = 0; //Desconfiguramos el ultimo valor de ON en modo AUTO
    encenderLamparas();
  } else if (buffer.indexOf("<MODE>OFF<MODE/>") != -1) {
    ESTADO_OFF = true;
    ESTADO_ON = false;
    ESTADO_AUTO = false;
    lastOnAutoMode = 0; //Desconfiguramos el ultimo valor de ON en modo AUTO
    if (debug) Serial.println("Dentro de estado OFF");
    apagarLamparas();
  } else if (buffer.indexOf("<MODE>AUTO<MODE/>") != -1) {
    ESTADO_AUTO = true;
    ESTADO_OFF = false;
    ESTADO_ON = false;
    VALOR_NOCTURNIDAD = buffer.substring(buffer.indexOf("<L>") + 3, buffer.indexOf("<L/>")).toInt(); // Extrae desde la posición 17 hasta la 21
    NUMERO_SEGUNDOS_DELAY = buffer.substring(buffer.indexOf("<S>") + 3, buffer.indexOf("<S/>")).toInt(); // Extrae desde la posición 14 hasta la 18
    if (debug) Serial.println("Se ejecuta Modo AUTO con valor nocturnidad = "+ String(VALOR_NOCTURNIDAD) +"; Valor Segundos =" + String(NUMERO_SEGUNDOS_DELAY));
  }
  String respuesta = rescatarEstado();
  if (debug) Serial.println(respuesta);
  if (debug) Serial.println("Enviando respuesta: " + respuesta);

  //Envía respuesta al cliente:
  clienteTCP.println(respuesta);
  savePreferences(); //Guardamos nuevas configuraciones recogidas en los mensajes
  printPreferences();
}
  
String rescatarEstado() {
  String respuesta = "";
  Serial.println("ESTADO_ON" + ESTADO_ON);
  if (ESTADO_ON) { 
    Serial.println("ESTADO_ON");
    respuesta = "<DEVICE>LUZ1<DEVICE/><MODE>ON<MODE/>";
  }
  else if (ESTADO_OFF) {
    
    if (debug) Serial.println("ESTADO_OFF");
    respuesta = "<DEVICE>LUZ1<DEVICE/><MODE>OFF<MODE/>";
  }
  else if (ESTADO_AUTO) {
    if (debug) Serial.println("ESTADO_AUTO");
    respuesta = "<DEVICE>LUZ1<DEVICE/><MODE>AUTO<MODE/>" "<S>" + String(NUMERO_SEGUNDOS_DELAY) + "<S/>";
  }
  return respuesta;
}

void autoMode() {
  if (!ESTADO_AUTO) return;

  if (debug) Serial.println("MODO AUTO: Ejecutando tarea tomar lectura ...");

  if (tomarLecturaFotorresistor() == 1) {
    int valorMovimiento = tomarLecturaMovimiento();

    if (valorMovimiento == 1) {
      if (!lamparasEncendidasEnAuto) {
        encenderLamparas();
        if (debug) Serial.println("Encendidas en modo AUTO");
        lamparasEncendidasEnAuto = true;
      }
      timeClient.update();
      lastOnAutoMode = timeClient.getEpochTime();  // 🔁 reinicia contador si hay movimiento
    }
  }

  checkTimeoutAutoMode(); // 💡 se apagará solo si pasó el tiempo y no hay movimiento
}


int tomarLecturaFotorresistor() {
  int lecturaDigital = digitalRead(fotoresistorPin); // Lee el valor analógico (0-4095)
  if (debug) Serial.print("Valor fotoresistor analógico: ");
  if (debug) Serial.println(lecturaDigital);
  return lecturaDigital;
}

int tomarLecturaMovimiento() {
  int lecturaDigital = digitalRead(movimientoPin);
  if (debug) Serial.print("Valor sensor digital de movimiento: ");
  if (debug) Serial.println(lecturaDigital);
  return lecturaDigital;
}

void encenderLamparas() {
  if (debug) Serial.println("Enciendo lámparas");
  digitalWrite(ledPin,HIGH);
}

void apagarLamparas() {
    if (debug) Serial.println("Apago lámparas");
    digitalWrite(ledPin,LOW);
}


void reconnectWiFi() {
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    while (intentos < 20 | WiFi.status() != WL_CONNECTED) {
        if (debug) Serial.println("WiFi desconectado. Intentando reconectar...");
        WiFi.begin("SSID", "password"); // Reemplaza con tu SSID y contraseña
        if (debug) Serial.println("Conectando a WiFi...");
        delay(intentos*1000);
        if (debug) Serial.println("Reconectado a WiFi");
        intentos ++;
    }
    intentos = 0;
  }
}

void restaurarEstado() {
  if (ESTADO_ON) {
    encenderLamparas();
    if (debug) Serial.println("Enciendo lámaparas por configuración anterior");
  }else if (ESTADO_OFF) {
    apagarLamparas();
    if (debug) Serial.println("Apago lámaparas por configuración anterior");
  }else if (ESTADO_AUTO) {
    autoMode();
    if (debug) Serial.printf("Activo estado auto con valor de nocturnidad:%i y segundos de delay:%i\n", VALOR_NOCTURNIDAD, NUMERO_SEGUNDOS_DELAY);  
  }
  estadoRecuperado = true;
}

void savePreferences() {
  preferences.begin("preferencesLuz", false);  // false para read/write
  preferences.putInt("NUMERO_S", NUMERO_SEGUNDOS_DELAY);
  preferences.putBool("ESTADO_ON", ESTADO_ON);
  preferences.putBool("ESTADO_OF", ESTADO_OFF);
  preferences.putBool("ESTADO_A", ESTADO_AUTO);
  preferences.end();
}

void getPreferences() {
  preferences.begin("preferencesLuz", true);  // false para read/write
  NUMERO_SEGUNDOS_DELAY = preferences.getInt("NUMERO_S", NUMERO_SEGUNDOS_DELAY);
  ESTADO_ON = preferences.getBool("ESTADO_ON", ESTADO_ON);
  ESTADO_OFF = preferences.getBool("ESTADO_OF", ESTADO_OFF);
  ESTADO_AUTO = preferences.getBool("ESTADO_A", ESTADO_AUTO);
  preferences.end();
}

void printPreferences() {
  Serial.println("** Valores disco **");
  Serial.printf("VALOR_NOCTURNIDAD: %i\n", VALOR_NOCTURNIDAD);
  Serial.printf("NUMERO_SEGUNDOS_DELAY: %i\n", NUMERO_SEGUNDOS_DELAY);
  Serial.printf("ESTADO_ON: %s\n", ESTADO_ON ? "true" : "false");
  Serial.printf("ESTADO_OFF: %s\n", ESTADO_OFF ? "true" : "false");
  Serial.printf("ESTADO_AUTO: %s\n", ESTADO_AUTO ? "true" : "false");
  Serial.println("*");
}

/*
    Estado de la conexión y obtencion de parámetros
      0 : WL_IDLE_STATUS cuando el Wi-Fi está en proceso de cambiar de estado
      1 : WL_NO_SSID_AVAIL en caso de que el SSID configurado no pueda ser alcanzado
      3 : WL_CONNECTED después de establecer una conexión satisfactoriamente
      4 : WL_CONNECT_FAILED si la contraseña es incorrecta
      6 : WL_DISCONNECTED si el módulo no está configurado en el modo de estación
  */
void valores_conexion() {
  Serial.println("*** VALORES CONEXION ***");
  Serial.printf("Estado de la conexión: %d\n", WiFi.status());
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("Password: %s\n", WiFi.psk().c_str());
  Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str());
  Serial.print("Conectado, dirección IP: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Getaway IP: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.print("Máscara de subred: ");
  Serial.println(WiFi.subnetMask());
  Serial.printf("Conectado, dirección MAC: %s\n", WiFi.macAddress().c_str());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  Serial.println("");
}

// Función que verifica si se ha superado el tiempo de inactividad
void checkTimeoutAutoMode() {
  if (!lamparasEncendidasEnAuto || lastOnAutoMode == 0) return;

  timeClient.update();
  tiempoTranscurrido = timeClient.getEpochTime() - lastOnAutoMode;

  if (debug) {
    Serial.print("Tiempo transcurrido en AUTO --> ");
    Serial.println(tiempoTranscurrido);
  }

  if (tiempoTranscurrido >= NUMERO_SEGUNDOS_DELAY || tomarLecturaFotorresistor() == 0) {
    apagarLamparas();
    lastOnAutoMode = 0;
    lamparasEncendidasEnAuto = false;
    if (debug) Serial.println("Apagadas en modo AUTO");
  }
}
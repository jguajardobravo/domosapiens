  #include <ESP8266WiFi.h>
//#include <WiFiClient.h>;

const char* ssid = "Router-Salon";
const char* password = "1234567890123";
const long TIMEOUT = 1000000;

/*
   Defino el servidor leyendo en el puerto 80
*/
WiFiServer server(1996);
WiFiClient cliente;


bool debug = false;

void setup(void)
{
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);

  if (debug) {
    Serial.begin(115200);
    delay (1000);
    Serial.printf("\nEstado de la conexión: %d\n", WiFi.status());
    Serial.printf("Conectando a %s\n", ssid);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    if (debug) Serial.printf("Estado de la conexión: %d\n", WiFi.status());
    delay(500);
  }
  if (debug) valores_conexion();

  /*
     Arranco el servidor
  */
  server.begin();
  if (debug) Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  long tiempo = 0;
  cliente = server.available();
  // wait for a client (web browser) to connect
  bool cliente_desconectado = true;
  if (cliente)
  {
    cliente_desconectado = false;
    if (debug) Serial.println("[Client connected]");
    char c = ' ';

    while (cliente.connected())
    {
      bool primeravez = true;
      while (cliente.available() == 0 && cliente.connected()) {
        if (primeravez && debug) Serial.println("     Esperando ... cliente.connected()");
        primeravez = false;
      }

      tiempo = 0;
      c = ' ';
      while ((c < '0' || c > '3') && tiempo < TIMEOUT && cliente.connected()) {
        //while (c < 48 || c > 51 && cliente.connected()) {
        c = cliente.read();
        //Serial.printf("%02x", c);
        if (c >= '0' && c <= '3') {
          if (debug) {
            Serial.println("     Leido:");
            Serial.print("        Serial.write(c)-->");
            Serial.write(c);
            Serial.println("");
            Serial.print("        Serial.println(c)-->");
            Serial.println(c);
            Serial.print("        Serial.printf(\"Caracter %c\", c)-->");
            Serial.printf("Caracter %c\n", c);
          }
          contestar_Cliente(evaluar_entrada(c));
        }
        tiempo++;
      }
    }

    if (debug) Serial.println("     Estoy en bucle client");

    if (tiempo >= TIMEOUT)
      cliente.stop();
  }
  if (!cliente_desconectado && debug)
    Serial.println("[Client disonnected]");
}
void contestar_Cliente (int respuesta) {
  cliente.println(respuesta);
}

int evaluar_entrada (char caracter) {
  if (debug) {
    Serial.print("     Evaluando entrada, recibo:");
    Serial.println(caracter);
  }

  if (caracter == '1') {
    if (debug) Serial.println("     Recibo 1 voy a evaluar");
    if (digitalRead(0) == LOW) {
      if (debug) Serial.println("     Esta apagado, enciendo");
      digitalWrite(0, HIGH);
    }
    else {
      if (debug) Serial.println("     Esta encendido, apago");
      digitalWrite(0, LOW);
    }
  }

  if (caracter == '2') {
    if (debug) Serial.println("     Recibo 2 voy a evaluar");
    if (digitalRead(2) == LOW) {
      if (debug) Serial.println("     Esta apagado, enciendo");
      digitalWrite(2, HIGH);
    }
    else {
      if (debug) Serial.println("     Esta encendido, apago");
      digitalWrite(2, LOW);
    }
  }

  int respuesta = 0;

  if (digitalRead(0) == HIGH) respuesta++;
  if (digitalRead(2) == HIGH) respuesta += 2;
  return respuesta;
}

// prepare a web page to be send to a client (web browser)
String prepareHtmlPage()
{
  String htmlPage =
    String("HTTP/1.1 200 OK\r\n") +
    "Content-Type: text/html\r\n" +
    "Connection: close\r\n" +  // the connection will be closed after completion of the response
    "Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
    "\r\n" +
    "<!DOCTYPE HTML>" +
    "<html>" +
    "Analog input:  " + String(analogRead(A0)) +
    "</html>" +
    "\r\n";
  return htmlPage;
}
void valores_conexion() {
  Serial.println("******** VALORES CONEXION **********");
  /*
     Estado de la conexión

    0 : WL_IDLE_STATUS cuando el Wi-Fi está en proceso de cambiar de estado
    1 : WL_NO_SSID_AVAIL en caso de que el SSID configurado no pueda ser alcanzado
    3 : WL_CONNECTED después de establecer una conexión satisfactoriamente
    4 : WL_CONNECT_FAILED si la contraseña es incorrecta
    6 : WL_DISCONNECTED si el módulo no está configurado en el modo de estación
  */

  Serial.printf("\nEstado de la conexión: %d\n", WiFi.status());

  /*
     Obtener el SSID
  */
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  /*
     Obtiene Password
  */
  Serial.printf("Password: %s\n", WiFi.psk().c_str());
  /*
     Obtiene BSSID
  */

  // Esta forma no me funciona
  //
  //  uint8_t bssid[6];
  //  WiFi.BSSID(bssid);
  //  Serial.printf("Dirección MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);


  /*
     Otra forma
  */
  Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str());

  /*
     Obtener la dirección IP
  */
  Serial.print("Conectado, dirección IP: ");
  Serial.println(WiFi.localIP());

  /*
     Obtener gateway
  */
  Serial.printf("Getaway IP: %s\n", WiFi.gatewayIP().toString().c_str());

  /*
     Obtener la mascara de subred
  */
  Serial.print("Máscara de subred: ");
  Serial.println(WiFi.subnetMask());

  /*
     Ibtener la macAddress
  */

  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  Serial.printf("Conectado, dirección MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  /*
     Otra manera de sacar la macAddress
  */
  Serial.printf("Conectado, dirección MAC: %s\n", WiFi.macAddress().c_str());

  /*
     Obtener DNS
  */
  Serial.print("DNS: #1, #2 IP: ");
  WiFi.dnsIP(0).printTo(Serial);
  Serial.print(", ");
  WiFi.dnsIP(1).printTo(Serial);
  Serial.println();

  /*
     Obtener y cambiar el Hostname
  */
  Serial.printf("Hostname por defecto: %s\n", WiFi.hostname().c_str());
  WiFi.hostname("Station_Tester_02");
  Serial.printf("Nuevo hostname: %s\n", WiFi.hostname().c_str());

  /*
     Potencia de la señal
  */
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());

}

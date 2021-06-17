#include <ESP8266WiFi.h>

/* VARIABLES GLOBALES */
const char* ssid = "Router-Salon";
//const char* ssid = "MOVISTAR_AAA5_EXT";
const char* password = "1234567890123";
const byte ip_middleware[] = { 192, 168, 1, 100 };
bool debug = true;
bool primeraVez = true;
int contconexion = 0;
String comando = "";
String bufferMiddleware = "";

WiFiServer serverTCP(1996);
String buffer = "";
void setup() {
  delay(2000);
  if (debug) {
    Serial.begin(115200);
    Serial.println("");
  }
  // Conexión WIFI
  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion < 50) {
    ++contconexion;
    delay(500);
    if (debug) Serial.print(".");
  }
  if (contconexion < 50) {
    if (debug) {
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
      Serial.println("MAC: " + WiFi.macAddress());
    }
    serverTCP.begin(); // iniciamos el servidor WEB
  }
  else if (debug) {
    Serial.println("");
    Serial.println("Error de conexion");
  }

}

void loop() {
  WiFiClient clienteTCP = serverTCP.available();
  if (debug) {
    if (primeraVez) {
      primeraVez = false;
      Serial.println("Escuchando peticiones... ");
    }
  }
  // wait for a client (Android) to connect
  if (clienteTCP) {
    if (debug) Serial.println("[Client connected]");
    while (clienteTCP.connected()) {
      if (clienteTCP.available()) {
        buffer = clienteTCP.readStringUntil('\n');
        if (buffer.indexOf("LEDS") >= 0 ) {
          if (debug)Serial.println("Recibo Leds");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("TIRA") >= 0) {
          if (debug)Serial.println("Recibo Tira");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("IniciarPrograma") >= 0) {
          if (debug)Serial.println("Recibo IniciarPrograma");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("IniciarZ1") >= 0) {
          if (debug)Serial.println("Recibo IniciarZ1");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("IniciarZ2") >= 0) {
          if (debug)Serial.println("Recibo IniciarZ2");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("ApagarTodo") >= 0) {
          if (debug)Serial.println("Recibo ApagarTodo");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("Estado") >= 0) {
          if (debug)Serial.println("Recibo Estado");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        }else if (buffer.indexOf("TEMPERATURA") >= 0) {
          if (debug)Serial.println("Recibo Temperatura");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else if (buffer.indexOf("HUMEDAD") >= 0) {
          if (debug)Serial.println("Recibo Humedad");
          String respuestaMiddleware = sendToMiddleWare(buffer);
          responseClient(clienteTCP, respuestaMiddleware);
        } else {
          clienteTCP.println("He recibido --> " + buffer + "Y no corresponde con ningun commando.¿Quieres conversación? \n");
          if (debug)Serial.println("He recibido --> " + buffer + "Y no corresponde con ningun commando.¿Quieres conversación? \n");
        }
      }
    }
    // Cerramos la conexión
    buffer = "";
     primeraVez = true;
    clienteTCP.stop();
    if (debug) {
      Serial.println("Client disconnected.");
    }
  }
}
String sendToMiddleWare(String comando) {
  if (debug)Serial.println("Mandando commando a MiddleWare");
  WiFiClient servidorMiddleware;
  char c = ' ';
  if (debug) Serial.println("***Estado***");
  if (!servidorMiddleware.connect(ip_middleware, 1996)) {
    if (debug)Serial.println("Imposible conectar con Middleware");
    return "error";
  }
  else {
    Serial.println("Mando a MiddleWare---->    <request><comando>" + comando + "</comando></request>");
    servidorMiddleware.println("<request><comando>" + comando + "</comando></request>");
    bool primeravez = true;
    while (servidorMiddleware.available() == 0 && servidorMiddleware.connected()) {
      if (debug) {
        if (primeravez) Serial.println("[Esperando ...]");
        primeravez = false;
      }
    }
    while (servidorMiddleware.available()) {
      if (debug) Serial.println("servidorMiddleware available");
      String bufferMiddleware = servidorMiddleware.readStringUntil('\n');
      String response = "";
      if (bufferMiddleware.indexOf("</response>") >= 0) {
        int finRespuesta = bufferMiddleware.indexOf("</response>");
        int iniRespuesta = bufferMiddleware.indexOf("<response>") + 10;
        int responseLength = finRespuesta - iniRespuesta;
        response = bufferMiddleware.substring(iniRespuesta, finRespuesta);
        if (debug) Serial.println("Encuentro etiqueta <response>" + response + "</response>");
        if (response.indexOf("</ack>") >= 0) {
          String ack = "";
          int finAck = response.indexOf("</ack>");
          int iniAck = response.indexOf("<ack>") + 5;
          ack = response.substring(finAck, iniAck);
          if (debug) Serial.println("Encuentro ACK, y es -->" + ack);
          if (ack.length() > 0 && ack == "1") {
            if (response.indexOf("</estado>") > 0 ) {
              int finEstado = response.indexOf("</estado>");
              int iniEstado = response.indexOf("<estado>") + 8;
              String estado = response.substring(finEstado, iniEstado);
              if (debug) {
                Serial.print("Estado recibido por MiddleWare --> ");
                Serial.println(estado);
              }
              return estado;
            }
            else {
              if (debug) Serial.println("No encuentro estado en la respuesta");
            }
          }
        }
      }
      break;
    }
  }
  servidorMiddleware.stop();
  Serial.println("estado = null");
  return "";
}
void responseClient(WiFiClient clienteTCP, String respuestaMiddleware) {
  if (debug)Serial.println("Mandando commando a MiddleWare");
  clienteTCP.println(respuestaMiddleware);
  clienteTCP.stop();
}

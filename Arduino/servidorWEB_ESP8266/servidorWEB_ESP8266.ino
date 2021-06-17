#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "Router-Salon"
#define STAPSK  "1234567890123"
#endif
const byte ip_middleware[] = { 192, 168, 1, 100 };
const char* ssid     = STASSID;
const char* password = STAPSK;
String bufferMiddleware = "";
ESP8266WebServer server(80);
bool debug = true;
String botonLed = "";
String botonTira = "";
String botonZ1 = "";
String botonZ2 = "";
String backend = "";
String botonIniciarPrograma = "";
String botonTemperatura = "";
String botonHumedad = "";
String graficaClima = "";
String error = "<p style='text-color:green'>No hay errores al evaluar comandos</p>";
const int led = LED_BUILTIN;
String estadoRiego, estadoAcuario, estadoClima = "";
String paginaHtml = "";
void handleRoot() {
  estadoAcuario, estadoClima, estadoRiego = "";
  botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Led</button></a></p>";
  botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Tira</button></a></p>";
  botonZ1 = "<p><a href='/IniciarZ1'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
  botonZ2 = "<p><a href='/IniciarZ2'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
  botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  botonTemperatura = "<p><a href='/Temperatura'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Mandar comando Temperatura</button></a></p>";
  botonHumedad = "<p><a href='/Humedad'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Mandar comando Humedad</button></a></p>";
  mandarComando("EstadoAcuario");
  mandarComando("EstadoRiego");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
String pagina() {
  //TODO analizar todos los posibles estados y recargar variables boton.
  mandarComando("Temperatura");
  tratarRespuesta();
  if ( estadoClima.equals("Failed to open file for reading")) {
    error = "<p style='text-color:red'>Comando Desconocido</p>";
  } else {
    graficaClima =
      "<script type=\"text/javascript\">"
      "var chart1;"
      "$(document).ready(function(){"
      "chart1 = new Highcharts.Chart({"
      "chart: {renderTo: 'container1'},"
      "series: [{data: [" + estadoClima + "]}]"      " });"
      "});"
      "</script>"
      "<div id=\"container1\" style=\"width: 700px; height: 400px \"></div>";
  }
  return "<html>"
         "<head>"
         "<meta charset='utf-8' />"
         "<title>Servidor Web ESP8266</title>"
         "<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.4.4/jquery.min.js' type='text/javascript'></script>"
         "<script src='https://code.highcharts.com/highcharts.js' type='text/javascript'></script>"
         "</head>"
         "<body>"
         "<center>"
         "<div>"
         + error +
         "</div>"
         "<div style='border-style: groove; border-color:black; margin:20px;'>"
         "<div style='border-style: groove; border-color:blue'>"
         "<h1>Pecera</h1>"
         + botonLed + botonTira +
         "</div>"
         "<div style='border-style: groove; border-color:green margin:20px;'>"
         "<h1>Riego</h1>"
         + botonZ1 + botonZ2 + botonIniciarPrograma +
         "</div>"
         "<div style='border-style: groove; border-color:orange margin:20px;'>"
         "<h1>Clima</h1>"
         "<div style='float:left;margin: 20px;align-text:center'>"
         + graficaClima +
         "</div>"
         "<div style='float:right;margin: 20px;align-text:center'>"
         + botonTemperatura + botonHumedad +
         "</div>"
         "</div>"
         "</center>"
         "</body>"
         "</html>";
}
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  delay(2000);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  //acuario
  server.on("/leds", leds);
  server.on("/tira", tira);
  server.on("/EstadoAcuario", comandoEstadoAcuario);
  //riego
  server.on("/EstadoRiego", comandoEstadoRiego);
  server.on("/IniciarZ1", riegoZ1);
  server.on("/IniciarZ2", riegoZ2);
  server.on("/IniciarPrograma", iniciarPrograma);
  server.on("/ApagarTodo", apagarTodo);
  //Clima
  server.on("/Temperatura", temperatura);
  server.on("/Humedad", humedad);
  //errores y root
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

//comandos riego
void comandoEstadoRiego () {
  mandarComando("EstadoRiego");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void riegoZ1() {
  mandarComando("IniciarZ1");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void riegoZ2 () {
  mandarComando("IniciarZ2");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void iniciarPrograma () {
  mandarComando("IniciarPrograma");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void apagarTodo() {
  mandarComando("ApagarTodo");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
//Comandos acuario
void comandoEstadoAcuario() {
  mandarComando("EstadoAcuario");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void tira() {
  mandarComando("TIRA");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void leds() {
  mandarComando("LEDS");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
//ComandosClima
void humedad() {
  mandarComando("Humedad");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void temperatura() {
  mandarComando("Temperatura");
  paginaHtml = pagina();
  server.send(200, "text/html", paginaHtml);
}
void loop(void) {
  server.handleClient();
}
void mandarComando(String comando) {
  WiFiClient servidorMiddleware;
  if (!servidorMiddleware.connect(ip_middleware, 1996)) {
    if (debug)Serial.println("Imposible conectar con Middleware");
  }
  else {
    if (debug) Serial.println("Mando a MiddleWare---->    <request><comando>" + comando + "</comando></request>");
    servidorMiddleware.println("<request><comando>" + comando + "</comando></request>\n");
    while ( servidorMiddleware.connected()) {
      if (servidorMiddleware.available()) {
        servidorMiddleware.setTimeout(5000);
        String bufferMiddleware = servidorMiddleware.readStringUntil('\n');
        String response = "";
        if (bufferMiddleware.indexOf("</response>") >= 0) {
          int finRespuesta = bufferMiddleware.indexOf("</response>");
          int iniRespuesta = bufferMiddleware.indexOf("<response>") + 10;
          int responseLength = finRespuesta - iniRespuesta;
          response = bufferMiddleware.substring(iniRespuesta, finRespuesta);
          if (debug) Serial.println(response);
          if (response.indexOf("</ack>") >= 0) {
            String ack = "";
            int finAck = response.indexOf("</ack>");
            int iniAck = response.indexOf("<ack>") + 5;
            ack = response.substring(finAck, iniAck);
            if (debug) Serial.println("Encuentro ACK, y es -->" + ack);
            if (ack.length() > 0 && ack == "1") {
              if (response.indexOf("</backend>") > 0 ) {
                int finBackend = response.indexOf("</backend>");
                int iniBackend = response.indexOf("<backend>") + 9;
                backend = response.substring(finBackend, iniBackend);
                if (response.indexOf("</estado>") > 0 ) {
                  int finEstado = response.indexOf("</estado>");
                  int iniEstado = response.indexOf("<estado>") + 8;
                  if (backend.equals("Clima")) {
                    estadoClima = response.substring(finEstado, iniEstado);
                  }
                  else if (backend.equals("Riego")) {
                    Serial.print("Guardo estado riego en variable estadoRiego");
                    estadoRiego = response.substring(finEstado, iniEstado);
                    Serial.println(" -->" + estadoRiego);
                  } else if (backend.equals("Acuario")) {
                    estadoAcuario = response.substring(finEstado, iniEstado);
                  }
                  if (debug) {
                    Serial.print("Estado recibido por " + backend + "--> ");
                    Serial.println(estadoAcuario + estadoClima + estadoRiego);
                  }
                }
                else {
                  if (debug) Serial.println("No encuentro estado en la respuesta");

                }
              }

            }
            else {
              if (debug)Serial.println("Error en la comunicación. Recibo ACK=0 o desconocido");
            }
          }
        }
      }
    }
    Serial.println("Salgo de while(connected) ");
    bufferMiddleware = "";
  }
}
void tratarRespuesta() {
  Serial.println("El estado estadoAcuario es " + estadoAcuario);
  Serial.println("El estado estadoClima es " + estadoClima);
  Serial.println("El estado estadoRiego es " + estadoRiego);

  if (estadoAcuario.equals("Los dos Apagados")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Tira</button></a></p>";
  } else if ( estadoAcuario.equals("Leds Encendidos")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Tira</button></a></p>";
  } else if ( estadoAcuario.equals("Tira Encendida")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Tira</button></a></p>";
  } else if ( estadoAcuario.equals("Los dos Encendidos")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Tira</button></a></p>";
  } else if ( estadoAcuario.equals("Error conexión acuario")) {
    error = "<p style='text-color:red'>Error conexión acuario</p>";
  } else if ( estadoAcuario.equals("Error en evaluación luces")) {
    error = "<p style='text-color:red'>Error en evaluación luces</p>";
  }

  if ( estadoRiego.equals("Error conexión Servidor Riego")) {
    error = "<p style='text-color:red'>Error conexión Servidor Riego</p>";
  } else if ( estadoRiego.equals("Riego Apagado")) {
    Serial.println("Entro aqui");
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estadoRiego.equals("Lado Izquierdo riego Encendido")) {
    botonZ1 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estadoRiego.equals("Lado Derecho riego Encendido")) {
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estadoRiego.equals("Programación Lado Derecho")) {
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estadoRiego.equals("Programación Lado Izquierdo")) {
    botonZ1 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  }
  if ( estadoRiego.equals("Comando Desconocido") || estadoAcuario.equals("Comando Desconocido") || estadoClima.equals("Comando Desconocido")) {
    error = "<p style='text-color:red'>Comando Desconocido</p>";
  }
  if (estadoClima.equals("Error conexión Servidor Clima")){
     error = "<p style='text-color:red'>Error conexión Servidor Clima</p>";
  }
}

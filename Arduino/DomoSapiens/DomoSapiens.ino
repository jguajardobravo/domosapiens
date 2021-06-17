#include <WiFi.h>
#include <WebServer.h>
//#include "soc/timer_group_struct.h"
//#include "soc/timer_group_reg.h"
//#include "esp_task_wdt.h"
/*
   Variables globales
*/
const char* ssid = "Router-Salon";
const char* password = "1234567890123";
bool debug = true;
TaskHandle_t t;
const byte ip_riego[] = { 192, 168, 1, 101 };
const byte ip_acuario[] = { 192, 168, 1, 104 };
const byte ip_temperatura[] = { 192, 168, 1, 105 };

/*
   Variables
*/
int tiempo = 0;

int contconexion = 0;

const long TIMEOUT = 10;

String buffer = "";                                       // Variable para guardar el request
String comando = "";                                      // Variable para guardar el comando leido
String estado = "";
String ack = "0";

/*
   Variables de TCP
*/

WiFiServer serverTCP(1996);

/*
   Variables WEB
*/
String bufferWEB = "";
WebServer serverWEB(80);
String botonLed = "";
String botonTira = "";
String botonZ1 = "";
String botonZ2 = "";
String botonIniciarPrograma = "";
String seleccion = "";

String error = "<p style='text-color:green'>No hay errores al evaluar comandos</p>";
String paginaHtml = "";


/*
   Comandos posibles
*/
String c_leds = "LEDS";
String c_tira = "TIRA";
String c_estadoAcuario = "EstadoAcuario";
String iniciaRiego = "IniciarPrograma";
String izquierda = "IniciarZ1";
String derecha = "IniciarZ2";
String apaga = "ApagarTodo";
String c_estadoRiego = "EstadoRiego";
String temp = "Temperatura";
String c_dia = "TempDia";
String c_mes = "TempMes";
String c_ano = "TempAno";

/*
   Respuestas posibles Acuario
            if (c == '0')
  String apagados = "Los dos Apagados";
  String ledsEncendido = "Leds Encendidos";
  String tiraEncendida = "Tira Encendida";
  String encendidos = "Los dos Encendidos";
            else if (c == 'e')
              estado = "Error conexión acuario";
            else
              estado = "Error en evaluación luces";

*/
/*
   Fin Variables
*/

void setup() {

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
      Serial.print("MAC: ");
      Serial.println(WiFi.macAddress());
    }
    serverTCP.begin(); // iniciamos el servidor TCP
  }
  else if (debug) {
    Serial.println("");
    Serial.println("Error de conexion");
  }

  /*
     Inicio server WEB
  */
  //acuario
  serverWEB.on("/leds", leds);
  serverWEB.on("/tira", tira);
  serverWEB.on("/EstadoAcuario", comandoEstadoAcuario);
  //riego
  serverWEB.on("/EstadoRiego", comandoEstadoRiego);
  serverWEB.on("/IniciarZ1", riegoZ1);
  serverWEB.on("/IniciarZ2", riegoZ2);
  //serverWEB.on("/IniciarPrograma", HTTP_POST, iniciarPrograma);
  serverWEB.on("/IniciarPrograma", iniciarPrograma);
  serverWEB.on("/ApagarTodo", apagarTodo);
  //errores y root
  serverWEB.onNotFound(handleNotFound);
  serverWEB.on("/", handleRoot);

  serverWEB.begin();
  if (debug) Serial.println("HTTP server started");



  /*
     Arranco tarea en core2
  */

  xTaskCreatePinnedToCore(
    loop2,
    "tarea 0",
    100000,
    NULL,
    1,
    &t,
    0);

  //  xTaskCreate(
  //    loop2,
  //    "tarea 0",
  //    90000,
  //    NULL,
  //    1,
  //    &t);
  delay(500);
}

void loop() {
  WiFiClient clienteTCP = serverTCP.available();                // Escucha a los clientes entrantes
  String comandoTCP, respuesta = "";
  if (clienteTCP) {                                             // Si se conecta un nuevo cliente
    while (clienteTCP.connected()) {
      clienteTCP.setTimeout(5000);
      if (clienteTCP.available()) {
        if (debug) Serial.println("[Cliente available]");        // si hay bytes para leer desde el cliente
        buffer = clienteTCP.readStringUntil('\n');              //
        String tiempo_riego = "0";
        if (buffer.length() > 15) {
          tiempo_riego = buffer.substring(15);
          buffer = buffer.substring(0, 15);
        }
        if (debug) Serial.println("buffer:" + buffer);
        if (buffer.indexOf("LEDS") >= 0 ) {
          comandoTCP = c_leds;
        } else if (buffer.indexOf("TIRA") >= 0) {
          comandoTCP = c_tira;
        } else if (buffer.indexOf(c_estadoAcuario) >= 0) {
          comandoTCP = c_estadoAcuario;
        } else if (buffer.indexOf("IniciarPrograma") >= 0) {
          comandoTCP = iniciaRiego;
        } else if (buffer.indexOf("IniciarZ1") >= 0) {
          comandoTCP = izquierda;
        } else if (buffer.indexOf("IniciarZ2") >= 0) {
          comandoTCP = derecha;
        } else if (buffer.indexOf("ApagarTodo") >= 0) {
          comandoTCP = apaga;
        } else if (buffer.indexOf(c_estadoRiego) >= 0) {
          comandoTCP = c_estadoRiego;
        } else if (buffer.indexOf(temp) >= 0) {
          comandoTCP = temp;
        } else if (buffer.indexOf(c_dia) >= 0) {
          comandoTCP = c_dia;
        } else if (buffer.indexOf(c_mes) >= 0) {
          comandoTCP = c_mes;
        } else if (buffer.indexOf(c_ano) >= 0) {
          comandoTCP = c_ano;
        }
        if (comandoTCP == "")
          respuesta = "<response><ack>0</ack><estado>Sin Comando</estado></response>\n";
        else
          respuesta = tratarComandoTCP(comandoTCP);
        clienteTCP.println(respuesta);
        if (debug) Serial.println("Escribo respuesta: " + respuesta);
        clienteTCP.stop();
        if (debug) {
          Serial.println("Client disconnected.");
        }
      }
    }
  }
}

String tratarComandoTCP(String comandoTCP) {
  // Respondemos y cerramos la conexión
  estado = "";
  if (debug) Serial.println("comandoTCP:" + comandoTCP);
  if (comandoTCP.equals(c_leds) || comandoTCP.equals(c_tira) || comandoTCP.equals(c_estadoAcuario)) {
    if (debug) Serial.println("entro en Acuario");
    ack = "1";
    char c = llamarAcuario(comandoTCP);
    if (c == '0')
      estado = "Los dos Apagados";
    else if (c == '1')
      estado = "Leds Encendidos";
    else if (c == '2')
      estado = "Tira Encendida";
    else if (c == '3')
      estado = "Los dos Encendidos";
    else if (c == 'e')
      estado = "Error conexión acuario";
    else
      estado = "Error en evaluación luces";
  }
  else if (comandoTCP.equals(iniciaRiego) || comandoTCP.equals(izquierda) || comandoTCP.equals(derecha) || comandoTCP.equals(apaga)  || comandoTCP.equals(c_estadoRiego)) {
    estado = llamarRiego(comandoTCP);
    if (debug) Serial.println("llamarRiego devuelve:" + estado);
  }
  else if (comandoTCP.equals(temp) || comandoTCP.equals(c_dia) || comandoTCP.equals(c_mes) || comandoTCP.equals(c_ano)) {
    estado = llamarClima(comandoTCP);
    if (debug) Serial.println("llamarClima devuelve:" + estado);
  }
  else {
    if (debug) Serial.println("entro en comando desconocido");
    ack = "0";
    estado = "Comando desconocido";;
  }
  buffer = "";
  return ("<response><ack>" + ack + "</ack><estado>" + estado + "</estado></response>\n");
}


void loop2(void *par) {
  //  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  //  TIMERG0.wdt_feed = 1;
  //  TIMERG0.wdt_wprotect = 0;
  //  esp_task_wdt_delete(t);
  disableCore0WDT();
  for (;;) {
    serverWEB.handleClient();
  }
}

/*
   Metodos WEB
*/
void handleRoot() {
  if (debug) {
    Serial.print("Servidor WEB corre en core:");
    Serial.println(xPortGetCoreID());
    Serial.println(xPortGetFreeHeapSize());
  }
  estado = "";
  botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Led</button></a></p>";
  botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Tira</button></a></p>";
  botonZ1 = "<p><a href='/IniciarZ1'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
  botonZ2 = "<p><a href='/IniciarZ2'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
  botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  //seleccion = "<p><select name=\"select\"><option value=\"5\">5 Minutos</option><option value=\"10\">10 Minutos</option><option value=\"15\">15 Minutos</option></select></p>";
  char c = llamarAcuario("EstadoAcuario");
  if (c == '0')
    estado = "Los dos Apagados";
  else if (c == '1')
    estado = "Leds Encendidos";
  else if (c == '2')
    estado = "Tira Encendida";
  else if (c == '3')
    estado = "Los dos Encendidos";
  else if (c == 'e')
    estado = "Error conexión acuario";
  else
    estado = "Error en evaluación luces";
  tratarRespuesta();
  estado = llamarRiego("EstadoRiego");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
  if (debug) {
    Serial.print("Servidor WEB corre en core:");
    Serial.println(xPortGetCoreID());
    Serial.println(xPortGetFreeHeapSize());
  }
}
String pagina() {
  //TODO analizar todos los posibles estados y recargar variables boton.
  String estadoClima = llamarClima("Temperatura");
  String datosTemperatura = "";
  String datosHumedad = "";
  String graficaTemperatura = "";
  String graficaHumedad = "";
  if (estadoClima.indexOf("</temperatura>")) {
    int posIni = estadoClima.indexOf("<temperatura>");
    int posFin = estadoClima.indexOf("</temperatura>");
    datosTemperatura = estadoClima.substring(posIni + 13, posFin);
  }
  if ( estado.equals("Failed to open file for reading")) {
    error = "<p style='text-color:red'>Comando Desconocido</p>";
  } else {
    graficaTemperatura =
      "<script type=\"text/javascript\">"
      "var chart1;"
      "$(document).ready(function(){"
      "chart1 = new Highcharts.Chart(\'container1\', {"
      "   title: {"
      "       text: \'Gráfica temperatura Garrapinillos\'"
      "},"
      "yAxis: {"
      "  title: {"
      "      text: \'Temperatura (Cº)\'"
      "  }"
      "},"
      "xAxis: {"
      "  title: {"
      "      text: \'Minutos\'"
      "  }"
      "},"
      "series: [{data: [" + datosTemperatura + "]}]"      " });"
      "});"
      "</script>"
      "<div id=\"container1\" style=\"width: 600px;margin-top: 20px; float:left; height: 400px \"></div>";
  }
  if (estadoClima.indexOf("</humedad>")) {
    int posIni = estadoClima.indexOf("<humedad>");
    int posFin = estadoClima.indexOf("</humedad>");
    datosHumedad = estadoClima.substring(posIni + 13, posFin);
  }
  if ( estado.equals("Failed to open file for reading")) {
    error = "<p style='text-color:red'>Comando Desconocido</p>";
  } else {
    graficaHumedad =
      "<script type=\"text/javascript\">"
      "var chart2;"
      "$(document).ready(function(){"
      "chart2 = new Highcharts.Chart(\'container2\', {"
      "   title: {"
      "       text: \'Gráfica Humedad última Garrapinillos\'"
      "},"
      "yAxis: {"
      "  title: {"
      "      text: \'Humedad (%)\'"
      "  }"
      "},"
      "xAxis: {"
      "  title: {"
      "      text: \'Minutos\'"
      "  }"
      "},"
      "series: [{data: [" + datosHumedad + "]}]"      " });"
      "});"
      "</script>"
      "<div id=\"container2\" style=\"width: 600px;float:right;margin-top:20px; height: 400px \"></div>";
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
         + botonZ1 + botonZ2 + botonIniciarPrograma + seleccion +
         "</div>"
         "<div style='border-style: groove; border-color:orange margin:20px;'>"
         "<h1>Clima</h1>"
         + graficaTemperatura + graficaHumedad +
         "</div>"
         "</div>"
         "</center>"
         "</body>"
         "</html>";
}

//comandos riego
void comandoEstadoRiego () {
  estado = llamarRiego("EstadoRiego");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void riegoZ1() {
  estado = llamarRiego("IniciarZ1");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void riegoZ2 () {
  estado = llamarRiego("IniciarZ2");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void iniciarPrograma () {
  estado = llamarRiego("IniciarPrograma");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void apagarTodo() {
  estado = llamarRiego("ApagarTodo");
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
//Comandos acuario
void comandoEstadoAcuario() {
  char c = llamarAcuario("EstadoAcuario");
  if (c == '0')
    estado = "Los dos Apagados";
  else if (c == '1')
    estado = "Leds Encendidos";
  else if (c == '2')
    estado = "Tira Encendida";
  else if (c == '3')
    estado = "Los dos Encendidos";
  else if (c == 'e')
    estado = "Error conexión acuario";
  else
    estado = "Error en evaluación luces";
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void tira() {
  char c = llamarAcuario("TIRA");
  if (c == '0')
    estado = "Los dos Apagados";
  else if (c == '1')
    estado = "Leds Encendidos";
  else if (c == '2')
    estado = "Tira Encendida";
  else if (c == '3')
    estado = "Los dos Encendidos";
  else if (c == 'e')
    estado = "Error conexión acuario";
  else
    estado = "Error en evaluación luces";
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
void leds() {
  char c = llamarAcuario("LEDS");
  if (c == '0')
    estado = "Los dos Apagados";
  else if (c == '1')
    estado = "Leds Encendidos";
  else if (c == '2')
    estado = "Tira Encendida";
  else if (c == '3')
    estado = "Los dos Encendidos";
  else if (c == 'e')
    estado = "Error conexión acuario";
  else
    estado = "Error en evaluación luces";
  tratarRespuesta();
  paginaHtml = pagina();
  serverWEB.send(200, "text/html", paginaHtml);
}
//ComandosClima


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += serverWEB.uri();
  message += "\nMethod: ";
  message += (serverWEB.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverWEB.args();
  message += "\n";
  for (uint8_t i = 0; i < serverWEB.args(); i++) {
    message += " " + serverWEB.argName(i) + ": " + serverWEB.arg(i) + "\n";
  }
  serverWEB.send(404, "text/plain", message);
}
void tratarRespuesta() {
  Serial.println("El estado es " + estado);

  if (estado.equals("Los dos Apagados")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Tira</button></a></p>";
  } else if ( estado.equals("Leds Encendidos")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Tira</button></a></p>";
  } else if ( estado.equals("Tira Encendida")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: red'>Encender Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Tira</button></a></p>";
  } else if ( estado.equals("Los dos Encendidos")) {
    botonLed = "<p><a href='/leds'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Led</button></a></p>";
    botonTira = "<p><a href='/tira'><button style='height:auto;width:auto;FONT-SIZE: 32pt;background-color: green'>Apagar Tira</button></a></p>";
  } else if ( estado.equals("Error conexión acuario")) {
    error = "<p style='text-color:red'>Error conexión acuario</p>";
  } else if ( estado.equals("Error en evaluación luces")) {
    error = "<p style='text-color:red'>Error en evaluación luces</p>";
  } else if ( estado.equals("Error conexión Servidor Riego")) {
    error = "<p style='text-color:red'>Error conexión Servidor Riego</p>";
  } else if ( estado.equals("Riego Apagado")) {
    Serial.println("Entro aqui");
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estado.equals("Lado Izquierdo riego Encendido")) {
    botonZ1 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estado.equals("Lado Derecho riego Encendido")) {
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estado.equals("Programación Lado Derecho")) {
    botonZ1 = "<p><a href='/IniciarZ1'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estado.equals("Programación Lado Izquierdo")) {
    botonZ1 = "<p><a href='/ApagarTodo'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Apagar Zona Izquierda</button></a></p>";
    botonZ2 = "<p><a href='/IniciarZ2'><button style='background-color:red;height:auto;width:auto;FONT-SIZE: 32pt'>Encender Zona Derecha</button></a></p>";
    botonIniciarPrograma = "<p><a href='/IniciarPrograma'><button style='background-color:green;height:auto;width:auto;FONT-SIZE: 32pt'>Iniciar Programa</button></a></p>";
  } else if ( estado.equals("Comando Desconocido") ) {
    error = "<p style='text-color:red'>Comando Desconocido</p>";
  } else if (estado.equals("Error conexión Servidor Clima")) {
    error = "<p style='text-color:red'>Error conexión Servidor Clima</p>";
  }
  estado = "";
}
/*
   Fin metodos WEB
*/

char llamarAcuario(String comando) {
  if (comando.equals(c_leds)) enciendeLeds();
  else if (comando.equals(c_tira)) enciendeTira();
  return pideEstadoAcuario();
}

void enciendeLeds() {
  WiFiClient clienteAcuario;
  if (debug) Serial.println("***LEDS***");

  char estado_ant = pideEstadoAcuario();
  char estado_recibido = ' ';

  if (debug) {
    Serial.print("Estado inicial Acuario :-->");
    Serial.println(estado_ant);
  }

  long tiempo = 0;
  while ((((estado_ant == '0' || estado_ant == '2') && (estado_recibido != '1' && estado_recibido != '3')) ||
          ((estado_ant == '1' || estado_ant == '3') && (estado_recibido != '0' && estado_recibido != '2'))) &&
         tiempo < TIMEOUT)
  {

    if (!clienteAcuario.connect(ip_acuario, 1996)) {
      if (debug) Serial.println("connection failed");
      return;
    }
    else {
      clienteAcuario.print('1');
      bool primeravez = true;
      while (clienteAcuario.available() == 0 && clienteAcuario.connected()) {
        if (primeravez) if (debug) Serial.println("[Esperando ...]");
        primeravez = false;
      }
      while (clienteAcuario.available()) {
        if (debug) Serial.println("clienteAcuario available");
        //String line = clienteAcuario.readStringUntil('\n');
        estado_recibido = clienteAcuario.read();
        if (debug) {
          Serial.print("     Estado inicial:-->");
          Serial.print(estado_ant);
          Serial.print("     Estado recibido:-->");
          Serial.println(estado_recibido);
        }
      }
      clienteAcuario.stop();
    }
    tiempo++;
  }
}

void enciendeTira() {
  WiFiClient clienteAcuario;
  if (debug) Serial.println("***TIRA***");

  char estado_ant = pideEstadoAcuario();
  char estado_recibido = ' ';

  if (debug) {
    Serial.print("     Estado inicial:-->");
    Serial.println(estado_ant);
  }

  long tiempo = 0;
  while ((((estado_ant == '0' || estado_ant == '1') && (estado_recibido != '2' && estado_recibido != '3')) ||
          ((estado_ant == '2' || estado_ant == '3') && (estado_recibido != '0' && estado_recibido != '1'))) &&
         tiempo < TIMEOUT)
  {

    if (!clienteAcuario.connect(ip_acuario, 1996)) {
      if (debug) Serial.println("connection failed");
      return;
    }
    else {
      clienteAcuario.print('2');
      bool primeravez = true;
      while (clienteAcuario.available() == 0 && clienteAcuario.connected()) {
        if (primeravez) if (debug) Serial.println("[Esperando ...]");
        primeravez = false;
      }
      while (clienteAcuario.available()) {
        if (debug) Serial.println("clienteAcuario available");
        //String line = clienteAcuario.readStringUntil('\n');
        estado_recibido = clienteAcuario.read();
        if (debug) {
          Serial.print("     Estado inicial:-->");
          Serial.print(estado_ant);
          Serial.print("     Estado recibido:-->");
          Serial.println(estado_recibido);
        }
      }
      clienteAcuario.stop();
    }
    tiempo++;
  }
}

char pideEstadoAcuario() {
  WiFiClient clienteAcuario;
  char c = ' ';
  if (debug) Serial.println("***Entro en estado Acuario***");
  if (!clienteAcuario.connect(ip_acuario, 1996)) {
    if (debug)Serial.println("connection failed");
    return 'e';
  }
  else {
    clienteAcuario.print('0');
    bool primeravez = true;
    while (clienteAcuario.available() == 0 && clienteAcuario.connected()) {
      if (primeravez) if (debug) Serial.println("[Esperando ...]");
      primeravez = false;
    }
    while (clienteAcuario.available()) {
      if (debug) Serial.println("clienteAcuario available");
      //String line = clienteAcuario.readStringUntil('\n');
      c = clienteAcuario.read();
      if (debug) Serial.println(c);
    }
    clienteAcuario.stop();
  }
  return c;
}

String llamarRiego(String comando) {
  if (debug) Serial.println("entro en Riego");
  WiFiClient clienteTCP;
  buffer = "";
  if (!clienteTCP.connect(ip_riego, 1996)) {
    if (debug) Serial.println("Error conexión Servidor Riego");
    ack = "0";
    return "Error conexión Servidor Riego";
  }
  else {
    String envio = "<request><comando>" + comando + "</comando></request>";
    clienteTCP.println(envio);
    if (debug) Serial.println("Mando a riego : " + envio);
    while (clienteTCP.connected()) {                            // loop mientras el cliente está conectado
      if (clienteTCP.available()) {                             // si hay bytes para leer desde el cliente
        buffer = clienteTCP.readStringUntil('\n');                             // lee un byte
        comando = "";                                       // Blanqueamos el comando
        ack = "0";
        estado = "";
        if (debug) Serial.println("Recibo n de riego. El mensaje es: " + buffer);
        String respuesta = "";
        int posi = buffer.indexOf("<ack>");
        int posf = buffer.indexOf("</ack>");
        if (posi == 0 || posf == 0) {
          ack = "0";
          if (debug) Serial.println("posiciones error");
          return "Error recibiendo respuesta Servidor Riego";
        }
        else {
          if (debug) Serial.println("Devuelvo ack de riego = " + buffer.substring(posi + 5, posf ));
          ack = buffer.substring(posi + 5, posf );
          posi = buffer.indexOf("<estado>");
          posf = buffer.indexOf("</estado>");
          if (posi == 0 || posf == 0) {
            ack = "0";
            if (debug) Serial.println("posiciones error");
            return "No se recibe causa de error";
          }
          else {
            if (debug) Serial.println("Devuelvo estado de riego = " + buffer.substring(posi + 8, posf ));
            return buffer.substring(posi + 8, posf );
          }
        }
      }
    }
  }
}

String llamarClima(String comando) {
  if (debug) Serial.println("entro en Clima");
  WiFiClient clienteTCP;
  buffer = "";
  if (!clienteTCP.connect(ip_temperatura, 1996)) {
    if (debug) Serial.println("Error conexión Servidor clima");
    ack = "0";
    return "Error conexión Servidor Clima";
  }
  else {
    String envio = "<request><comando>" + comando + "</comando></request>";
    clienteTCP.println(envio);
    if (debug) Serial.println("Mando a clima : " + envio);
    while (clienteTCP.connected()) {                            // loop mientras el cliente está conectado
      if (clienteTCP.available()) {                             // si hay bytes para leer desde el cliente
        buffer = clienteTCP.readStringUntil('\n');                             // lee un byte
        comando = "";                                       // Blanqueamos el comando
        ack = "0";
        estado = "";
        if (debug) Serial.println("Recibo n de clima. El mensaje es: " + buffer);
        String respuesta = "";
        int posi = buffer.indexOf("<ack>");
        int posf = buffer.indexOf("</ack>");
        if (posi == 0 || posf == 0) {
          ack = "0";
          if (debug) Serial.println("posiciones error");
          return "Error recibiendo respuesta Servidor Clima";
        }
        else {
          if (debug) Serial.println("Devuelvo ack de riego = " + buffer.substring(posi + 5, posf ));
          ack = buffer.substring(posi + 5, posf );
          posi = buffer.indexOf("<estado>");
          posf = buffer.indexOf("</estado>");
          if (posi == 0 || posf == 0) {
            ack = "0";
            if (debug) Serial.println("posiciones error");
            return "No se recibe causa de error";
          }
          else {
            if (debug) Serial.println("Devuelvo estado de clima = " + buffer.substring(posi + 8, posf ));
            return buffer.substring(posi + 8, posf );
          }
        }
      }
    }
  }
}

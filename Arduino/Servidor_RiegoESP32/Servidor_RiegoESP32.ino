#include <WiFi.h>


const char* ssid = "Router-Salon";
const char* password = "1234567890123";

bool debug = true;
int contconexion = 0;

const long TIMEOUT = 10;

String buffer = "";                                       // Variable para guardar el request
String comando = "";                                      // Variable para guardar el comando leido

WiFiServer serverTCP(1996);

TaskHandle_t t;
int minutos = 0;
int lado = 0;
int estado = 0;
/*
   Valores de estado
   0 - Apagado
   1 - Izquierdo
   2 - Derecho
   3 - Programación
*/
int eval_Derecha_1 = 26;
int eval_Derecha_2 = 27;
int eval_Izquierda_1 = 25;
int eval_Izquierda_2 = 33;

String iniciaRiego = "IniciarPrograma";
String izquierda = "IniciarZ1";
String derecha = "IniciarZ2";
String apaga = "ApagarTodo";
String estadoRiego = "EstadoRiego";

const long TIEMPORIEGO = 5000;                    // sugerencia 1 minuto -> 60000
long contador = 0;
void setup() {
  if (debug) {
    Serial.begin(115200);
    Serial.println("");
  }

  pinMode(eval_Derecha_1, OUTPUT);
  pinMode(eval_Derecha_2, OUTPUT);
  pinMode(eval_Izquierda_1, OUTPUT);
  pinMode(eval_Izquierda_2, OUTPUT);

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
      Serial.println(WiFi.macAddress());
      delay(10000);
    }
    serverTCP.begin(); // iniciamos el servidor TCP
  }
  else if (debug) {
    Serial.println("");
    Serial.println("Error de conexion");
  }
}
void loop2(void *par)
{
  //  if (debug) Serial.println("Empieza tarea 2 en core " + String(xPortGetCoreID()));
  //  if (debug) Serial.println("Empieza Programa");
  int tiempo = *((int*)par);
  unsigned long duracion = tiempo * 60000;

  if (debug) Serial.println("-->Empieza Programa encender derecha");

  lado = 1;
  encenderDerecha();
  delay (duracion);
  lado = 2;
  apagarDerecha();
  encenderIzquierda();
  if (debug) Serial.println("--> Zona 2");
  delay (duracion);
  apagarIzquierda();
  estado = 0;
  vTaskDelete(NULL);
}


void loop() {
  WiFiClient clienteTCP = serverTCP.available();                // Escucha a los clientes entrantes

  if (clienteTCP) {                                             // Si se conecta un nuevo cliente
    if (debug) Serial.println("[Cliente conectado]");           //
    while (clienteTCP.connected()) {                            // loop mientras el cliente está conectado
      if (clienteTCP.available()) {                             // si hay bytes para leer desde el cliente
        buffer = clienteTCP.readStringUntil('\n');                             // lee un byte
        comando = "";                                         // Blanqueamos el comando
        if (debug) Serial.println(buffer);

        String respuesta = "";
        int posi = buffer.indexOf("<comando>");
        int posf = buffer.indexOf("</comando>");
        int ack = 0;
        if (posi == 0 || posf == 0) {
          if (debug) Serial.println("posiciones error");
        }
        else {
          comando = buffer.substring(posi + 9, posf );
          if (debug) Serial.println(comando);

          if (comando == iniciaRiego) {
            /*
               Calculo minutos
            */
            posi = buffer.indexOf("<tiempo>");
            posf = buffer.indexOf("</tiempo>");
            if (posi == 0 || posf == 0) {
              minutos = buffer.substring(posi + 8, posf).toInt();
            }
            else
              minutos = 1;
            if (debug) Serial.println("entro en Iniciar Riego");
            contador = 0;
            estado = 3;
            lado = 0;
            ack = 1;
            minutos = 1;
            apagarDerecha();
            apagarIzquierda();
            xTaskCreate(
              loop2,
              "tarea 0",
              10000,
              (void*)&minutos,
              1,
              &t);

          } else if (comando == izquierda) {
            if (debug) Serial.println("entro en IZQUIERDO");
            if (estado == 3) vTaskDelete(t);
            apagarDerecha();
            encenderIzquierda();
            estado = 1;
            ack = 1;
          } else if (comando == derecha) {
            if (debug) Serial.println("entro en DERECHO");
            if (estado == 3) vTaskDelete(t);
            apagarIzquierda();
            encenderDerecha();
            estado = 2;
            ack = 1;
          }
          else if (comando == apaga) {
            if (debug) Serial.println("entro en APAGAR");
            if (estado == 3) vTaskDelete(t);
            apagarDerecha();
            apagarIzquierda();
            estado = 0;
            ack = 1;
          }
          else if (comando == estadoRiego) {
            if (debug) Serial.println("entro en estadoRiego");
            ack = 1;
          }
          else {
            if (debug) Serial.println("entro en comando desconocido");
          }
        }
        respuesta = "<response><ack>" + String(ack) + "</ack><estado>" + getEstado() + "</estado></response>\n";
        clienteTCP.println(respuesta);
        if (debug) Serial.println(respuesta);
        break;
      }
    }
    buffer = "";                            // Limpiamos la variable buffer
    clienteTCP.stop();                      // Cerramos la conexión
    //  if (debug) Serial.println("Client disconnected.");
  }
  //  if (estado == 3) ]
}

String getEstado() {
  if (debug) Serial.println("Estado:" + String(estado));
  if (estado == 0) return "Riego Apagado";
  else if (estado == 1) return "Lado Izquierdo riego Encendido";
  else if (estado == 2) return "Lado Derecho riego Encendido";
  else if (estado == 3 && (lado == 1 || lado == 0)) return "Programación Lado Derecho";
  else if (estado == 3 && lado == 2) return "Programación Lado Izquierdo";
  else return "Estado Desconocido";
}

void encenderDerecha() {
  if (debug) Serial.println("entro en encender derecha");
  digitalWrite(eval_Derecha_1, HIGH);
  digitalWrite(eval_Derecha_2, LOW);

  delay(500);

  digitalWrite(eval_Derecha_1, LOW);
  digitalWrite(eval_Derecha_2, LOW);
}
void apagarDerecha() {
  digitalWrite(eval_Derecha_1, LOW);
  digitalWrite(eval_Derecha_2, HIGH);

  delay(500);

  digitalWrite(eval_Derecha_1, LOW);
  digitalWrite(eval_Derecha_2, LOW);
}
void encenderIzquierda() {
  digitalWrite(eval_Izquierda_1, HIGH);
  digitalWrite(eval_Izquierda_2, LOW);

  delay(500);

  digitalWrite(eval_Izquierda_1, LOW);
  digitalWrite(eval_Izquierda_2, LOW);
}
void apagarIzquierda() {
  digitalWrite(eval_Izquierda_1, LOW);
  digitalWrite(eval_Izquierda_2, HIGH);

  delay(500);

  digitalWrite(eval_Izquierda_1, LOW);
  digitalWrite(eval_Izquierda_2, LOW);
}

/*
   Connect the SD card to the following pins:

     ESP32  | SD Card  | Nuestro ESP32
      D2       -
      D3       SS CS        G5
      CMD      MOSI         G23
      VSS      GND          GND
      VDD      3.3V         3V3
      CLK      SCK          G18
      VSS      GND
      D0       MISO         G19
      D1       -
*/
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <DHT.h>
#include <WiFi.h>
#include "time.h"
#include <LiquidCrystal.h>
DHT sensorDht(26, DHT22);
WiFiServer climaServer(1996);
LiquidCrystal lcd(15, 13, 32, 33, 25, 27);

TaskHandle_t t;
TaskHandle_t t2;

String root_dir = "/Temperatura";
String ano_dir = "";
String mes_dir = "";
String dia_dir = "";
String hora_file = "/25";
String minuto = "";
float valorTemperatura = 0;
float valorHumedad = 0;
String texto = "Temperatura ";
String texto2 = "Humedad ";
const char* ssid = "Router-Salon";
const char* password = "1234567890123";
float humidity, temperature;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
String buffer = "";
String comando = "";
String estado = "";
bool debug = true;
/*
   Comandos
*/

String temp = "Temperatura";
String c_dia = "TempDia";
String c_mes = "TempMes";
String c_ano = "TempAno";

//bool semaforo = true;


char timeStringBuff[200]; //50 chars should be enough
String hora_actual = "";
String minuto_actual = "";

void setup() {
  Serial.begin(115200);
  sensorDht.begin();
  lcd.begin(16, 2);
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  Serial.println(WiFi.localIP());
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  climaServer.begin();


  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }



  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  //listarDir("/", 10);

  File temperature_dir = SD.open(root_dir);
  if (!temperature_dir) {
    if (debug)Serial.println("No encontrado directorio" + root_dir + " \n Voy a crearlo");
    crearDir(root_dir);
  }
  //if (debug)Serial.println("Despues");
  //listarDir("/", 10);

  xTaskCreatePinnedToCore(
    loop2,
    "tarea 0",
    20000,
    NULL,
    1,
    &t,
    0);
  delay(500);

}
void loop() {
  escucharCliente();
}

//void loop2() {
void loop2(void *par) {
  //  delay(500);
  struct tm timeinfo;
  for (;;) {
    if (debug) Serial.println("0-->Bucle loop2");
    if (!getLocalTime(&timeinfo)) {
      if (debug)Serial.println("Failed to obtain time");
    }

    adjustTimeFolders(timeinfo);
    tomarValor(timeinfo);
    //    tomarValor();
    digitalWrite(14, HIGH);
    for (int x = 1; x < 40; x++) {
      lcd.setCursor(15, 0);
      lcd.print(texto);
      lcd.print(valorTemperatura);
      lcd.print(" C");
      lcd.setCursor(15, 1);
      lcd.print(texto2);
      lcd.print(valorHumedad);
      lcd.print(" %");
      lcd.scrollDisplayLeft();
      delay(400);
    }
    lcd.clear();
    digitalWrite(14, LOW);
    if (debug) Serial.println("0-->Espero 30000");
    delay(34000); //34Secs + LCDScroll 400*40=16Secs
    if (debug) Serial.println("0-->Salgo espero 30000");

  }
}
void escucharCliente() {
  //  if (debug) Serial.println("1-->Available.");

  WiFiClient clienteTCP = climaServer.available();                // Escucha a los clientes entrantes

  if (clienteTCP) {                                             // Si se conecta un nuevo cliente
    if (debug) Serial.println("[Cliente conectado]");           //
    while (clienteTCP.connected()) {                            // loop mientras el cliente está conectado
      if (clienteTCP.available()) {                             // si hay bytes para leer desde el cliente
        buffer = clienteTCP.readStringUntil('\n');                             // lee un byte
        comando = "";                                         // Blanqueamos el comando
        estado = "";
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

          if (comando.equals(temp)) {
            ack = 1;
            estado = leerTxt(root_dir + ano_dir + mes_dir + dia_dir + hora_file + ".txt");

          } else if (comando.equals(c_dia)) {
            ack = 1;
            estado = leerTxt(root_dir + ano_dir + mes_dir + dia_dir + ".med");

          } else if (comando.equals(c_mes)) {
            ack = 1;
            estado = leerTxt(root_dir + ano_dir + mes_dir + ".med");
          } else if (comando.equals(c_ano)) {
            ack = 1;
            estado = leerTxt(root_dir + ano_dir + ".med");
          }
          else {
            if (debug) Serial.println("entro en comando desconocido");
          }
        }
        respuesta = "<response><ack>" + String(ack) + "</ack><estado>" + estado + "</estado></response>\n";
        clienteTCP.println(respuesta);
        if (debug) Serial.println(respuesta);
        break;
      }
    }
    buffer = "";                            // Limpiamos la variable buffer
    clienteTCP.stop();                      // Cerramos la conexión
    if (debug) Serial.println("1-->Client disconnected.");
  }
}

void tomarValor(struct tm timeinfo) {
  //struct tm timeinfo;
  if (debug) Serial.println("Tras struct");
  hora_actual = "";
  minuto_actual = "";
  //  if (!getLocalTime(&timeinfo)) {
  //    if (debug)Serial.println("Failed to obtain time");
  //  }
  if (debug) Serial.println("Tras obtener time en tomar valor");
  if (debug)Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //char timeStringBuff[200];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H", &timeinfo);
  String numero_hora_actual = String (timeStringBuff);
  hora_actual = "/" + String (timeStringBuff);
  strftime(timeStringBuff, sizeof(timeStringBuff), "%M", &timeinfo);
  minuto_actual = String (timeStringBuff);
  if (hora_actual != hora_file) {
    hora_file = hora_actual;
  }
  if (minuto_actual != minuto) {
    minuto = minuto_actual;
    //    File fichero = SD.open(root_dir + ano_dir + mes_dir + dia_dir + hora_file + ".txt", FILE_WRITE);
    if (debug) Serial.println("Escribo minuto-->" + minuto + " en el fichero--> " + root_dir + ano_dir + mes_dir + dia_dir + hora_file + ".txt");
    valorTemperatura = sensorDht.readTemperature(false);
    valorHumedad = sensorDht.readHumidity();
    while (isnan(valorHumedad)) {
      valorHumedad = sensorDht.readHumidity();
    }
    while (isnan(valorTemperatura)) {
      valorTemperatura = sensorDht.readTemperature(false);
    }
    String valor = minuto + ";" + valorTemperatura + ";" + valorHumedad;
    if (debug) Serial.println(valor);
    int longitud = valor.length() + 1;
    char valor_char [longitud];
    valor.toCharArray(valor_char, longitud);
    appendFile(root_dir + ano_dir + mes_dir + dia_dir + hora_file + ".txt", valor);
    actualizarMedias(numero_hora_actual);

  }
}
void actualizarMedias(String hora_actual) {
  String media_hora = mediaHora(hora_actual);
}

String mediaHora(String hora_actual) {
  File file = SD.open(root_dir + ano_dir + mes_dir + dia_dir + hora_file + ".txt");
  if (!file) {
    return ("Failed to open file for reading");
  }
  String salida = "";
  char c;
  while (file.available()) {
    c = file.read();
    salida += c;
  }
  file.close();
  float temperaturas [60];
  float humedades [60];

  for (int i = 0 ; i < 60; i++) {
    temperaturas[i] = -273;
    humedades[i] = -273;
  }

  int puntero = salida.indexOf('\n');
  int punteroant = 0;
  String minuto, temperatura, humedad = "";
  while (puntero >= 0) {
    if (debug) Serial.print("Puntero=");
    if (debug) Serial.println(puntero);
    String linea = salida.substring(punteroant, puntero);
    salida = salida.substring(puntero + 1);
    puntero = salida.indexOf('\n');

    minuto = linea.substring(0, 2);
    temperatura = linea.substring(3, 8);
    humedad = linea.substring(9, 14);
    if (debug) Serial.println("Minuto=" + minuto);
    temperaturas[minuto.toInt()] = temperatura.toFloat();
    humedades[minuto.toInt()] = humedad.toFloat();
  }
  float media_temperatura = 0;
  float media_humedad = 0;
  int cantidad_valores = 0;
  for (int i = 0; i < 60; i++) {
    if (temperaturas[i] > -273) {
      cantidad_valores++;
      media_temperatura += temperaturas[i];
      media_humedad += humedades[i];
    }
  }
  Serial.print("Numero de elementos hora");
  Serial.println(cantidad_valores);
  media_temperatura = media_temperatura / cantidad_valores;
  media_humedad = media_humedad / cantidad_valores;
  Serial.print("media humedad");
  Serial.println(media_humedad);
  Serial.print("media temperatura");
  Serial.println(media_temperatura);

  String valor = hora_actual + ";" + media_temperatura + ";" + media_humedad;
  if (debug) Serial.println(valor);
  int longitud = valor.length() + 1;
  char valor_char [longitud];
  valor.toCharArray(valor_char, longitud);

  writeDia(valor);
  return ("Fuera....");
}
void writeDia(String message) {
  if (debug) Serial.print("Entrade en WriteDia=" + message);
  String path = root_dir + ano_dir + mes_dir + dia_dir + ".med";
  float temperaturas[24];
  float humedades[24];
  String horas[24];
  for (int i = 0 ; i < 24; i++) {
    temperaturas[i] = -273;
    humedades[i] = -273;
  }
  Serial.println("writing to file: " + path);
  String salida;
  String hora, temperatura, humedad = "";
  File file = SD.open(path, FILE_READ);
  if (!file) {
    if (debug) Serial.print("Sin Fichero");
  }
  else
  {
    salida = "";
    char c;
    while (file.available()) {
      c = file.read();
      salida += c;
    }
    file.close();

    int puntero = salida.indexOf('\n');
    int punteroant = 0;

    while (puntero >= 0) {
      String linea = salida.substring(punteroant, puntero);
      salida = salida.substring(puntero + 1);
      puntero = salida.indexOf('\n');
      hora = linea.substring(0, 2);
      Serial.println("linea" + linea + " hora" + hora);
      horas[hora.toInt()] = hora;
      temperatura = linea.substring(3, 8);
      humedad = linea.substring(9, 14);
      temperaturas[hora.toInt()] = temperatura.toFloat();
      humedades[hora.toInt()] = humedad.toFloat();
    }
  }
  float media_temperatura = 0;
  float media_humedad = 0;
  int cantidad_valores = 0;

  hora = message.substring(0, 2);
  Serial.println("Hora del mensaje leido->" + hora + "<-");
  temperatura = message.substring(3, 8);
  Serial.println("Temperatura del mensaje leido->" + temperatura + "<-");
  humedad = message.substring(9, 14);
  Serial.println("Humedad del mensaje leido->" + humedad + "<-");
  temperaturas[hora.toInt()] = temperatura.toFloat();
  humedades[hora.toInt()] = humedad.toFloat();
  horas[hora.toInt()] = hora;
  file = SD.open(path, FILE_WRITE);
  for (int i = 0; i < 24; i++) {
    Serial.println(horas[i]);
    if (temperaturas[i] > -273) {
      cantidad_valores++;
      media_temperatura += temperaturas[i];
      media_humedad += humedades[i];
      file.println(horas[i] + ";" + temperaturas[i] + ";" + humedades[i]);

    }
  }
  file.close();

  Serial.print("Numero de elementos");
  Serial.println(cantidad_valores);
  media_temperatura = media_temperatura / cantidad_valores;
  media_humedad = media_humedad / cantidad_valores;

  String valor = dia_dir.substring(1) + ";" + media_temperatura + ";" + media_humedad;
  if (debug) Serial.println(valor);
  int longitud = valor.length() + 1;
  char valor_char [longitud];
  valor.toCharArray(valor_char, longitud);

  actualizarMes(valor);
}

void actualizarMes(String message) {

  if (debug) Serial.print("Entrada en actualizarMes=" + message);
  String path = root_dir + ano_dir + mes_dir + ".med";
  float temperaturas[31];
  float humedades[31];
  String dias[31];
  for (int i = 0 ; i < 31; i++) {
    temperaturas[i] = -273;
    humedades[i] = -273;
  }
  Serial.println("writing to file: " + path);
  String salida;
  String dia, temperatura, humedad = "";
  File file = SD.open(path, FILE_READ);
  if (!file) {
    if (debug) Serial.print("Sin Fichero");
  }
  else
  {
    salida = "";
    char c;
    while (file.available()) {
      c = file.read();
      salida += c;
    }
    file.close();

    int puntero = salida.indexOf('\n');
    int punteroant = 0;

    while (puntero >= 0) {
      String linea = salida.substring(punteroant, puntero);
      salida = salida.substring(puntero + 1);
      puntero = salida.indexOf('\n');
      dia = linea.substring(0, 2);
      Serial.println("linea" + linea + " dia" + dia);
      dias[dia.toInt()] = dia;
      temperatura = linea.substring(3, 8);
      humedad = linea.substring(9, 14);
      temperaturas[dia.toInt()] = temperatura.toFloat();
      humedades[dia.toInt()] = humedad.toFloat();
    }
  }
  float media_temperatura = 0;
  float media_humedad = 0;
  int cantidad_valores = 0;

  dia = message.substring(0, 2);
  Serial.println("Dia del mensaje leido->" + dia + "<-");
  temperatura = message.substring(3, 8);
  Serial.println("Temperatura del mensaje leido->" + temperatura + "<-");
  humedad = message.substring(9, 14);
  Serial.println("Humedad del mensaje leido->" + humedad + "<-");
  temperaturas[dia.toInt()] = temperatura.toFloat();
  humedades[dia.toInt()] = humedad.toFloat();
  dias[dia.toInt()] = dia;
  file = SD.open(path, FILE_WRITE);
  for (int i = 0; i < 31; i++) {
    Serial.println(dias[i]);
    if (temperaturas[i] > -273) {
      cantidad_valores++;
      media_temperatura += temperaturas[i];
      media_humedad += humedades[i];
      file.println(dias[i] + ";" + temperaturas[i] + ";" + humedades[i]);

    }
  }
  file.close();

  Serial.print("Numero de elementos");
  Serial.println(cantidad_valores);
  media_temperatura = media_temperatura / cantidad_valores;
  media_humedad = media_humedad / cantidad_valores;

  String mes_numero = "";
  if (mes_dir.equals("/May")) mes_numero = "05";
  else if (mes_dir.equals("/June")) mes_numero = "06";

  String valor = mes_numero + ";" + media_temperatura + ";" + media_humedad;
  if (debug) Serial.println(valor);
  int longitud = valor.length() + 1;
  char valor_char [longitud];
  valor.toCharArray(valor_char, longitud);
  actualizarAno(valor);
}

void actualizarAno(String message) {

  if (debug) Serial.print("Entrada en actualizarAno=" + message);
  String path = root_dir + ano_dir + ".med";
  float temperaturas[12];
  float humedades[12];
  String meses[12];
  for (int i = 0 ; i < 12; i++) {
    temperaturas[i] = -273;
    humedades[i] = -273;
  }
  Serial.println("writing to file: " + path);
  String salida;
  String mes, temperatura, humedad = "";
  File file = SD.open(path, FILE_READ);
  if (!file) {
    if (debug) Serial.print("Sin Fichero");
  }
  else
  {
    salida = "";
    char c;
    while (file.available()) {
      c = file.read();
      salida += c;
    }
    file.close();

    int puntero = salida.indexOf('\n');
    int punteroant = 0;

    while (puntero >= 0) {
      String linea = salida.substring(punteroant, puntero);
      salida = salida.substring(puntero + 1);
      puntero = salida.indexOf('\n');
      mes = linea.substring(0, 2);
      Serial.println("linea" + linea + " dia" + mes);
      meses[mes.toInt()] = mes;
      temperatura = linea.substring(3, 8);
      humedad = linea.substring(9, 14);
      temperaturas[mes.toInt()] = temperatura.toFloat();
      humedades[mes.toInt()] = humedad.toFloat();
    }
  }
  float media_temperatura = 0;
  float media_humedad = 0;
  int cantidad_valores = 0;

  mes = message.substring(0, 2);
  Serial.println("Mes del mensaje leido->" + mes + "<-");
  temperatura = message.substring(3, 8);
  Serial.println("Temperatura del mensaje leido->" + temperatura + "<-");
  humedad = message.substring(9, 14);
  Serial.println("Humedad del mensaje leido->" + humedad + "<-");
  temperaturas[mes.toInt()] = temperatura.toFloat();
  humedades[mes.toInt()] = humedad.toFloat();
  meses[mes.toInt()] = mes;
  file = SD.open(path, FILE_WRITE);
  for (int i = 0; i < 12; i++) {
    Serial.println(meses[i]);
    if (temperaturas[i] > -273) {
      cantidad_valores++;
      media_temperatura += temperaturas[i];
      media_humedad += humedades[i];
      file.println(meses[i] + ";" + temperaturas[i] + ";" + humedades[i]);

    }
  }
  file.close();

  Serial.print("Numero de elementos");
  Serial.println(cantidad_valores);
  media_temperatura = media_temperatura / cantidad_valores;
  media_humedad = media_humedad / cantidad_valores;
}

void adjustTimeFolders(struct tm timeinfo)
{
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y", &timeinfo); //Saco %Year de timeInfo
  ano_dir = "/" + String(timeStringBuff);
  if (!SD.open(root_dir + ano_dir)) {
    if (debug)Serial.println("No encontrado directorio" + root_dir + ano_dir + " \n Voy a crearlo");
    crearDir(root_dir + ano_dir);
  }
  Serial.println("Directorio año--> " + ano_dir);
  strftime(timeStringBuff, sizeof(timeStringBuff), "%B", &timeinfo);
  mes_dir = "/" + String(timeStringBuff);
  if (!SD.open(root_dir + ano_dir + mes_dir)) {
    if (debug)Serial.println("No encontrado directorio" + root_dir + ano_dir + mes_dir + " \n Voy a crearlo");
    crearDir(root_dir + ano_dir + mes_dir);
  }
  if (debug)Serial.println("Directorio mes--> " + mes_dir);
  strftime(timeStringBuff, sizeof(timeStringBuff), "%d", &timeinfo);
  dia_dir = "/" + String(timeStringBuff);
  if (!SD.open(root_dir + ano_dir + mes_dir + dia_dir)) {
    if (debug)Serial.println("No encontrado directorio" + root_dir + ano_dir + mes_dir + dia_dir + " \n Voy a crearlo");
    crearDir(root_dir + ano_dir + mes_dir + dia_dir);
  }
  if (debug)Serial.println("Directorio dia--> " + dia_dir);

}

void listarDir(String dirname, uint8_t levels) {
  Serial.println("Listing directory: " + dirname);
  File root = SD.open(dirname);
  if (!root) {
    if (debug)Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    if (debug)Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      if (debug)Serial.print("  DIR : ");
      if (debug)Serial.println(file.name());
      if (levels) {
        listarDir(file.name(), levels - 1);
      }
    } else {
      if (debug)Serial.print("  FILE: ");
      if (debug)Serial.print(file.name());
      if (debug)Serial.print("  SIZE: ");
      if (debug)Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
void crearDir(String path) {
  Serial.println("Creating Dir: " + path);
  if (SD.mkdir(path)) {
    if (debug)Serial.println("Dir created");
  } else {
    if (debug)Serial.println("mkdir failed");
  }
}


String leerTxt(String path) {

  File file = SD.open(path);
  if (!file) {
    return ("Failed to open file for reading");
  }
  String salida = "";
  Serial.print("Read from file: ");
  char c;
  while (file.available()) {
    c = file.read();
    salida += c;
  }
  file.close();

  String retorno, titulos, temperaturas, humedades = "";
  //  if (debug) Serial.println("Fichero :" + salida);
  int puntero = salida.indexOf('\n');
  //  if (debug) Serial.println("primer barra n :" + puntero);
  int punteroant = 0;
  String minuto = "";
  while (puntero >= 0) {
    String linea = salida.substring(punteroant, puntero);
    //    if (debug) Serial.println("Linea:" + linea);
    salida = salida.substring(puntero + 1);
    puntero = salida.indexOf('\n');
    //    if (debug) Serial.println("siguiente barra n :" + puntero);
    if (!minuto.equals(linea.substring(0, 2))) {
      minuto = linea.substring(0, 2);
      titulos += minuto;
      //      if (debug) Serial.println("minuto leido :" + minuto);
      temperaturas += linea.substring(3, 8);
      //      if (debug) Serial.println("estado actual :" + retorno);
      humedades += linea.substring(9, 14);
      if (puntero >= 0) // hay mas lineas
      {
        titulos += ',';
        temperaturas += ',';
        humedades += ',';
      }
    }
  }
  retorno = "<leyenda>" + titulos + "</leyenda><temperatura>" + temperaturas + "</temperatura><humedad>" + humedades + "</humedad>";
  return retorno;
}



void appendFile(String path, String message) {
  //Serial.printf("Appending to file: %s\n", path);
  Serial.println("Appending to file: " + path);
  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.println(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

String floatToString( float n, int l, int d, boolean z) {
  char c[l + 1];
  String s;

  dtostrf(n, l, d, c);
  s = String(c);

  if (z) {
    s.replace(" ", "0");
  }

  return s;
}

/*
   Metodos no usados
  void eliminarDir(String path) {
  Serial.println("Removing Dir: " + path);
  if (SD.rmdir(path)) {
    if (debug)Serial.println("Dir removed");
  } else {
    if (debug)Serial.println("rmdir failed");
  }
  }
  void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
  }

  void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
  }
  void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
  }
*/

/*
   dispositivo receptor de alertas.

  Este es un pequeñó programa para
  recibir datos de un broker mqtt
  y emitir una alerta en caso de
  sismo.

  Yeffri J. Salazar
  Comunidad Arduino Guatemala, Xibalba Hackerspace
  1 octubre 2017

  Circuito
  Cualquier placa basada en ESP8266 (nodeMCU, wemosD1, etc)
  buzzer
  Led
  Relay 5v ~110



*/
/************************************
 **              Librerias         **
 ***********************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
/************************************
 **              Constantes        **
 ***********************************/
#define WIFISSID "HoneyPot"
#define WIFIPASSWORD "HoneyNet"
#define TOPICOALERTA "Alertas"
#define ENVIARALERTA "Alertas"
#define TOPICOCONEXION "/conexion"
#define SALIDALED D5
#define SALIDABUZZER D6
#define SALIDARELAY D7
#define PUERTOMQTT 19809
#define NOMBREDEALARMA "Chiquimula"
#define USUARIOMQTT "alertor"
#define CLAVEMQTT "alertor"

/************************************
 **           Variables            **
 ***********************************/
WiFiClient clienteWifi;//este cliente se encarga de la comunicacion con el wifi
PubSubClient clienteMQTT(clienteWifi);//este utiliza el cliente anterior para hacer poder crear la conexion mqtt
const char * brokerMqtt = "m13.cloudmqtt.com";// ip del broker sin http ni nada solo los numeros

uint32_t ultimoIntentoReconexion;
volatile uint32_t tiempoEnvio = 0;


/************************************
 **           funciones            **
 ***********************************/

/*
   Funcion Conectar WIFI
   Esta funcion nos conectara al wifi
*/
void conectarWifi() {
  WiFi.begin(WIFISSID, WIFIPASSWORD);
  Serial.print("conectando a");
  Serial.println(WIFISSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi Conectado ");
  Serial.println("direccion IP: ");
  Serial.println(WiFi.localIP());
}

/*
   Funcion callback
   esta funcion se activa cada vez que recibimos un mensaje
   de el topico al cual estamos suscritos
   recibe los siguientes parametros
   Char * nombreTopico = cadena de caracteres con el topico del cual recibe el mensaje
   Byte * mensajeEntrante = es un arreglo de bytes con al informacion del mensaje
   este podemos pasarlo a char para que sea legible
  unsigned int tamanoMensaje
*/


void callback(char* nombreTopico, byte* mensajeEntrante, unsigned int tamanoMensaje) {
  String topico = nombreTopico;
  String mensaje;
  for (unsigned int i = 0; i < tamanoMensaje; i++) {
    mensaje += (char) mensajeEntrante[i];
  }

  Serial.print("Mensaje Recibido del topico: ");
  Serial.println(topico);
  Serial.print("mensaje : ");
  Serial.println(mensaje);
}
/*
   reconexionMQTT
   Esta funcion nos permitira saber si estamos o no conectados al broker mqtt
   si en caso no lo estuvieramos nos conecta y sino pues siemplemente pasa de largo
   tambien nos suscribimos al topico de alerta para recibir
   las notificaciones
*/

boolean reconexionMQTT() {
  Serial.print("Conectando al broker mqtt");
  //intentando conectar al broker
  if (clienteMQTT.connect(NOMBREDEALARMA, USUARIOMQTT, CLAVEMQTT)) {
    Serial.println("Conectado");
    //publicamos que estamos conectados
    clienteMQTT.publish("/conexion", "Conectado");
    //nos suscribimos a los topicos para controlar las salidas
    clienteMQTT.subscribe(TOPICOALERTA);
  } else {
    Serial.print("falló, rc=");
    Serial.print(clienteMQTT.state());
  }
  return clienteMQTT.connected();
}



boolean sensor() {
  if (millis() - tiempoEnvio > 5000) {
    tiempoEnvio = millis();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("iniciando programa " );
  pinMode(SALIDALED, OUTPUT);
  pinMode(SALIDABUZZER, OUTPUT);
  pinMode(SALIDARELAY, OUTPUT);
  digitalWrite(SALIDALED, LOW);
  digitalWrite(SALIDABUZZER, LOW);
  digitalWrite(SALIDARELAY, LOW);
  conectarWifi();
  clienteMQTT.setServer(brokerMqtt, PUERTOMQTT); //le decimos cual es el servidor y el puerto al que se debe conectar
  clienteMQTT.setCallback(callback);//el nombre de la funcion a la cual haremos un callback cuando ingresen nuevos datos
}

void loop() {
  if (!clienteMQTT.connected()) {
    uint32_t tiempoActual = millis();
    if (tiempoActual - ultimoIntentoReconexion > 5000) {
      ultimoIntentoReconexion = tiempoActual;
      if (reconexionMQTT()) {
        ultimoIntentoReconexion = 0;
      }
    }
  } else {
    if (sensor()) {
      Serial.println("Enviando Alerta");
      String a = "chiquimula ";
      a += String(random(99));
      char s[15];
      a.toCharArray(s, 15);
      Serial.println(s);
      clienteMQTT.publish(ENVIARALERTA, s);
    }
    clienteMQTT.loop();
  }
}

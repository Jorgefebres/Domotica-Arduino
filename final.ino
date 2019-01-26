#include <SoftwareSerial.h> //interfaz de resultado
#include <OneWire.h> // sensor de temperatura , cable sensor de datos
#include <DallasTemperature.h> // sensor de temperatura

#include "Ultrasonic.h"     //Libreria del sonar
#include "pitches.h"        //Libreria que contiene las notas musicales

#define PinSensorTemperatura 2//Sensor de temperatura en el pin digital 2
#define LDR 0 //SENDSOR DE LUMINOSIDAD EN EL A0 (ANALOGICO 0)
#define pinTransistorFoco 4 //PIN QUE ACTIVA EL TRANSISTOR DEL FOCO
#define pinTransistorVentilador 5 //PIN QUE ACTIVA EL TRANSISTOR DEL VENTILADOR
#define RxD 10 //PIN DE RECEPCION DE LA COMUNICACION CON BT
#define TxD 11 //PIN DE TRASMISION DE LA COMUNICACION CON BT

OneWire ourWire(PinSensorTemperatura); //Se establece el pin declarado como bus para la comunicación OneWire
DallasTemperature sensors(&ourWire); //Se instancia la librería DallasTemperature
SoftwareSerial BTSerial(RxD, TxD);


byte pinEstadoFoco = 0;
byte pinEstadoVentilador = 0;
byte estadoFoco = 0;
byte estadoVentilador = 0;

int sonido = NOTE_E5;    //Declaramos la nota musical elegida como el sonido para alarma
int alarma = 7;          //Declaramos el pin donde se encuentra conectada la alarma
int sonar=0;               //Declaramos la variable sonar para calcular la distancia

int luminosidad = 0;
int valor_sensor_luz = 0;
float temperatura = 0;
byte automaticoFoco = 1; //inicia como automatico (1) si se presiona el switch manual cambiará a 0
byte automaticoVentilador = 1;

Ultrasonic ultrasonic(8,9); //declaramos los pines 8 y 9 como Trigger y  Echo

void setup () {
  // Estado inicial
  pinMode(LDR, INPUT);
  pinMode(pinTransistorFoco, OUTPUT);
  digitalWrite(pinTransistorFoco, LOW);
  pinMode(pinTransistorVentilador, OUTPUT);
  digitalWrite(pinTransistorVentilador, LOW);

  // Configuracion del puerto serie por software para comunicar con el modulo HC-06
  BTSerial.begin(9600);
  BTSerial.flush();
  delay(500);

  // Configuramos el puerto serie de Arduino para Debug
  Serial.begin(9600);
  Serial.println("Ready");

  sensors.begin(); //Se inicia la lectura en el sensor de temperatura
}

void loop() {
  //BLUETOOTH
  // Esperamos ha recibir datos.
  if (BTSerial.available()) {
    // La funcion read() recibe un caracter
    char command = BTSerial.read();
    BTSerial.flush();
    Serial.println(command);
    // En caso de que el caracter recibido sea "F" cambiamos
    // El estado del FOCO
    if (command == 'F') {
      Serial.println("SE PRESIONO EL BOTON DEL FOCO EN LA APLICACION");
      //interruptor(pinTransistorFoco, estadoFoco);
      estadoFoco = !estadoFoco;
      digitalWrite(pinTransistorFoco, estadoFoco);
      automaticoFoco = 0;
    }
    //LO MISMO PARA EL VENTILADOR si se recibe la letra "V"
    if (command == 'V') {
      Serial.println("SE PRESIONO EL BOTON DEL VENTILADOR EN LA APLICACION");
      //interruptor(pinTransistorVentilador, estadoVentilador);
      estadoVentilador = !estadoVentilador;
      digitalWrite(pinTransistorVentilador, estadoVentilador);
      automaticoVentilador = 0;
    }
    if (command == 'A') {
      Serial.println("SE PRESIONO EL BOTON DE AUTOMATICO EN LA APLICACION ");
      if (automaticoFoco == 0 || automaticoVentilador == 0) {//si alguno de los controles no esta en automatico
        automaticoFoco = 1;
        automaticoVentilador = 1;
      }
      else
      {
        automaticoFoco = 0;
        automaticoVentilador = 0;
      }

    }
  }
  //MIENTRAS ESTE EN AUTOMATICO EL CONTROL DE FOCO O VENTILADOR
  if (automaticoFoco == 1) {
    //foco
    valor_sensor_luz = analogRead(LDR);
    luminosidad = (5.0 * valor_sensor_luz * 100.0) / 1024.0;
    Serial.print("LUMINOSIDAD: ");
    Serial.print(luminosidad);
    delay(300);
    if ((luminosidad >= 490) && (estadoFoco == 0)) //Si la resistencia que produce el ldr es mayor o igual que 490 y el foco esta apagado
    {
      digitalWrite (pinTransistorFoco, HIGH);  //El foco se ENCIENDE
      Serial.println(" FOCO ENCENDIDO.");
      estadoFoco = 1;
    }
    else   //Si es menor que 490
    {
      digitalWrite (pinTransistorFoco, LOW);  //El foco se apaga
      Serial.println(" FOCO APAGADO.");
      estadoFoco = 0;
    }
    estadoFoco = 0;
  }
  if (automaticoVentilador == 1) {
    //VENTILADOR
    sensors.requestTemperatures(); //Prepara el sensor para la lectura
    temperatura = sensors.getTempCByIndex(0);
    Serial.print("TEMPERATURA: ");
    Serial.print(temperatura); //Se lee e imprime la temperatura en grados Celsius
    Serial.print(" GRADOS CENTIGRADOS");
    //Serial.print(sensors.getTempFByIndex(0)); //Se lee e imprime la temperatura en grados Fahrenheit
    //Serial.println(" grados Fahrenheit");
    if (temperatura >= 25) //Si la temperatura es mayor o igual que 25 y el ventilador esta apagado
    {
        if (estadoVentilador == 0) {
          digitalWrite (pinTransistorVentilador, HIGH);  //El ventilador se ENCIENDE          
          estadoVentilador = 1;
        }
      Serial.println(" VENTILADOR ENCENDIDO.");
    }
    if(temperatura<25)   //Si es menor que 24
    {
      digitalWrite (pinTransistorVentilador, LOW);  //El ventilador se APAGA
      Serial.println(" VENTILADOR APAGADO.");
      estadoVentilador = 0;
    }
  }
  String a = String(luminosidad) + "," + String(temperatura) + "\n";
  envioDatosBluetooth(a);

  //ALARMA
  //La funcion ultrasonic.ranging(cm) viene declarada en la libreria del sonar
   //Calcula la distancia a la que rebota una señal enviada basandose en el 
   //tiempo que tarda en recorrer dicha distancia, devolviendonos la distancia
   //en centimetros, lista para utilizar en casos de medicion por ultrasonidos.
   sonar = ultrasonic.Ranging(CM); //Leemos la distancia del sonar                          
                                   
   Serial.print("DISTANCIA: ");
   Serial.println(sonar);                            
     
       if (sonar < 15)     //Si la distancia del sonar es menor que 15 cm
       {
         tone(alarma, sonido);                //Suena sin interrupciones indicando la proximidad del objeto
         sonar = ultrasonic.Ranging(CM); //Distancia del sonar
         Serial.println("ALARMA ACTIVADA!!!");  
         delay(3000); //SI LA DISTANCIA ES MENOR A 18 HAREMOS SONAR LA ALARMA POR 3 SEGUNDOS         
       }
       delay(500);
       noTone(alarma);
}
void envioDatosBluetooth(String d) {
  BTSerial.println(d);
}

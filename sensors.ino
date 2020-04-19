#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include "Adafruit_HTU21DF.h"

// Define the times for the loop
unsigned long starttime;
unsigned long endtime;


// Rain gauge interrupt
int interruption = 0;
unsigned long gaugeTicks = 0;
void interruptFunction(){
  gaugeTicks++;
}

// Pressure sensor
Adafruit_BMP280 bmp; // I2C
float pres_280; // Almacena la presion atmosferica (Pa)
float temp_280; // Almacena la temperatura (oC)

// Temperature and humisity sensor
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
float temp_htu;
float humi_htu;
 
// Analog Sensor Input Pin
int moistureInput = 0;    // soil probe is on analog input 0
int moistureReading;

//LoRa sending String
String sendingString;
int sizeSendingString;

void setup() {
// Inicializamos comunicación serie (Testing prupourses)
  // Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // for testing
  
  // Initialize the I2C bus and the sensors
  Wire.begin(); // Light sensor
  bmp.begin(0x76); // Pressure sensor
  htu.begin(); // Temperature and humidity sensor
  delay(1000);

  // Raing gauge interrupt
  attachInterrupt(interruption, interruptFunction, RISING);

  //LoRa part
  // the setup routine runs once when you press reset:
  Serial.begin(115200);
  delay(100);
  Serial.print("AT+RESET\r\n");
  delay(4000);
  Serial.print("AT+MODE=1\r\n"); //Mode = 0 is sleep mode
  delay(100);
  Serial.print("AT+IPR=115200\r\n"); //Set the UART baud rate
  delay(100);
  Serial.print("AT+PARAMETER=12,3,1,7\r\n"); //< 3km range recommended parameters
  delay(100);
  Serial.print("AT+BAND=915000000\r\n"); //Select the band in Hz
  delay(100);
  Serial.print("AT+ADDRESS=1\r\n"); //1 for the transmitter, 2 for the reciever
  delay(100);
  Serial.print("AT+NETWORKID=10\r\n"); //Same in both trans and rec
  delay(100);
}

void loop() { 

  starttime = millis();
  endtime = starttime;

  while ((endtime - starttime) <=60000){ // do this loop for up to 1 min
  
  send_reading();
  // Esperamos X milisegundos entre medidas
  delay(5000);

  endtime = millis();
  }
  
  delay(240000); // 4 minutos
}


void send_reading(){

  sendingString = "";
  //PRESSURE
  // Lee valores del sensor:
  pres_280 = bmp.readPressure()/100;
  temp_280 = bmp.readTemperature();
  sendingString += String(pres_280);
  sendingString += ",";
  sendingString += String(temp_280);
  sendingString += ",";
  delay(20);

  // TEMPERATURE AND HUMIDUTY
  temp_htu = htu.readTemperature();
  humi_htu = htu.readHumidity();
  sendingString += String(temp_htu);
  sendingString += ",";
  sendingString += String(humi_htu);
  sendingString += ",";
  delay(20);
  
  //MOISTURE
  moistureReading = analogRead(moistureInput);
  sendingString += String(moistureReading);
  sendingString += ",";
  delay(20);

  //RAIN GAUGE
  sendingString += String(gaugeTicks);
  delay(20);
  
  //LoRa Sending
  String s = "AT+SEND=2,";
  sizeSendingString = sendingString.length();
  s += String(sizeSendingString);
  s += ",";
  s += sendingString;
  s += "\r\n";
  Serial.print(s);

}

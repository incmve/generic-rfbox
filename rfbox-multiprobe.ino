/*
                 Arduino Nano V3
                     _______
  RX             D0 -|  N  |- 
  TX             D1 -|  A  |-             GND
					-|  N  |-             
  GND               -|  O  |-   		  5V
  receive        D2 -|     |- A7
                 D3 -|  V  |- A6      
  transmit       D4 -|  3  |- A5          SCL
  DS18B20        D5 -|     |- A4          SDA
                 D6 -|     |- A3      
                 D7 -|     |- A2      
				 D8 -|     |- A1      
  DHT11          D9 -|     |- A0      			 
				D10 -|     |-             
	ECHO		D11 -|     |-             3.3V			
	TRIG		D12 -|_____|- D13         INT.LED

 * Generic Sender code : Send a value (counter) over RF 433.92 mhz
 * Fréquence : 433.92 mhz
 * Protocole : homepi 
 * Licence : CC -by -sa
 * Auteur : Yves Grange * https://github.com/Yves911/generic_433_sender
 * Version : 0.1
 * Lase update : 10/10/2014
 * rfbox source https://github.com/incmve/generic-rfbox
 * Based on: Valentin CARRUESCO aka idleman and Manuel Esteban aka Yaug (http://manuel-esteban.com) work  
 * used the code from (https://github.com/koffienl/pimatic-probe) to enable/disable modules
 */

// Includes
#include <OneWire.h> // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h> // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip
#include <dht.h> // http://playground.arduino.cc/Main/DHTLib#.UyMXevldWCQ


// Define vars
#define DHT11_PIN 9
#define senderPin 4 // 
const int ledPin = 13; // internal LED PIN
#define ONE_WIRE_BUS 5 // DS18B20 PIN
#define echoPin 11 // Echo Pin
#define trigPin 12 // Trigger Pin

long codeKit = 1001;  // Your unique ID for your Arduino node
int Bytes[30]; 
int BytesData[30]; 
int maximumRange = 200; // Maximum range sonar
int minimumRange = 0; // Minimum range sonar
long duration, distance; // Duration used to calculate distance

// Config which modules to use
boolean DHT11 = false;
boolean DS18B20 = true;
boolean ultrasonic = false;


// Start includes
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature  
dht DHT;    

void itob(unsigned long integer, int length)
{  
 for (int i=0; i<length; i++){
   if ((integer / power2(length-1-i))==1){
     integer-=power2(length-1-i);
     Bytes[i]=1;
   }
   else Bytes[i]=0;
 }
}

void itobCounter(unsigned long integer, int length)
{  
 for (int i=0; i<length; i++){
   if ((integer / power2(length-1-i))==1){
     integer-=power2(length-1-i);
     BytesData[i]=1;
   }
   else BytesData[i]=0;
 }
}

unsigned long power2(int power){    //gives 2 to the (power)
 unsigned long integer=1;          
 for (int i=0; i<power; i++){      
   integer*=2;
 }
 return integer;
}

/**
 * Crée notre signal sous forme binaire
**/
void buildSignal()
{
  Serial.println(codeKit);
  // Converti les codes respectifs pour le signal en binaire
  itob(codeKit, 14);
  for(int j=0;j < 14; j++){
   Serial.print(Bytes[j]);
  }
  Serial.println();
}

// Convert 0 in 01 and 1 in 10 (Manchester conversion)
void sendPair(bool b) {
 if(b)
 {
   sendBit(true);
   sendBit(false);
 }
 else
 {
   sendBit(false);
   sendBit(true);
 }
}

//Envois d'une pulsation (passage de l'etat haut a l'etat bas)
//1 = 310µs haut puis 1340µs bas
//0 = 310µs haut puis 310µs bas
void sendBit(bool b) {
 if (b) {
   digitalWrite(senderPin, HIGH);
   delayMicroseconds(650);   //506 orinally, but tweaked.
   digitalWrite(senderPin, LOW);
   delayMicroseconds(2024);  //1225 orinally, but tweaked.
 }
 else {
   digitalWrite(senderPin, HIGH); 
   delayMicroseconds(650);   //506 orinally, but tweaked.
   digitalWrite(senderPin, LOW);
   delayMicroseconds(4301);   //305 orinally, but tweaked.
 }
}

/** 
 * Transmit data
 * @param boolean  positive : if the value you send is a positive or negative one
 * @param long Counter : the value you want to send
 **/
void transmit(boolean positive, unsigned long Counter, int BytesType[], int repeats)
{
 int ii;
 for(ii=0; ii<repeats;ii++)
 {
  int i;
  itobCounter(Counter, 30);

  // Send the unique ID of your Arduino node
  for(i=0; i<14;i++)
 {
  sendPair(Bytes[i]);
 }

  // Send protocol type
 for(int j = 0; j<4; j++)
 {
  sendPair(BytesType[j]);
 }

 // Send the flag to mark the value as positive or negative
 sendPair(positive);

 // Send value (ie your counter)
 for(int j = 0; j<30; j++)
 {
   sendPair(BytesData[j]);
 }

 // Send the flag "End of the transmission"
 digitalWrite(senderPin, HIGH);
 delayMicroseconds(650);     
 digitalWrite(senderPin, LOW);
 delayMicroseconds(8602);
 }
}

void setup()
{
  pinMode(senderPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  buildSignal();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  
  if (DS18B20) {
     //start up temp sensor
    sensors.begin();
  }
}

void loop()
{
  if (DS18B20) {
     // Read DS18B20 and transmit value as sensor 1
 float temperature;
 sensors.requestTemperatures(); // Get the temperature
 temperature = sensors.getTempCByIndex(0); // Get temperature in Celcius
 unsigned long CounterValue = temperature * 10;
 int BytesType[] = {0,0,0,1}; // type = 1
  if (temperature >= 0.0) {
      Serial.println("Positive temp");
      Serial.println(CounterValue);
      transmit(true, CounterValue, BytesType, 6); 
    }
    if (temperature < 0.0) {
      Serial.println("Negative temp");
      Serial.println("-");
      Serial.println(CounterValue);
      transmit(false, CounterValue, BytesType, 6);
    }
 Blink(ledPin,1);
 Serial.println(CounterValue);
 delay(10000); // wait for 10 seconds to go to next sensor
  }

  if (DHT11) {
    // Read DHT11 and transmit value as sensor 2
    int chk = DHT.read11(DHT11_PIN);
    switch (chk)
    {
      case DHTLIB_OK:
      float humfloat = DHT.humidity;
      int CounterValue = humfloat * 10;
      int BytesType[] = {0,0,1,0}; // type = 2
      transmit(true, CounterValue, BytesType, 6);
	  Blink(ledPin,2);
      break;
	  Serial.println(CounterValue);
	  delay(10000); // wait for 10 seconds to go to next sensor
    }
  }
  if (ultrasonic) {
    // Read Sonar and transmit value as sensor 3
    /* The following trigPin/echoPin cycle is used to determine the
	distance of the nearest object by bouncing soundwaves off of it. */ 
	digitalWrite(trigPin, LOW); 
	delayMicroseconds(2); 

	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10); 
 
	digitalWrite(trigPin, LOW);
	duration = pulseIn(echoPin, HIGH);
 
	//Calculate the distance (in cm) based on the pace of sound. http://www.instructables.com/id/Using-a-SR04/
	distance = duration/58.2;
 
	if (distance >= maximumRange || distance <= minimumRange){
	/* Send a negative number to computer and Turn LED ON 
	to indicate "out of range" */
	Serial.println("Out of range");
 
	}
	else {
	/* Send the distance to the computer using Serial protocol, and
	turn LED OFF to indicate successful reading. */
	int BytesType[] = {0,0,1,1}; //transmit value as sensor 3
	transmit(true, distance, BytesType, 6);
	Serial.println(distance);
        Blink(ledPin,3);
		}
	}

  delay(1800000); // wait for 30 minutes to restart loop, be aware if to short RF pollution will occur.
  
}
void Blink(int led, int times)
{
 for (int i=0; i< times; i++)
 {
  digitalWrite(ledPin,HIGH);
  delay (250);
  digitalWrite(ledPin,LOW);
  delay (250);
 }
}

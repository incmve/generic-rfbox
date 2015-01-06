/*
 * Active low relay receiver working with kaku protocol and retrieve status
*/

//includes
#include <NewRemoteReceiver.h>

//variables
int state1;
int state2;
int state3;

#define RELAY1  8
#define RELAY2  9
#define RELAY3  10
#define senderPin 4 // 

long codeKit = 1000;  // Your unique ID for your Arduino node
int Bytes[30]; 
int BytesData[30]; 



void setup() {
  Serial.begin(115200);
    NewRemoteReceiver::init(0, 2, rfstart);
    digitalWrite(RELAY1, HIGH);
    pinMode(RELAY1, OUTPUT);
    pinMode(senderPin, OUTPUT);
    buildSignal();
  
  // Initialize receiver on interrupt 0 (= digital pin 2), calls the callback "rfstart"
  // after 2 identical codes have been received in a row. (thus, keep the button pressed
  // for a moment)
  //
  // See the interrupt-parameter of attachInterrupt for possible values (and pins)
  // to connect the receiver.
  
}

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

void loop() 
{
  
}

// Callback function is called only when a valid code is received.
void rfstart(NewRemoteCode receivedCode) {
  // Note: interrupts are disabled. You can re-enable them if needed.
  
   if (receivedCode.address == 99 && receivedCode.unit == 9 && receivedCode.switchType == 1) //Unit 99, ID 9, ON
    {
    // Disable the receiver
    NewRemoteReceiver::disable();
    state1 = digitalRead(RELAY1);
    Serial.println(state1);
    if (state1 = HIGH)
    {state1 = 0;}
    else state1 = 1; 
    int BytesType[] = {0,0,0,1}; //transmit value as sensor 1
	transmit(true, state1, BytesType, 6);
    delay (5000);
    state2 = digitalRead(RELAY2);
    Serial.println(state2);
    if (state2 = HIGH)
    {state2 = 0;}
    else state2 = 1; 
    int BytesType2[] = {0,0,1,0}; //transmit value as sensor 2
	transmit(true, state2, BytesType2, 6);
    delay (5000);
    state3 = digitalRead(RELAY3);
    Serial.println(state3);
    if (state3 = HIGH)
    {state3 = 0;}
    else state3 = 1; 
    int BytesType3[] = {0,0,1,1}; //transmit value as sensor 3
	transmit(true, state3, BytesType3, 6);
    Serial.println(state1);
    delay (5000);
  // Enable the receiver.
  NewRemoteReceiver::enable();
  }
 
  if (receivedCode.address == 66 && receivedCode.unit == 6 && receivedCode.switchType == 1) //Unit 66, ID 6, ON
    {
	digitalWrite(RELAY1,LOW);           // Turns ON Relays 1
  }
  if (receivedCode.address == 66 && receivedCode.unit == 6 && receivedCode.switchType == 0) //Unit 66, ID 6, OFF
    {
   digitalWrite(RELAY1,HIGH);          // Turns Relay Off 
  }
    if (receivedCode.address == 67 && receivedCode.unit == 6 && receivedCode.switchType == 1) //Unit 67, ID 6, ON
    {
	digitalWrite(RELAY2,LOW);           // Turns ON Relays 1
  }
  if (receivedCode.address == 67 && receivedCode.unit == 6 && receivedCode.switchType == 0) //Unit 67, ID 6, OFF
    {
   digitalWrite(RELAY2,HIGH);          // Turns Relay Off 
  }
    if (receivedCode.address == 68 && receivedCode.unit == 6 && receivedCode.switchType == 1) //Unit 68, ID 6, ON
    {
	digitalWrite(RELAY3,LOW);           // Turns ON Relays 1
  }
  if (receivedCode.address == 68 && receivedCode.unit == 6 && receivedCode.switchType == 0) //Unit 68, ID 6, OFF
    {
   digitalWrite(RELAY3,HIGH);          // Turns Relay Off 
  }
  }
  

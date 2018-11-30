/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
**/

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_RFM69

// The switch Node ID
#define MY_NODE_ID 0x43


/* Each Button status (On or Off) can be sent to a different Relay or Actuator NodeId address.  
 *  relayNodeID stores Relay or Actuator NodeId addresses. Each adress can have different Child\Sensor ID.  
 *  relayChildID is Child\Sensor ID's array. 
 * int relayNodeID[4] = {0xF2, 0xA0,0x0}; 
 * int relayChildID[4] = {1, 2, NULL};
 * above declaration means: Button 1 will send it state to assigned Relay Address 0xF2 with Child\Sensor ID 1. 
 * Button 2 will send it state to assigned Relay Address 0xA0 with Child\Sensor ID 2. 
 * Button 3 have no attached sensorsors and will not be "presented" since there is NULL value.
 * NULL value indicates no switch attached to the corresponding JST connector
*/
int relayNodeID = 0xf3; // Is the recepient address
int relayChildID = 1; //NULL

// Avoid battery drain if Gateway disconnected and the node sends more than MY_TRANSPORT_STATE_RETRIES times message.
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_PARENT_NODE_IS_STATIC
#define MY_PARENT_NODE_ID 0


// Enable and select radio type attached

//#define MY_RADIO_NRF24

#define MY_RADIO_RFM69

// if you use MySensors 2.0 use this style 
//#define MY_RFM69_FREQUENCY   RF69_433MHZ
//#define MY_RFM69_FREQUENCY   RF69_868MHZ
//#define MY_RFM69_FREQUENCY   RF69_915MHZ


//#define MY_RFM69_FREQUENCY   RFM69_915MHZ
//#define MY_RFM69_FREQUENCY   RFM69_433MHZ


//#define MY_RFM69_NEW_DRIVER

// Enable Crypto Authentication to secure the node
//#define MY_SIGNING_ATSHA204
//#define  MY_SIGNING_REQUEST_SIGNATURES


#include <MySensors.h>

#define SPIFLASH_BLOCKERASE_32K   0xD8
#define SPIFLASH_CHIPERASE        0x60

#define SKETCH_NAME "Door\Window Sensor "
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define SENSOR_INTERUPT_PIN 3
#define GREEN_LED 5
#define RED_LED 6

int BATTERY_SENSE_PIN = A6;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

// Initialising array holding button state messages
MyMessage msgSensorState;

void before()
{
  //analogReference(DEFAULT); // DEFAULT

  #ifdef  MY_RADIO_RFM69
    /*  RFM reset pin is 9
     *  A manual reset of the RFM69HCW\CW is possible even for applications in which VDD cannot be physically disconnected.
     *  Pin RESET should be pulled high for a hundred microseconds, and then released. The user should then wait for 5 ms
     *  before using the module.
     */
    pinMode(9, OUTPUT);
    //reset RFM module
    digitalWrite(9, 1);
    delay(1);
    // set Pin 9 to high impedance
    pinMode(9, INPUT);
    delay(10);
  #endif

  // Setup the pins
    //pinMode(SENSOR_INTERUPT_PIN, INPUT_PULLUP);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
}


void blinkGreenSensorLed(){
  digitalWrite(GREEN_LED, HIGH);
  wait(50);
  digitalWrite(GREEN_LED, LOW);
}
void blinkRedSensorLed(){
  digitalWrite(RED_LED, HIGH);
  wait(50);
  digitalWrite(RED_LED, LOW);
}
void blinkSensorLedOK(){
  for (int i = 0; i < 4; i++) {
    digitalWrite(GREEN_LED, HIGH);
    wait(30);
    digitalWrite(GREEN_LED, LOW);
    wait(30);
  }
}
void blinkSensorLedFail(){
  for (int i = 0; i < 4; i++) {
    digitalWrite(RED_LED, HIGH);
    wait(30);
    digitalWrite(RED_LED, LOW);
    wait(30);
  }
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register binary input sensor to sensor_node (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.


      msgSensorState = MyMessage(relayChildID, V_LIGHT);
      present(relayChildID, S_LIGHT);
}

void setup() {

}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference

  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  
  delay(5); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}


void loop(){ 

  uint8_t retry = 5;
    byte SensorValue = digitalRead(SENSOR_INTERUPT_PIN);
    while (!send(msgSensorState.set(digitalRead(SensorValue)), true)  && retry > 0) { 
      // send did not go through, try  "uint8_t retry = 5" more times
      //wait(100); 
      retry--;
      }
    // Failed transmission for "retry" times
    (retry > 0) ? blinkSensorLedOK() : blinkSensorLedFail();
    // Show LED status
    (SensorValue) ? blinkRedSensorLed() : blinkGreenSensorLed();
    wait(100);    

  /* Get battery level in mV
   *  2.4V - lowest level, 3v - max level
   */
  int sensorValue = readVcc();
  int batteryPcnt = 100 * (sensorValue - 2400) / 600;
  
  batteryPcnt = batteryPcnt > 0 ? batteryPcnt:0; // Cut down negative values. Just in case the battery goes below 4V and the node still working. 
  batteryPcnt = batteryPcnt < 100 ? batteryPcnt:100; // Cut down more than "100%" values. In case of ADC fluctuations. 

  if (oldBatteryPcnt != batteryPcnt ) {
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }

  sleep(SENSOR_INTERUPT_PIN - 2, CHANGE  , 0); //FALLING
}

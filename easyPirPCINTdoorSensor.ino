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

#include <avr/wdt.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//#define MY_RADIO_RFM95
//#define MY_RFM95_MODEM_CONFIGRUATION  RFM95_BW125CR45SF128
#define MY_RFM95_MODEM_CONFIGRUATION RFM95_BW_500KHZ | RFM95_CODING_RATE_4_5, RFM95_SPREADING_FACTOR_2048CPS | RFM95_RX_PAYLOAD_CRC_ON, RFM95_AGC_AUTO_ON // 
#define MY_RFM95_TX_POWER_DBM (14u)
//#define MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR48SF4096
//RFM95_BW125CR45SF128

// air-time approximation for timeout, 1 hop ~15 bytes payload - adjust if needed
// BW125/SF128: 50ms
// BW500/SF128: 15ms
// BW31.25/SF512: 900ms
// BW125/SF4096: 1500ms

#define RFM95_RETRY_TIMEOUT_MS      (2500ul)      //!< Timeout for ACK, adjustments needed if modem configuration changed (air time different)

//#define   MY_RFM95_FREQUENCY RFM95_915MHZ
//#define   MY_RFM95_FREQUENCY RFM95_868MHZ
//#define   MY_RFM95_FREQUENCY RFM95_433MHZ



// Enable and select radio type attached
#define MY_RADIO_RFM69
//#define MY_IS_RFM69HW
#define MY_RFM69_TX_POWER_DBM (13) 

// if you use MySensors 2.0 use this style 
//#define MY_RFM69_FREQUENCY   RFM69_433MHZ
#define MY_RFM69_FREQUENCY   RFM69_868MHZ
//#define MY_RFM69_FREQUENCY   RFM69_915MHZ


// Comment it out for Auto Node ID #
#define MY_NODE_ID 0xD0


int relayNodeIDPIRSensor  = 0x0; // Relay addressess to send switch ON\OFF states. Can be any address; 0 is SmartHome controller address.
int relayNodeIDmagSensor  = 0x0; // Relay addressess to send switch ON\OFF states. Can be any address; 0 is SmartHome controller address.
int relayChildID  = 1; //0 value means no need to report\present it to controller;


// Avoid battery drain if Gateway disconnected and the node sends more than MY_TRANSPORT_STATE_RETRIES times message.
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_PARENT_NODE_IS_STATIC
#define MY_PARENT_NODE_ID 0


//Enable OTA feature
//#define MY_OTA_FIRMWARE_FEATURE
//#define MY_OTA_FLASH_JDECID 0x0//0x2020

//Enable Crypto Authentication to secure the node
//#define MY_SIGNING_ATSHA204
//#define  MY_SIGNING_REQUEST_SIGNATURES

#include <Wire.h>

// Written by Christopher Laws, March, 2013.
// https://github.com/claws/BH1750
#include <BH1750.h>
BH1750 lightMeter;

#include "SparkFun_Si7021_Breakout_Library.h"
//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

#include <MySensors.h>

// Redefining write codes for JDEC FLASH used in the node
// These two defines should always be after #include <MySensors.h> declaration
#define SPIFLASH_BLOCKERASE_32K   0xD8
#define SPIFLASH_CHIPERASE        0x60

#include <stdlib.h>

#define MAGNET_PIN 3
#define PIR_PIN 7
#define RED_LED_PIN 6
#define GREEN_LED_PIN 5



// Assign numbers for all sensors we will report to gateway\controller (they will be created as child devices)
#define PIR_sensor 1
#define MAG_sensor 2
#define HUM_sensor 3
#define TEMP_sensor 4
#define VIS_sensor 5


// Create MyMessage Instance for sending readins from sensors to gateway\controller (they will be created as child devices)

MyMessage msg_PIR(PIR_sensor, V_LIGHT);
MyMessage msg_mag(MAG_sensor, V_LIGHT);
MyMessage msg_hum(HUM_sensor, V_HUM);
MyMessage msg_temp(TEMP_sensor, V_TEMP);
MyMessage msg_vis(VIS_sensor, V_LEVEL); //V_LIGHT_LEVEL

unsigned long wdiDelay2  = 0;

int BATTERY_SENSE_PIN = A6;  // select the input pin for the battery sense point

static int32_t oldLux = 0, lux;
static int16_t oldHumdty = 0, humdty;
static int16_t oldTemp = 0, temp;



volatile bool flagIntPIR = false, flagIntMagnet = false;
//#define G_VALUE 16380
//#define G_VALUE2 268304400 //G_VALUE * G_VALUE

void pinsIntEnable(){  
  //   Pcint 23 d7 D0-D7 = PCINT 16-23 = PCIR2 = PD = PCIE2 = pcmsk2
  // PCINT23 (PCMSK2 / PCIF2 / PCIE2)
  
  PCMSK2 |= bit (PCINT23);
  PCIFR  |= bit (PCIF2);   // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);   // enable pin change interrupts for A0 to A1
}

ISR (PCINT2_vect){
  flagIntPIR = true;
}

void magnetSensorInterruptHandler(){
  flagIntMagnet = true;
}


void blinkGreenSensorLed(int  i = 1){
  for (;i>0;i--){
    digitalWrite(GREEN_LED_PIN, HIGH);
    wait(50);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}
void blinkRedSensorLed(int  i = 1){
  for (;i>0;i--){
  digitalWrite(RED_LED_PIN, HIGH);
  wait(50);
  digitalWrite(RED_LED_PIN, LOW);
  }
}

void battery_report(){
return;
  static int oldBatteryPcnt = 0;

  // Get the battery Voltage
  int sensorValue = analogRead(BATTERY_SENSE_PIN);
  /* 1M, 470K divider across batteries
   * 610 ~ 100 % is close to 6.1 V
   * 400 ~ 0 % is close to 4V
   */
  int batteryPcnt = (sensorValue - 400)  / 2;
  
  batteryPcnt = batteryPcnt > 0 ? batteryPcnt:0; // Cut down negative values. Just in case the battery goes below 4V and the node still working. 
  batteryPcnt = batteryPcnt < 100 ? batteryPcnt:100; // Cut down more than "100%" values. In case of ADC fluctuations. 

  if (oldBatteryPcnt != batteryPcnt ) {
    wait(100);
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }

}

void swarm_report()
{
  char humiditySi7021[10];
  char tempSi7021[10];
  char visualLight[10];

  lightMeter.begin(BH1750::ONE_TIME_LOW_RES_MODE); // need for correct wake up
  lux = lightMeter.readLightLevel();// Get Lux value
  // dtostrf(); converts float into string
  dtostrf(lux,5,0,visualLight);
  if ( oldLux != lux ) {
    // this wait(); is 2.0 and up RFM69 specific. Hope to get rid of it soon
    wait(100);
    send(msg_vis.set(visualLight), true);  // Send LIGHT BH1750     sensor readings
    oldLux = lux;
  }

   
  // Measure Relative Humidity from the Si7021
  humdty = sensor.getRH();
  dtostrf(humdty,0,2,humiditySi7021);  
  if (humdty != oldHumdty) {
    wait(100);
    send(msg_hum.set(humiditySi7021), true); // Send humiditySi7021     sensor readings
    oldHumdty = humdty; 
  }

  // Measure Temperature from the Si7021
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  temp = sensor.getTemp();
  dtostrf(temp,0,2,tempSi7021);
  if (temp != oldTemp) {
    wait(100);
    send(msg_temp.set(tempSi7021), true); // Send tempSi7021 temp sensor readings
    oldTemp = temp;
  }

  battery_report();
}


void before() {
    //No need watch dog enabled in case of battery power.
    //wdt_enable(WDTO_4S);
    wdt_disable();
  
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
  
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN,0);
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(GREEN_LED_PIN,0);
    pinMode(PIR_PIN, INPUT_PULLUP);
    pinsIntEnable();
    pinMode(MAGNET_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MAGNET_PIN), magnetSensorInterruptHandler, CHANGE);
}

void setup() {
}

void presentation() 
{  
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("PIR node", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(PIR_sensor, S_BINARY);
  present(MAG_sensor, S_BINARY);
  present(HUM_sensor, S_HUM);
  present(TEMP_sensor, S_TEMP);
  present(VIS_sensor, S_LIGHT_LEVEL);
}

unsigned long wdiDelay  = 0;


int PIRValue = 0, MagSensorValue = 0;

void loop()
{
  //No need watch dog in case of battery power.
  //wdt_reset();

  if ( flagIntPIR && digitalRead(PIR_PIN) == HIGH )  {
      PIRValue ?  PIRValue  = 0: PIRValue = 1;  // inverting the value each time
      msg_PIR.setDestination(relayNodeIDPIRSensor); 
      // Blink  respective LED's once if message delivered to controller. 3 times if failed
      send(msg_PIR.set(PIRValue),true) ? blinkGreenSensorLed(1) : blinkGreenSensorLed(3); 
      // wait for ACK signal up to RFM95_RETRY_TIMEOUT_MS or 50ms for rfm miliseconds
      #ifdef  MY_RADIO_RFM95
        wait(RFM95_RETRY_TIMEOUT_MS, 1, 3);
      #endif
      #ifdef  MY_RADIO_RFM69
        wait(100, 1, 3);
      #endif
      swarm_report();    
  }

  if ( flagIntMagnet )  {
      MagSensorValue ?  MagSensorValue  = 0: MagSensorValue = 1;  // inverting the value each time
      msg_mag.setDestination(relayNodeIDmagSensor); 
      // Blink  respective LED's once if message delivered to controller. 3 times if failed
      send(msg_mag.set(MagSensorValue),true) ? blinkGreenSensorLed(1) : blinkGreenSensorLed(3);
            // wait for ACK signal up to RFM95_RETRY_TIMEOUT_MS or 50ms for rfm miliseconds
      #ifdef  MY_RADIO_RFM95
        wait(RFM95_RETRY_TIMEOUT_MS, 1, 3);
      #endif
      #ifdef  MY_RADIO_RFM69
        wait(100, 1, 3);
      #endif
      swarm_report();  
  }

  
  flagIntMagnet = false;
  flagIntPIR = false;

  sleep(0);
  // sleep( 3 - 2, CHANGE,0);  // 300000 BUTTONS_INTERUPT_PIN
}

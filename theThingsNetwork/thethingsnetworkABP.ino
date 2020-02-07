


/*******************************************************************************
 * Skectch is baesed on   ABP example sketches from https://github.com/matthijskooijman/arduino-lmic 
 *
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1, 
*  0.1% in g2). 
 *
 * Change DEVADDR to a unique address! 
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h, default is:
 *   #define CFG_sx1272_radio 1
 * for SX1272 and RFM92, but change to:
 *   #define CFG_sx1276_radio 1
 * for SX1276 and RFM95.
 *
 *******************************************************************************/

// https://github.com/rocketscream/Low-Power
#include <LowPower.h>
// https://github.com/matthijskooijman/arduino-lmic
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>



// LoRaWAN Application identifier (AppEUI)
// Not used in this example
static const u1_t APPEUI[8]  = { 0xF6,0x65,0x02,0xD0,0x7E,0xD5,0xB3,0x70 };

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = {0xD8,0x09,0x0B,0x45,0xBE,0xDF,0x6A,0x00};


// LoRaWAN NwkSKey, network session key 
// Use this key for The Things Network

static const u1_t DEVKEY[16] = { 0x7A, 0xFF, 0x12, 0x18, 0x18, 0x4B, 0x4B, 0xE3, 0x8F, 0xBA, 0x58, 0x6C, 0x68, 0xAC, 0x3B, 0x17 };
//static const u1_t DEVKEY[16] = { 0x17, 0x3B, 0xAC, 0x68, 0x6C, 0x58, 0xBA, 0x8F, 0xE3, 0x4B, 0x4B, 0x18, 0x18, 0x12, 0xFF, 0x7A };


// LoRaWAN AppSKey, application session key
// Use this key to get your data decrypted by The Things Network

//static const u1_t ARTKEY[16] = { 0x83, 0xB3, 0x44, 0x36, 0xB1, 0x59, 0x30, 0x9F, 0x38, 0x8D, 0xE2, 0x4A, 0xD9, 0x21, 0x50, 0x02 };
static const u1_t ARTKEY[16] = { 0x02, 0x50, 0x21, 0xD9, 0x4A, 0xE2, 0x8D, 0x38, 0x9F, 0x30, 0x59, 0xB1, 0x36, 0x44, 0xB3, 0x83 };


// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
static const u4_t DEVADDR =  0x2601190E; //0E190126 0x2601190E <-- Change this address for every node!

uint8_t mydata[2] = {0,1};

#include "SparkFun_Si7021_Breakout_Library.h"
//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

uint32_t LEDmillisStarted = 0, wakeUpMillisStarted = 0;
volatile boolean magnetSensorSendFlag = false, evTxCompleteFlag = false, evJoinedFlag = false;
uint16_t magnetSensorPin = 0;

static int16_t oldHumdty = 0, humdty;
static int16_t oldTemp = 0, temp;


#define RED_LED_PIN 6 //5
#define GREEN_LED_PIN 5 //7



#include <EEPROM.h>

// http://playground.arduino.cc/Code/EEPROMWriteAnything
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  //return 1;
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)  
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
  //return 1;
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}


void intToBytes(byte *buf, int32_t i, uint8_t byteSize) {
    for(uint8_t x = 0; x < byteSize; x++) {
        buf[x] = (byte) (i >> (x*8));
    }
}

void TempHumReport(){
   
  // Measure Relative Humidity from the Si7021
  humdty = sensor.getRH();
  if (humdty != oldHumdty) {
    // send humiditySi7021     sensor readings
    oldHumdty = humdty; 
  }

  // Measure Temperature from the Si7021
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  temp = sensor.getTemp();
  if (temp != oldTemp) {
    // Send tempSi7021 temp sensor readings
    Serial.println("Sending Temp...");
    evTxCompleteFlag = false;
    temp = temp*100;
    intToBytes(mydata,temp,sizeof(mydata));
    // First parameter is the TTN chanel, last one is ACK request
    LMIC_setTxData2(2, mydata, sizeof(mydata), 1);
    oldTemp = temp;
 }

}



//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////



// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}

static osjob_t sendjob;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10, // 6
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9, //5
    .dio = {2,A0,LMIC_UNUSED_PIN}, //{2, 3, 4},
};

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          evTxCompleteFlag = true;

          EEPROM_writeAnything(10, LMIC.seqnoUp);
          
          Serial.print("Event EV_TXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          if(LMIC.dataLen) { // data received in rx slot after tx
              //debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              Serial.println("Data Received!");
          }
          break;
       default:
          break;
    }
}

void do_send(osjob_t* j){
      Serial.print("Time: ");
      Serial.println(millis() / 1000);
      // Show TX channel (channel numbers are local to LMIC)
      Serial.print("Send, txCnhl: ");
      Serial.println(LMIC.txChnl);
      Serial.print("Opmode check: ");
      // Check if there is not a current TX/RX job running
    if (LMIC.opmode & (1 << 7)) {
      Serial.println("OP_TXRXPEND, not sending");
    } else {
      Serial.println("ok");
      // Prepare upstream data transmission at the next possible time.
      // First parameter is the TTN chanel, last one is ACK request
      Serial.println(LMIC_setTxData2(1, mydata, sizeof(mydata), 1));
    }
    // Schedule a timed job to run at the given timestamp (absolute system time)
    //os_setTimedCallback(j, os_getTime()+sec2osticks(120), do_send);
         
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  EEPROM_readAnything(10,  LMIC.seqnoUp);
  if (LMIC.seqnoUp == 0xFFFFFFFF){
    LMIC.seqnoUp = 0;
    EEPROM_writeAnything(10, (u4_t) 0);
  }
  Serial.print(F("FCnt "));
  Serial.println(LMIC.seqnoUp);
  
  // Set static session parameters. Instead of dynamically establishing a session 
  // by joining the network, precomputed session parameters are be provided.
  LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);
  // Disable data rate adaptation
  LMIC_setAdrMode(0);
  // Disable link check validation
  LMIC_setLinkCheckMode(0);
  // Disable beacon tracking
  LMIC_disableTracking ();
  // Stop listening for downstream data (periodical reception)
  LMIC_stopPingable();
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  //LMIC_setDrTxpow(DR_SF11,14);
  //
  //Serial.flush();

  // Use a medium spread factor. This can be increased up to SF12 for
  // better range, but then the interval should be (significantly)
  // lowered to comply with duty cycle limits as well.
  LMIC.datarate = DR_SF7B;
  // This sets CR 4/5, BW125 (except for DR_SF7B, which uses BW250)
  LMIC.rps = updr2rps(LMIC.datarate);


  
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), interruptHandler3, FALLING);
  //attachInterrupt(digitalPinToInterrupt(2), interruptHandler, RISING);

  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN,1);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN,0);
  //pinMode(PIR_PIN, INPUT_PULLUP);
  LEDmillisStarted = millis();
  do_send(&sendjob);
  
}


void interruptHandler3(){
  magnetSensorSendFlag = true;
}
void interruptHandler(){
}


void loop() {

    if (digitalRead(RED_LED_PIN) == 1 && ((millis() - LEDmillisStarted) > 50))  {
        digitalWrite(RED_LED_PIN,0);
    }
    if (digitalRead(RED_LED_PIN) == 0 && ((millis() - LEDmillisStarted) > 5000))  {
        LEDmillisStarted = millis();
        digitalWrite(RED_LED_PIN,1);
    }

    // Check if there is not a current TX/RX job running !(LMIC.opmode & OP_TXRXPEND)
    if ( magnetSensorSendFlag  && !(LMIC.opmode & OP_TXRXPEND)  && evTxCompleteFlag) { //
        Serial.println("Sending magnet...");
        evTxCompleteFlag = false;
        magnetSensorPin = digitalRead(3);
        intToBytes(mydata,magnetSensorPin,sizeof(mydata));
        LMIC_setTxData2(1, mydata, sizeof(mydata), 1);
        magnetSensorSendFlag = false;
    } 

    // if the magnet sensor value sent, let's send temperaure if it has been chenged
    if (!magnetSensorSendFlag  &&  oldTemp != temp && !(LMIC.opmode & OP_TXRXPEND) && evTxCompleteFlag) { //
          TempHumReport();
      }
    
    os_runloop_once();

    // check if magnet sensor chnged state whiile we were sending
    if (!magnetSensorSendFlag && magnetSensorPin != digitalRead(3) && evTxCompleteFlag  ) {
        magnetSensorSendFlag =  true;
    }
    
    // if all sends complete - let's sleep
    if (!magnetSensorSendFlag && oldTemp == temp && evTxCompleteFlag  ) {
          Serial.println("Sleeping...");
          digitalWrite(RED_LED_PIN,0);
          digitalWrite(GREEN_LED_PIN,1);
          delay(100);
          digitalWrite(GREEN_LED_PIN,0);
          magnetSensorSendFlag = false;
          LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //SLEEP_30MS
          wakeUpMillisStarted = millis();
          }
    // something takes too long let's sleep. 
    if ( millis() - wakeUpMillisStarted > 600000 ){
          Serial.println("Sleeping by timeout...");
          digitalWrite(RED_LED_PIN,0);
          digitalWrite(GREEN_LED_PIN,1);
          delay(100);
          digitalWrite(GREEN_LED_PIN,0);
          magnetSensorSendFlag = false;
          evTxCompleteFlag = true;
          LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //SLEEP_30MS
          wakeUpMillisStarted = millis();
      }
}

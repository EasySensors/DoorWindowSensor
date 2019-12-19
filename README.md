![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_enclosure.jpg?raw=true)
![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/PIP_window_bottom.jpg?raw=true)

### The Wireless Door/Window  Sensor (CW1DB3A) Detects if window or door is opened . It is basd on Arduino IDE compatible (the Atmel ATMega328P 8Mhz) microcontroller with HopeRF RFM 69 CW or RFM 69 HCW or RFM 95 LoRa radios on board. Best sutable for Home Automation, IOT. You may think of it as Arduino Pro Mini plus all the items in the picture below:

![](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/replace.jpg?raw=true)


## Specification: ##

 - MCU Atmel ATMega328P 8MHz) microcontroller
 - Radio- HopeRF RFM 69 CW or RFM 69 HCW or RFM 95 LoRa  (915, 868 0r 433 MHz) radio on board
 - Enclosure dimensions 95mm*40mm*20mm 
 - Powered by two AA batteries
 - Booster converter alows to work from batteries drained as low as 0.7V combined
 - Wide operating temperature range. Tested -20 +40 Celsius
  - Temperature and humidity sensor Si7021 
 - High Accuracy Temperature Sensor ±0.4 °C (max), –10 to 85 °C
 - Precision Relative Humidity Sensor ± 3% RH (max), 0–80% RH
 - Light sensor BH1750,  spectral responsibility is approximately human eye response.
 - Authentication security - Atmel ATSHA204A Crypto Authentication Chip
 - Dualoptiboot bootloader. Implements over the air (OTA) firmware update ability
 - FTDI  header for programming
 - Reverse polarity protection.
 - Arduino pins A0 A1 have pads on PCB for any special needs.
 - PIR triggers interrupt PCINT23 D7
 - Magnet sensor triggers hardware interrupt 1

**Pin out:** 


Arduino Pins|	Description
------------|--------------
A0, A1 |	Available ARDUINO analog GPIO / DIGITAL GPIO as PCB pads close to radio module
A6 |	Connected to Battery voltage sensor (via divider) 3M/470k 
A4 |	Connected to sensors i2c
A5 |	Connected to sensors i2c
A6 |	Battery sensing voltage divider 1M/470k
A3 |	Connected to  ATSHA204A
D3 |	Connected to  Magnet sensor
D4 |	Connected to RFM 69/95 DIO1 
D5 | Connected to  GREEN_LED_PIN
D6 | Connected to  RED_LED_PIN
D7 | Connected to  PIR sensor footprint PCINT23 
D8 |	Connected to CS FLASH chip (OTA) M25P40
D2 |	Connected to RFM 69/95 DIO0 
D9 | Connected to RFM 69/95 Reset pin 
D10 |	Connected to RFM 69/95 CS/NSS
D11 |	Connected to  MOSI
D12 |	Connected to  MISO
D13 |	Connected to  SCK



![enter image description here](https://github.com/EasySensors/easyPIRmultisensorsBox2/blob/master/pics/FTDIpinout.png?raw=true)

**Arduino IDE Settings**

![Arduino IDE Settings](https://github.com/EasySensors/ButtonSizeNode/blob/master/pics/IDEsettings.jpg?raw=true)




**programming FTDI adapter connection**

![FTDI Power](https://github.com/EasySensors/SwitchNode/blob/master/pics/FTDIvcc3.jpg?raw=true)


3.3V power option should be used.



How to use it as home automation (IOT) node controller
------------------------------------------------------


door.ino is the Arduino example sketch using [MySensors](https://www.mysensors.org/) API. 

Burn the Door.ino sketch into it an it will became  one of the MySensors home automation network Node. 
To create the network you need controller and at least two Nodes one as a Sensor, relay or Door Node and the other one as 
“Gateway Serial”. I personally love [Domoticz](https://domoticz.com/) as conroller. Please check this [HowTo](https://github.com/EasySensors/ButtonSizeNode/blob/master/DomoticzInstallMySensors.md) to install Domoticz.

However, for no-controller setup, as example, you can use 3 nodes - first node as “Gateway Serial”, second node as relay and last one as switch for that relay. No controller needed then, keep the switch and the relay on the same address and the switch will operate the  relay.

Things worth mentioning about the  [MySensors](https://www.mysensors.org/) Arduino sketch: 


Code |	Description
------------|--------------
#define MY_RADIO_RFM69<br>#define MY_RFM69_FREQUENCY   RF69_433MHZ<br>#define MY_IS_RFM69HW|	Define which radio we use – here is RFM 69<br>with frequency 433 MHZ and it is HW<br>type – one of the most powerful RFM 69 radios.<br>If your radio is RFM69CW - comment out line<br>with // #define MY_IS_RFM69HW 
#define MY_NODE_ID 0xE0 | Define Node address (0xE0 here). I prefer to use static addresses<br> and in Hexadecimal since it is easier to identify the node<br> address in  [Domoticz](https://domoticz.com/) devices list after it<br> will be discovered by controller ( [Domoticz](https://domoticz.com/)).<br> However, you can use AUTO instead of the hardcoded number<br> (like 0xE0) though.  [Domoticz](https://domoticz.com/) will automatically assign node ID then.
#define MY_SIGNING_ATSHA204 <br>#define  MY_SIGNING_REQUEST_SIGNATURES | Define if you like to use Crypto Authentication to secure your nodes<br> from intruders or interference. After that, you have to “personalize”<br> all the nodes, which have those, defines enabled.<br> [**How to “personalize” nodes with encryption key**](https://github.com/EasySensors/ButtonSizeNode/blob/master/SecurityPersonalizationHowTo.md).<br> You need both defines in the nodes you need to protect.<br> The Gateway Serial could be with only one of those<br> defines enabled - #define MY_SIGNING_ATSHA204
sleep(BUTTONS_INTERUPT_PIN - 2, RISING, 0); | Sends the Door Controller into the sleep mode untill<br> somebody press any button. 

Connect the Node to FTDI USB adaptor, select Pro Mini 8MHz board in Arduino IDE and upload the door.ino sketch.

**Done**


The board is created by  [Koresh](https://www.openhardware.io/user/143/projects/Koresh)

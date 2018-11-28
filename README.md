![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_pcb_with_enclosure.jpg?raw=true)
![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_top_rfm69.jpg	
?raw=true)
![arduino Door Windo wSensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_bottom_nrf.jpg?raw=true)
![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_enclosure.jpg?raw=true)

### The Wireless Door/Window  Sensor (CW1DB3A) Detects if window or door is opened . It is basd on Arduino IDE compatible (the Atmel ATMega328P 8Mhz) microcontroller with HopeRF RFM69 868\915 or NRF24L01+ 2.4 GHz radio on-board. Best sutable for Home Automation, IOT. You may think of it as Arduino Pro Mini plus all the items in the picture below:

![](https://github.com/EasySensors/switchBox/blob/master/pics/replace2.jpg?raw=true)


## Specification: ##
 - Encloosure Dimensions l95 x w40 x h20 Magnet Dimensions l95 x w14 x h20
 - Sleep current consumption 5uA
 - Optional Authentication security - Atmel ATSHA204A Crypto Authentication Chip 
 - LED indicator
 - RFM69-CW (low power consumption version) 868\915 MHz Radio transceiver
 - Footprint available for replacing  RFM69-CW with NRF24L01+ 2.4 GHz radio
 - Battery voltage sensor (via divider)
 - FTDI header for programming
 - Battery connectors: 2 x AAA batteries
 - Reverse polarity protection


If you find the Wireless Door/Window  Sensor useful, it is possible to buy it here: [link to buy](https://www.tindie.com/products/easySensors/arduino-ide-compatible-wireless-switch-box)

**Pin out:** 

Arduino Pins|	Description
------------|--------------
A6 |	Connected to Battery voltage sensor (via divider) 
D5 | green LED
D6 | red LED
D3 Interrupt1 | Connected to magned (hall) sensor
Interrupt 1 | Occurs when sensor detects change in the magnetic field
A3 |	Connected to  ATSHA204A



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

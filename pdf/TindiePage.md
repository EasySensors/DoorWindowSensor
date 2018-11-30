### The Wireless Door/Window  Sensor (CW1DB3A) Detects if window or door is opened . It is based on Arduino IDE compatible (the Atmel ATMega328P 8Mhz) microcontroller with HopeRF RFM69 868\915 or NRF24L01+ 2.4 GHz radio on-board. Best suitable for Home Automation, IOT. 

## Features
- Fully compatible with the Arduino IDE (PRO MINI 8 MG Hz)
- Enclosure Dimensions l95 x w40 x h20 Magnet Dimensions l95 x w14 x h20
- RFM69-CW (low power consumption version) 868\915 MHz Radio transceiver with onboard tuned pcb antenna.
- Optional NRF24L01+ 2.4 GHz radio
- Optional onboard Atmel ATSHA204A CryptoAuthentication chip makes communication with the Switch Box very secured. ATSHA204A implements secured handshake and if handshake fails the Switch Box will not executing any command sent to it
- Programming header is FTDI, TTL Serial. For programming you need to have USB TTL Serial – FTDI adaptor
- Battery voltage sensor via Vref internal readings   [link to article](https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/)
- MySensors compatible. You can use superb set of scripts and libraries from [mysensors.org](http://www.mysensors.org) project  and convert the nodes into reliable network of little home sensors, relays and actuators.
- LED indicator
- Battery connectors: 2 x AAA batteries
- Reverse polarity protection
- Sleep current is ~1 uA
- Expected life time 10 years if good batteries used (reporting door openning every hour 24 hrs a day for 10 years).

Comes with Arduino example sketch door.ino. It uses [MySensors](https://www.mysensors.org/) API.

## Overview
If you are familiar with Arduino boards, the Switch Box is Arduino pro Mini with onboard HopeRF RFM69-CW radio and crypto authentication chip. To save some board dimensions the Switch Box does not have onboard USB-Serial converter. You need to buy FTDI Adapter for programming. HopeRF RFM69-CW provide extremely good range. Hundreds of meters in open areas. Onboard Atmel ATSHA204A CryptoAuthentication chip secures communication with other controllers. Some IOT ideas based on the Switch Box and similar controllers plus complete specs you may find here [my GitHub page](https://github.com/EasySensors/DoorWindowSensor)

![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_enclosure.jpg?raw=true)
![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_pcb_with_enclosure.jpg?raw=true)
![arduino Door Window Sensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_top_rfm69.jpg?raw=true)
![arduino Door Window wSensor](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/DoorWindowSensor_bottom_nrf.jpg?raw=true)

**Pin out:** 

Arduino Pins|	Description
------------|--------------
A6 |	Connected to Battery voltage sensor (via divider) 
D5 | green LED
D6 | red LED
D3 Interrupt1 | Connected to magnet (hall) sensor
Interrupt 1 | Occurs when sensor detects change in the magnetic field
A3 |	Connected to  ATSHA204A



The board is created by  [Koresh](https://www.openhardware.io/user/143/projects/Koresh)

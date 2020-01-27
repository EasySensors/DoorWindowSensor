

Decoder part

function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};
 
  if ( port === 1 ) 
	  decoded.magnet = parseInt(bytes);
  if ( port === 2 ) 
	  decoded.temp = parseInt(bytes)/100;
  if ( port === 3 ) 
	  decoded.humidity = parseInt(bytes);

return decoded;
}


![ABP settings](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/decoder.png?raw=true)


Arduino sketch Keys values

![ABP settings](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/ABPsetup.png?raw=true)

![ABP keys](https://github.com/EasySensors/DoorWindowSensor/blob/master/pics/APPsessionKeys.png?raw=true)

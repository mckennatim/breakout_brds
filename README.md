# Breadkout Boards
(git push from powershell terminal)

[FC-28 moisture (analog or digital)](#FC-28_moisture-(analog|digital))

[ds18b20 1-wire temp](#DS18B20_1-wire-temp)

[DHT11 / DHT22](#DHT11-DHT22_temp-hum)


## sensors

### FC-28_moisture-(analog|digital)
The analog output produces a value from 0-1023.

Can alternatively be used as a digital sensor. The module also contains a potentiometer, which will set the threshold value. This threshold value will be compared by the LM393 comparator. The output LED will light up and down according to this threshold value.

* Analog output (AO): This pin generates an analog signal and must be connected to an analog input of the microcontroller.
* Digital output (DO): This pin generates a digital signal and must be connected to a digital input of the microcontroller.
* VCC: Pin to supply power to the sensor (3.3 volts (V)-5 V).
* Ground (GND): Ground connection.

      "type": "se",
      "senses": "moisture",
      "model": "FC-28",
      "in": "A0" || "D*"

![STM32](FC-28moisture.png)

#### refs
* https://maker.pro/arduino/projects/arduino-soil-moisture-sensor

### ds18b20_1-wire-temp
The DS18B20 digital thermometer provides temperature measurements  The DS18B20 communicates
over a 1-Wire bus that by definition requires only one
data line (and ground) for communication with a central
microprocessor. 
Each DS18B20 has a unique 64-bit serial code, which
allows multiple DS18B20s to function on the same 1-Wire
bus. 

Config below shows an se and a cs sharing a 1-wire input.

    {
      "sr": 0,
      "type": "se",
      "senses": "temp"
      "model": "DS18B20", 
      "in": "D3",
      "reading": 68
    },
    {
      "sr": 1,
      "type": "cs",
      "senses": "temp"
      "model": "DS18B20", 
      "in": "D3",
      "out" "D4",
      "reading": 68,
      "onoff": 0,
      "hi": 69,
      "lo": 67
    }


![ds18b20](ds18b20_temp.png)
![ds18b20](ds18b20_circuit.png)

#### refs
* [datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf)
* https://docs.devicehive.com/v2.0/docs/ds18b20-and-esp8266

### DHT11-DHT22_temp-hum
The DHT22 is the more costly model, but it offers superior characteristics. It has a temperature measurement range of -40°C to +125°C with +-0.5°C precision, whereas the DHT12 has a temperature range of -20°C to 60°C with +-0.5°C accuracy and the DHT11 has a temperature range of 0°C to 50°C with +-2 degrees accuracy. The DHT22 sensor also has a greater humidity measurement range, ranging from 0 to 100 percent with 2-5 percent accuracy, compared to the DHT12 humidity range of 20 to 95 percent with 5 percent accuracy and the DHT11 humidity range of 20 to 80 percent with 5 percent accuracy.

DHT11/22 uses an external approximately 10k pull up resistor, so that the state is high when the bus is idle.

    {
      "sr": 0,
      "type": "se",
      "senses": "temp-hum"
      "model": "DHT22", 
      "in": "D3",
      "reading": 68
    },
    {
      "sr": 1,
      "type": "cs",
      "senses": "temp-hum"
      "model": "DHT22", 
      "in": "D3",
      "out": "D4",
      "reading": 48,
      "onoff": 0,
      "hi": 90,
      "lo": 60
    },


![dht11-dht22](dht11-dht22_temp-hum.png)
####  refs
* https://www.hnhcart.com/blogs/sensors-modules/dht11-vs-dht12-vs-dht22
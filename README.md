# Breadkout Boards

## sensors

### FC-28 moisture (analog or digital)
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
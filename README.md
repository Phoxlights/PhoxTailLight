PhoxTailLight
------------
Phox TailLight is a bicycle taillight built around an RGB LED ring that can display custom running light animations, brake lights and turn signal animations. It's wifi enabled so connecting new components is a snap. Plus, use your phone to configure it. Tight!  

Development
------------
There are a few tools required to develop this stuff:
* \*Nix box (for now)
* [makeEspArduino makefile](https://github.com/plerup/makeEspArduino
* [ArduinoEsp](https://github.com/esp8266/Arduino)

First, pull this repo. Be sure to include `--recursive` so that it will pull down submodules:

    git pull --recursive https://github.com/Phoxlights/PhoxTailLight.git

Then update the makefile to point to your local copy of the makeEspArduino and the ArduinoEsp repo:

    ESP_MAKE=$(HOME)/src/makeEspArduino
    ESP_LIBS=$(HOME)/src/ArduinoEsp/libraries

Finally, hook up your esp8266 and try `make upload`. Will it work? Probably!

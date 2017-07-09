# Pickathon_tree_ornaments

All programmed in Arduino IDE

leonardo runs lights, senses for taps and sends data software serial 115200 to the ESP8266 at 115200
data format is 

'[' start data

Cocoon#

tap intensity(0-254);

extr data here (0-254);

extra data here (0-254);

...up to ~20 bytes

']' end data character


ESP8266 receives data from leonardo and HTTP GET request to server at 192.168.16.200
ESP8266 logs onto router SSID IoB2016 password: standard password

teensy3.2 runs web server and receives the data and plays sounds or sends to teensy3.6 to play sounds

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


ESP8266 receives data from leonardo and UDP over wifi server at 192.168.16.200

ESP8266 logs onto router SSID IoB2016 password: standard password

teensy3.2 runs web server and receives the data and plays sounds or sends to teensy3.6 to play sounds

teensy3.2 calculates distance from tapped pendulum and performs a UDP broadcast to all pendulums the following

//Example Data: 91,123,228,43,13,25,24,22,21,19,18,16,15,14,12,11,9,8,7,5,4,2,1,0,1,
// 91 is indicates the beginning of the data
//Next Three Bytes are RGB - 123,228,43
//Next Byte is Intensity - 13 (0-254)
//Next 20 bytes are Distance from Pendulum that was tapped,
//where the 1st byte is the distance between pendulum 1 and the pendulum that was tapped
//... in this case the tapped pendulum is #19 and it is a distance of 25 from the 1st pendulum
//Distance is measured in 10ths of feet so, 25 equates to 2.5 feet.

## An example implementation of a web interface for an Arduino Nano 33 IoT using a websocket (AP version)

This web interface communicates with the Arduino using a websocket on port 8080.

The Arduino serves as both a web server and a websocket server.

The Arduino is set up as a WiFi Access Point; you need to connect to its Access Point before you try to open the web interface page.

### Libraries used:

- https://github.com/ocrdu/WiFiNINA for WiFi connection and servers (**Don't** use https://github.com/arduino-libraries/WiFiNINA 1.4.0 for this sketch)
- https://github.com/ocrdu/NINA-Websocket as the websocket library (Minor change from https://github.com/morrissinger/ESP8266-Websocket);
- https://github.com/ocrdu/Arduino_LSM6DS3_T as the LSM6DS3 (IMU) library (Adds code for LSM6DS3 internal temperature sensor to https://github.com/arduino-libraries/Arduino_LSM6DS3);
- https://github.com/adamvr/arduino-base64 for base64 decoding;
- https://github.com/arduino-libraries/RTCZero as the Real Time Clock library.

### Notes:

There is no documentation yet, but the least you need to do to get it to work is this:

- Edit the wifi_secrets.h to the SSID and password you want to use for the Arduino's WiFi Access Point

This example will then show the interface in your web browser when you point it at the Arduino's IP address  http://192.168.2.1/ .

Almost forgot: if you want to see something happen, put a LED on pin 12 😎.

If you feel like donating for this, you can do so here: http://ocrdu.nl/donations .
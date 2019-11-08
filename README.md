## An example implementation of a web interface for an Arduino Nano 33 IoT using a websocket (AP version with WiFi toggle switch)

The Arduino serves as both a web server and a websocket server. The idea is that the Arduino can serve up its own interface for changing settings and showing data from sensors, logging, etc. and can also run using those settings with the WiFi switched off and do its job controlling stuff with lower power consumption.

The web interface HTML page communicates with the Arduino using a websocket on port 8080.

The Arduino is set up as a WiFi Access Point; you need to connect to its Access Point before you try to open the web interface page. A version that simply connects to your existing WiFi network can be found here: https://github.com/ocrdu/arduino-webinterface-websocket. Note that this (AP) version is further along in development and features.

### Features:
- The interface webpage is included in the code as a base64-encoded, gzipped HTML page, and can be served by the Arduino;
- The Access Point (and WiFi) can be toggled on and off by pulling pin 2 low for a short while with a push button (or a bit of wire, or a reed switch);
- The Access Point (and WiFi) will be switched off when there has been no connection to the Access Point for more than 5 minutes;
- Settings are saved to flash when you turn WiFi off (or when WiFi switches itself off), and will survive a reset or power down;
- Part of the code in the loop() will always run (in the example, the bit that dims and flashes a LED), and another part will run only when the Access Point is up;
- The RTC on the Arduino is set to the correct time and date when you open the webpage interface.

### Libraries used:

- https://github.com/ocrdu/WiFiNINA for WiFi connection and servers (**Don't** use https://github.com/arduino-libraries/WiFiNINA 1.4.0 for this sketch)
- https://github.com/ocrdu/NINA-Websocket as the websocket library (Minor change from https://github.com/morrissinger/ESP8266-Websocket);
- https://github.com/ocrdu/Arduino_LSM6DS3_T as the LSM6DS3 (IMU) library (Adds code for LSM6DS3 internal temperature sensor to https://github.com/arduino-libraries/Arduino_LSM6DS3);
- https://github.com/ocrdu/arduino-base64 for base64 decoding (changed from https://github.com/adamvr/arduino-base64 to accept const char*);
- https://github.com/arduino-libraries/RTCZero as the Real Time Clock library;
- https://github.com/cmaglie/FlashStorage for storing settings in flash.

### Notes:

There is no documentation yet, but the least you need to do to get it to work is this:

- Install the libraries above;
- Edit the wifi_secrets.h to the SSID and password you want to use for the Arduino's WiFi Access Point.

Connect to the WiFi Access Point (it helps if you switch it on first), and this example will then show the interface in your web browser when you point it at the Arduino's IP address http://192.168.2.1/ .

Almost forgot: if you want to see something happen, put a LED on pin 12 😎 (but don't pull more than 7mA).

If you feel like donating for this, you can do so here: http://ocrdu.nl/donations .
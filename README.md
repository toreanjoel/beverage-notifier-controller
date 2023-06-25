# Beverage Notifier
The controller core code that manages the physical sensors and BLE server connection to advertise and pass data to the client when relevant around temperatures.

# Device
The device used for this is the ESP32 S series DevKitc board (https://www.espressif.com/en/products/devkits/esp32-devkitc/overview). Using another board will result in you needing to use the relevant libraries and code to work along side this project to get the BLE working as this board has all the components packaged.

# Notes
The code for this is written in C++ - It was built for the Arduino framework using Platform.io (https://platformio.org/) - this should still work using Arduino.

Make sure to use the relevant code with what your development env is made for, this will only contain the src of the C++ code and the setup of an env is out of the scope for this. - It should however just be COPY and PASTE if setup correctly.

The final project might not use the board as that might change for the size requirements for the project but this is the base controller used for initial testing

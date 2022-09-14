# Kaiju-ECRFv2

**Idea, development and implementation:** Joel Serna (@JoelSernaMoreno).

**Kaiju API:** ComThings (@ComThingsSAS).

**Discord Group:** https://discord.gg/jECPUtdrnW

The developers and collaborators of this project do not earn money with this. 
You can invite me for a coffee to further develop Low-Cost hacking devices. If you don't invite me for a coffee, nothing happens, I will continue developing devices.

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/E1E614OA5)

## Introduction

Evil Crow RF V2 supports the Kaiju API. You need to create an account and buy a license or some tokens to use this:

**Create account:** https://rolling.pandwarf.com/

**Buy licenses:** https://pandwarf.com/product-category/license/

**Buy tokens:** https://pandwarf.com/product-category/token/

**NOTE:** Kaiju-ECRFv2 is a beta firmware, possibly with bugs because it is not stable yet. You can use the serial monitor to debug errors.

## Installation

1. Install esptool: sudo apt install esptool
2. Install pyserial: sudo pip install pyserial
3. Download and Install the Arduino IDE: https://www.arduino.cc/en/main/software
4. Download Kaiju-ECRFv2 repository: git clone https://github.com/joelsernamoreno/Kaiju-ECRFv2.git
5. Download the ESPAsyncWebServer library in the Arduino library directory: git clone https://github.com/me-no-dev/ESPAsyncWebServer.git
6. Download the AsyncTCP library in the Arduino library directory: git clone https://github.com/me-no-dev/AsyncTCP.git
7. Edit AsyncTCP/src/AsyncTCP.h and change the following:

* #define CONFIG_ASYNC_TCP_USE_WDT 1 to #define CONFIG_ASYNC_TCP_USE_WDT 0 

8. Open Arduino IDE
9. Go to File - Preferences. Locate the field "Additional Board Manager URLs:" Add "https://dl.espressif.com/dl/package_esp32_index.json" without quotes. Click "Ok"
10. Select Tools - Board - Boards Manager. Search for "esp32". Install "esp32 by Espressif system version 1.0.6". Click "Close".
11. Open the Kaiju-ECRFv2/Kaiju-ECRFv2/Kaiju-ECRFv2.ino sketch
12. Select Tools:
    * Board - "ESP32 Dev Module".
    * Flash Size - "4MB (32Mb)".
    * CPU Frequency - "80MHz (WiFi/BT)".
    * Flash Frequency - "40MHz"
    * Flash Mode - "DIO"
13. Upload the code to the Evil Crow RF V2 device
14. Copy the Kaiju-ECRFv2/SD/HTML folder to a MicroSD card.

**NOTE:** The Kaiju-ECRFv2 firmware requires an internet connection. Evil Crow RF V2 is configured in STATION mode, you will need to set up a WiFi access point with a mobile phone or router. Evil Crow RF V2 will connect to it to access the internet.

By default, you have to configure a WiFi access point with the following information:

* SSID: testing
* Password: 123456789

You can edit the firmware to change the configuration of the WiFi access point.

## Demo

* Rolling code analyzer & generator with Evil Crow RF V2 and Kaiju: https://www.youtube.com/watch?v=XRhHKfoptkQ


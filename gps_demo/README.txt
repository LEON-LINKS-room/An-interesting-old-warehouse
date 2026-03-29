ESP32-S3 WEB GPS Display

This example demonstrates how to implement a web server on ESP32-S3 to display real-time GPS data. The hardware consists of an ESP32-S3 module and a NEO-M8N positioning module. This functionality is derived from an open-source project I developed during college: a portable outdoor expedition device.

Usage

1. Compile the code and flash it to your ESP32 module.
2. Use any Wi-Fi enabled device with a web browser to connect to the Wi-Fi network:  
   **SSID:** `Leon-Links-GPS`  
   **Password:** `12345678`
3. Open your browser and navigate to: `192.168.4.1`
4. You will then see the positioning data collected by the ESP32.

A specific example project: gps_demo. It demonstrated this function on the ESP32-S3 chip.
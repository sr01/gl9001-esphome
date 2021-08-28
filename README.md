# gl9001-esphome

Galcon 9001BT irrigation controller component for [ESPHome](https://esphome.io/index.html).  
Designed for ESP32 which has a built-in support for wifi and bluetooth.

## How to build?

run esphome with [Substitutions](https://esphome.io/guides/configuration-types.html#substitutions):

esphome -s device_ip_address **1.2.3.4** -s api_password **password** -s ota_password **password** -s wifi_ssid **wifi-name** -s wifi_password **password** run .\gl9001_device.yaml

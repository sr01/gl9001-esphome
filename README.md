# gl9001-esphome

Galcon 9001BT irrigation controller component for [ESPHome](https://esphome.io/index.html).  
Designed for ESP32 which has a built-in support for wifi and bluetooth.

## How to build?

1. Install [esphome](https://esphome.io/guides/installing_esphome.html)
2. run esphome with [substitutions](https://esphome.io/guides/configuration-types.html#substitutions):

esphome -s device_name **{device name}** -s device_ip_address **{ip address}** -s api_password **{api password}** -s ota_password **{ota password}** -s wifi_ssid **{wifi name}** -s wifi_password **{wifi password}** run .\gl9001_device.yaml

## Home Assistant Lovelace UI Demo  
see the custom card: [custon-card.yaml](/lovelace/custon-card.yaml)  

![Faucet closed](/lovelace/screenshots/Closed.jpg)  
![Faucet opened](/lovelace/screenshots/Opened.jpg)  

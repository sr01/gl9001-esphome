#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"

namespace esphome {
namespace gl9001 {

namespace espbt = esphome::esp32_ble_tracker;

class GL9001Node : public ble_client::BLEClientNode {
 public:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  uint16_t heartHandle;

  espbt::ESPBTUUID time_service_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x00,
                                      0x02, 0x68, 0xE8});  //"e8680200-9c4b-11e4-b5f7-0002a5d5c51b" TimeService

  espbt::ESPBTUUID heart_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x01,
                                      0x02, 0x68, 0xE8});  //"e8680201-9c4b-11e4-b5f7-0002a5d5c51b" HEART, Write

  esp_err_t heartBeat();

  const char *const TAG = "gl9001_node";
};
}  // namespace gl9001
}  // namespace esphome
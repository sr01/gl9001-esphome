#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "gl9001_node.h"

#include <esp_gattc_api.h>

namespace esphome {
namespace gl9001 {

namespace espbt = esphome::esp32_ble_tracker;

class GL9001OpenDurationSensor : public PollingComponent, public text_sensor::TextSensor, public GL9001Node {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  std::string empty_value = "";
  const char *timeFormat = "%02d:%02d:%02d";
  uint16_t irrigationStatusHandle;

  espbt::ESPBTUUID irrigation_service_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x00,
                                      0x01, 0x68, 0xE8});  //"e8680100-9c4b-11e4-b5f7-0002a5d5c51b" IrrigationService

  espbt::ESPBTUUID irrigation_status_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x02,
                                      0x01, 0x68, 0xE8});  // "e8680102-9c4b-11e4-b5f7-0002a5d5c51b"  Read Notify

  uint32_t hash_base() override;
  std::string parse_irrigation_status_data(uint8_t *value, uint16_t value_len);
  void readFaucetStatus();
};

}  // namespace gl9001
}  // namespace esphome

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

class GL9001FaucetSwitch : public PollingComponent, public switch_::Switch, public GL9001Node {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void write_state(bool state) override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  uint16_t irrigationControlHandle;
  uint16_t irrigationStatusHandle;

  espbt::ESPBTUUID irrigation_service_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x00,
                                      0x01, 0x68, 0xE8});  //"e8680100-9c4b-11e4-b5f7-0002a5d5c51b" IrrigationService

  espbt::ESPBTUUID irrigation_program_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x01,
                                      0x01, 0x68, 0xE8});  // "e8680101-9c4b-11e4-b5f7-0002a5d5c51b"  Read Write

  espbt::ESPBTUUID irrigation_status_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x02,
                                      0x01, 0x68, 0xE8});  // "e8680102-9c4b-11e4-b5f7-0002a5d5c51b"  Read Notify

  espbt::ESPBTUUID irrigation_control_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x03,
                                      0x01, 0x68, 0xE8});  // "e8680103-9c4b-11e4-b5f7-0002a5d5c51b"  Read Write

  void openFaucet();
  void closeFaucet();
  void readFaucetStatus();
  bool parse_irrigation_status_data(uint8_t *value, uint16_t value_len);
};

}  // namespace gl9001
}  // namespace esphome
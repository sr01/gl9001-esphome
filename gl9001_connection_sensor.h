#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "gl9001_node.h"

#include <esp_gattc_api.h>

namespace esphome {
namespace gl9001 {

namespace espbt = esphome::esp32_ble_tracker;

class GL9001ConnectionSensor : public PollingComponent, public binary_sensor::BinarySensor, public GL9001Node {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  uint32_t hash_base() override;
};

}  // namespace gl9001
}  // namespace esphome

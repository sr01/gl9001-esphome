#include "gl9001_connection_sensor.h"

namespace esphome {
namespace gl9001 {

uint32_t GL9001ConnectionSensor::hash_base() { return 412571265UL; }

void GL9001ConnectionSensor::dump_config() {
  LOG_BINARY_SENSOR("", "GL9001ConnectionSensor", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
  LOG_UPDATE_INTERVAL(this);
}

void GL9001ConnectionSensor::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                                 esp_ble_gattc_cb_param_t *param) {
  GL9001Node::gattc_event_handler(event, gattc_if, param);

  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "[%s] Connected successfully!", this->get_name().c_str());
        publish_state(true);
        break;
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "[%s] Disconnected!", this->get_name().c_str());
      publish_state(false);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      this->node_state = espbt::ClientState::Established;
      break;
    }
    default:
      break;
  }
}

void GL9001ConnectionSensor::update() {
  if (!this->parent()->enabled) {
    return;
  }

  bool state = this->node_state == espbt::ClientState::Established;
  publish_state(state);
}

}  // namespace gl9001
}  // namespace esphome
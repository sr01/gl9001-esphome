#include "gl9001_node.h"

namespace esphome {
namespace gl9001 {

void GL9001Node::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        this->heartBeat();
        break;
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      this->heartHandle = 0;

      auto heartChr = this->parent()->get_characteristic(this->time_service_uuid_, this->heart_char_uuid_);
      if (heartChr != nullptr) {
        this->heartHandle = heartChr->handle;
        this->heartBeat();
      }
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      break;
    }
    default:
      break;
  }
}

esp_err_t GL9001Node::heartBeat() {
  ESP_LOGD(TAG, "send heartBeat");
  uint8_t data[] = {1, 2};
  esp_err_t status = esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->heartHandle,
                                              sizeof(data), data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "heartBeat failed, status=%d", status);
  }

  return status;
}
}  // namespace gl9001
}  // namespace esphome
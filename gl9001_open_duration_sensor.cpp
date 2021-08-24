#include "gl9001_open_duration_sensor.h"

namespace esphome {
namespace gl9001 {

uint32_t GL9001OpenDurationSensor::hash_base() { return 424868861UL; }

void GL9001OpenDurationSensor::dump_config() {
  LOG_TEXT_SENSOR("", "GL9001OpenDurationSensor", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
  LOG_UPDATE_INTERVAL(this);
}

void GL9001OpenDurationSensor::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                                   esp_ble_gattc_cb_param_t *param) {
  GL9001Node::gattc_event_handler(event, gattc_if, param);

  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "[%s] Connected successfully!", this->get_name().c_str());
        break;
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "[%s] Disconnected!", this->get_name().c_str());
      this->status_set_warning();
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      this->irrigationStatusHandle = 0;

      auto chr1 =
          this->parent()->get_characteristic(this->irrigation_service_uuid_, this->irrigation_status_char_uuid_);
      if (chr1 == nullptr) {
        this->status_set_warning();
        ESP_LOGW(TAG, "[%s] No faucet status characteristic found at service %s char %s", this->get_name().c_str(),
                 this->irrigation_service_uuid_.to_string().c_str(),
                 this->irrigation_status_char_uuid_.to_string().c_str());
        break;
      }

      this->irrigationStatusHandle = chr1->handle;
      this->node_state = espbt::ClientState::Established;
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      if (param->read.conn_id != this->parent()->conn_id)
        break;
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "[%s] Error reading char at handle %d, status=%d", this->get_name().c_str(), param->read.handle,
                 param->read.status);
        break;
      }
      if (param->read.handle == this->irrigationStatusHandle) {
        ESP_LOGI(TAG, "[%s] reading char at handle %d, status=%d, value[0]: %d, len: %d", this->get_name().c_str(),
                 param->read.handle, param->read.status, (int) param->read.value, param->read.value_len);
        this->status_clear_warning();
        this->publish_state(this->parse_irrigation_status_data(param->read.value, param->read.value_len));
      }
      break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT: {
      if (param->write.conn_id != this->parent()->conn_id)
        break;

      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "[%s] Error writing char at handle %d, status=%d", this->get_name().c_str(), param->write.handle,
                 param->write.status);
        break;
      }

      ESP_LOGD(TAG, "[%s] writing char event, handle=%d, status=%d", this->get_name().c_str(), param->write.handle,
               param->write.status);
      break;
    }
    default:
      break;
  }
}

std::string GL9001OpenDurationSensor::parse_irrigation_status_data(uint8_t *value, uint16_t value_len) {
  if (value_len >= 5) {
    bool open = (value[0] & 1);
    bool manual = (value[1] & 1);
    uint8_t hour = value[2];
    uint8_t minute = value[3];
    uint8_t second = value[4];

    ESP_LOGI(TAG, "faucet status: open=%d, manual=%d, duration: %02d:%02d:%02d", open, manual, hour, minute, second);

    char buf[100];
    snprintf(buf, sizeof(buf), this->timeFormat, hour, minute, second);
    return buf;
  }

  return this->empty_value;
}

void GL9001OpenDurationSensor::update() {
  ESP_LOGW(TAG, "[%s] update", this->get_name().c_str());

  if (this->node_state != espbt::ClientState::Established) {
    ESP_LOGW(TAG, "[%s] Cannot poll, not connected", this->get_name().c_str());
    return;
  }
  if (this->irrigationStatusHandle == 0) {
    ESP_LOGW(TAG, "[%s] Cannot poll, no service or characteristic found", this->get_name().c_str());
    return;
  }

  this->readFaucetStatus();
}

void GL9001OpenDurationSensor::readFaucetStatus() {
  this->heartBeat();

  auto status = esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id, this->irrigationStatusHandle,
                                        ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    this->status_set_warning();
    ESP_LOGW(TAG, "[%s] Error sending read request for faucet status, status=%d", this->get_name().c_str(), status);
  }
}

}  // namespace gl9001
}  // namespace esphome
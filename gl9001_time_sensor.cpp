#include "gl9001_time_sensor.h"

namespace esphome {
namespace gl9001 {

uint32_t GL9001TimeSensor::hash_base() { return 442878864UL; }

void GL9001TimeSensor::setup() { register_service(&GL9001TimeSensor::on_sync_time, "syncTime"); }

void GL9001TimeSensor::dump_config() {
  LOG_TEXT_SENSOR("", "GL9001TimeSensor", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
  LOG_UPDATE_INTERVAL(this);
}

void GL9001TimeSensor::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
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
      this->getTimeHandle = 0;
      this->setTimeHandle = 0;

      auto chr1 = this->parent()->get_characteristic(this->time_service_uuid_, this->get_time_char_uuid_);
      if (chr1 == nullptr) {
        this->status_set_warning();
        this->publish_state(this->empty_value);
        ESP_LOGW(TAG, "[%s] No sensor characteristic found at service %s char %s", this->get_name().c_str(),
                 this->time_service_uuid_.to_string().c_str(), this->get_time_char_uuid_.to_string().c_str());
        break;
      }

      auto chr2 = this->parent()->get_characteristic(this->time_service_uuid_, this->set_time_char_uuid_);
      if (chr2 == nullptr) {
        this->status_set_warning();
        this->publish_state(this->empty_value);
        ESP_LOGW(TAG, "[%s] No sensor characteristic found at service %s char %s", this->get_name().c_str(),
                 this->time_service_uuid_.to_string().c_str(), this->set_time_char_uuid_.to_string().c_str());
        break;
      }

      this->getTimeHandle = chr1->handle;
      this->setTimeHandle = chr2->handle;
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
      if (param->read.handle == this->getTimeHandle) {
        this->status_clear_warning();
        this->publish_state(this->parse_data(param->read.value, param->read.value_len));
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

      break;
    }
    default:
      break;
  }
}

std::string GL9001TimeSensor::parse_data(uint8_t *value, uint16_t value_len) {
  if (value_len == 8) {
    uint8_t hour = value[4];
    uint8_t minute = value[5];
    uint8_t second = value[6];
    uint8_t dayOfWeek = value[7];

    char buf[100];
    snprintf(buf, sizeof(buf), this->timeFormat, hour, minute, second, dayOfWeek);
    return buf;
  } else {
    return this->empty_value;
  }
}

void GL9001TimeSensor::update() {
  if (!this->parent()->enabled) {
    return;
  }

  if (this->node_state != espbt::ClientState::Established) {
    ESP_LOGW(TAG, "[%s] Cannot poll, not connected", this->get_name().c_str());
    return;
  }
  if (this->getTimeHandle == 0) {
    ESP_LOGW(TAG, "[%s] Cannot poll, no service or characteristic found", this->get_name().c_str());
    return;
  }

  this->heartBeat();

  auto status = esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id, this->getTimeHandle,
                                        ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    this->status_set_warning();
    this->publish_state(this->empty_value);
    ESP_LOGW(TAG, "[%s] Error sending read request for sensor, status=%d", this->get_name().c_str(), status);
  }
}

void GL9001TimeSensor::on_sync_time() {
  ESP_LOGD(TAG, "sync time");

  if (this->node_state != espbt::ClientState::Established) {
    ESP_LOGW(TAG, "[%s] Cannot sync time, not connected", this->get_name().c_str());
    return;
  }

  if (is_connected()) {
    if (this->rtc) {
      this->heartBeat();

      time::ESPTime espTime = this->rtc->now();
      uint8_t hour = espTime.hour;
      uint8_t minute = espTime.minute;
      uint8_t second = espTime.second;
      uint8_t day_of_week = espTime.day_of_week;
      ESP_LOGD(TAG, "[%s] sync time to %02d:%02d:%02d, day of week: %d", this->get_name().c_str(), hour, minute, second,
               day_of_week);

      uint8_t data[] = {0, 0, 0, 0, hour, minute, second, day_of_week};

      esp_err_t status =
          esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->setTimeHandle, sizeof(data),
                                   data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
      if (status) {
        ESP_LOGE(TAG, "[%s] set time failed, status=%d", this->get_name().c_str(), status);
      }
    } else {
      ESP_LOGW(TAG, "[%s] can't sync time, no time component set.", this->get_name().c_str());
    }
  }
}

}  // namespace gl9001
}  // namespace esphome
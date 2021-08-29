#include "gl9001_faucet_switch.h"

namespace esphome {
namespace gl9001 {

void GL9001FaucetSwitch::dump_config() {
  LOG_SWITCH("", "GL9001FaucetSwitch", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
}

void GL9001FaucetSwitch::write_state(bool state) {
  // This will be called every time the user requests a state change.

  if (state) {
    this->openFaucet();
  } else {
    this->closeFaucet();
  }
}

void GL9001FaucetSwitch::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
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
      this->irrigationControlHandle = 0;
      this->irrigationStatusHandle = 0;

      auto chr1 =
          this->parent()->get_characteristic(this->irrigation_service_uuid_, this->irrigation_control_char_uuid_);
      if (chr1 == nullptr) {
        this->status_set_warning();
        ESP_LOGW(TAG, "[%s] No faucet control characteristic found at service %s char %s", this->get_name().c_str(),
                 this->irrigation_service_uuid_.to_string().c_str(),
                 this->irrigation_control_char_uuid_.to_string().c_str());
        break;
      }

      auto chr2 =
          this->parent()->get_characteristic(this->irrigation_service_uuid_, this->irrigation_status_char_uuid_);
      if (chr2 == nullptr) {
        this->status_set_warning();
        ESP_LOGW(TAG, "[%s] No faucet status characteristic found at service %s char %s", this->get_name().c_str(),
                 this->irrigation_service_uuid_.to_string().c_str(),
                 this->irrigation_status_char_uuid_.to_string().c_str());
        break;
      }

      this->irrigationControlHandle = chr1->handle;
      this->irrigationStatusHandle = chr2->handle;
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

      break;
    }
    default:
      break;
  }
}

void GL9001FaucetSwitch::openFaucet() {
  this->heartBeat();

  ESP_LOGD(TAG, "[%s] send open faucet", this->get_name().c_str());

  uint8_t hours = 0;    // TODO: move to config
  uint8_t minutes = 5;  // TODO: move to config
  uint8_t data[] = {0, 3, 0, hours, minutes, 0, 0};

  esp_err_t status =
      esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->irrigationControlHandle,
                               sizeof(data), data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGE(TAG, "[%s] open faucet failed, status=%d", this->get_name().c_str(), status);
  } else {
    publish_state(true);
  }
}

void GL9001FaucetSwitch::closeFaucet() {
  this->heartBeat();

  ESP_LOGD(TAG, "[%s] send close faucet", this->get_name().c_str());

  uint8_t data[] = {1, 0, 0, 0, 0, 0, 0};

  esp_err_t status =
      esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->irrigationControlHandle,
                               sizeof(data), data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGE(TAG, "[%s] close faucet failed, status=%d", this->get_name().c_str(), status);
  } else {
    publish_state(false);
  }
}

void GL9001FaucetSwitch::readFaucetStatus() {
  this->heartBeat();

  auto status = esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id, this->irrigationStatusHandle,
                                        ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    this->status_set_warning();
    ESP_LOGW(TAG, "[%s] Error sending read request for faucet status, status=%d", this->get_name().c_str(), status);
  }
}

bool GL9001FaucetSwitch::parse_irrigation_status_data(uint8_t *value, uint16_t value_len) {
  if (value_len >= 5) {
    bool open = (value[0] & 1);
    return open;
  }

  return false;
}

void GL9001FaucetSwitch::update() {
  if (!this->parent()->enabled) {
    return;
  }

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

}  // namespace gl9001
}  // namespace esphome
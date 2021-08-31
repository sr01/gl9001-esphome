#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "gl9001_node.h"

#include <esp_gattc_api.h>

namespace esphome {
namespace gl9001 {

namespace espbt = esphome::esp32_ble_tracker;

class GL9001TimeSensor : public PollingComponent,
                         public api::CustomAPIDevice,
                         public text_sensor::TextSensor,
                         public GL9001Node {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void set_time_format(const char *timeFormat) { this->timeFormat = timeFormat; }
  void set_time_component(time::RealTimeClock *rtc) { this->rtc = rtc; }

  uint16_t getTimeHandle;
  uint16_t setTimeHandle;

 protected:
  std::string empty_value = "--:--:--";
  const char *timeFormat = "%02d:%02d:%02d";
  time::RealTimeClock *rtc = NULL;

  espbt::ESPBTUUID time_service_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x00,
                                      0x02, 0x68, 0xE8});  //"e8680200-9c4b-11e4-b5f7-0002a5d5c51b" TimeService
  espbt::ESPBTUUID get_time_char_uuid_ = espbt::ESPBTUUID::from_raw((uint8_t *) (const uint8_t[16]){
      0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x02, 0x02, 0x68,
      0xE8});  //"e8680202-9c4b-11e4-b5f7-0002a5d5c51b" getTime , Read Notify

  espbt::ESPBTUUID set_time_char_uuid_ = espbt::ESPBTUUID::from_raw(
      (uint8_t *) (const uint8_t[16]){0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xF7, 0xB5, 0xE4, 0x11, 0x4B, 0x9C, 0x03,
                                      0x02, 0x68, 0xE8});  //"e8680203-9c4b-11e4-b5f7-0002a5d5c51b"  setTime, Write

  uint32_t hash_base() override;
  std::string parse_data(uint8_t *value, uint16_t value_len);
  void on_sync_time();
};

}  // namespace gl9001
}  // namespace esphome

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "pb_hub.h"

#define PBHUB_TAG "PBHUB"

#define PB_HUB_SDA (GPIO_NUM_32)
#define PB_HUB_SCL (GPIO_NUM_33)
#define PB_HUB_I2C_CLK (100 * 1000)
#define PB_HUB_I2C I2C_NUM_1
#define PB_HUB_I2C_ADDR (uint8_t)(0x61<<1)

const uint8_t PB_READ_DIGITAL[6][2] = {
  { 0x44, 0x45 },
  { 0x54, 0x55 },
  { 0x64, 0x65 },
  { 0x74, 0x75 },
  { 0x84, 0x85 },
  { 0xa4, 0xa5 }
};
const uint8_t PB_WRITE_DIGITAL[6][2] = { 
  { 0x40, 0x41 },
  { 0x50, 0x51 },
  { 0x60, 0x61 },
  { 0x70, 0x71 },
  { 0x80, 0x81 },
  { 0xa0, 0xa1 }
};
const uint8_t PB_READ_ANALOG[6] = { 
  0x46,
  0x56,
  0x66,
  0x76,
  0x86,
  0xa6
};
const uint8_t PB_WRITE_ANALOG[6][2] = { 
  { 0x42, 0x43 },
  { 0x52, 0x53 },
  { 0x62, 0x63 },
  { 0x72, 0x73 },
  { 0x82, 0x83 },
  { 0xa2, 0xa3 }
};

esp_err_t pb_hub_init(void)
{
  esp_err_t err;
  i2c_config_t i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = PB_HUB_SDA,
    .scl_io_num = PB_HUB_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = PB_HUB_I2C_CLK
  };
  err = i2c_param_config(PB_HUB_I2C, &i2c_config);
  if (err != ESP_OK) {
    return err;
  }
  err = i2c_driver_install(PB_HUB_I2C, I2C_MODE_MASTER, 0, 0, 0);
  if (err != ESP_OK) {
    return err;
  }
  // ESP_LOGI(PBHUB_TAG, "ticks = %d", pdMS_TO_TICKS(4000000));
  i2c_set_timeout(PB_HUB_I2C, 400000); // ??
  return ESP_OK;
}

esp_err_t pb_hub_deinit(void)
{
  return i2c_driver_delete(PB_HUB_I2C);
}

uint8_t pb_hub_digital_read(pb_hub_channel_t ch, pb_hub_io_t io)
{
  esp_err_t err = ESP_OK;
  uint8_t value = PB_READ_DIGITAL[ch][io];
  uint8_t v = 0x00;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, value, true);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &v, false);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(PB_HUB_I2C, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  ESP_LOGV(PBHUB_TAG, "error = %d", err);
  ESP_ERROR_CHECK(err);
  return v;
}

void pb_hub_digital_write(pb_hub_channel_t ch, pb_hub_io_t io, uint8_t value)
{
  esp_err_t err = ESP_OK;
  uint8_t v = PB_WRITE_DIGITAL[ch][io];
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, v, true);
  i2c_master_write_byte(cmd, value, false);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(PB_HUB_I2C, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  ESP_LOGV(PBHUB_TAG, "error = %d", err);
  ESP_ERROR_CHECK(err);
}

void pb_hub_analog_read(pb_hub_channel_t ch, uint8_t *value)
{
  esp_err_t err = ESP_OK;
  uint8_t v = PB_READ_ANALOG[ch];
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, v, true);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, value, I2C_MASTER_ACK);
  i2c_master_read_byte(cmd, value+1, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(PB_HUB_I2C, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  ESP_LOGV(PBHUB_TAG, "error = %d", err);
  ESP_ERROR_CHECK(err);
}

void pb_hub_analog_write(pb_hub_channel_t ch, pb_hub_io_t io, uint16_t value)
{
  esp_err_t err = ESP_OK;
  uint8_t v = PB_WRITE_ANALOG[ch][io];
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, PB_HUB_I2C_ADDR | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, v, true);
  i2c_master_write_byte(cmd, value, false);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(PB_HUB_I2C, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  ESP_LOGV(PBHUB_TAG, "error = %d", err);
  ESP_ERROR_CHECK(err);
}

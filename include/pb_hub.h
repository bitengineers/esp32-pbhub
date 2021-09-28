#pragma once

typedef enum {
  PB_CH0 = 0,
  PB_CH1 = 1,
  PB_CH2 = 2,
  PB_CH3 = 3,
  PB_CH4 = 4,
  PB_CH5 = 5
} pb_hub_channel_t;

typedef enum {
  PB_IO0 = 0,
  PB_IO1 = 1,
} pb_hub_io_t;

esp_err_t pb_hub_init(void);
esp_err_t pb_hub_deinit(void);

uint8_t pb_hub_digital_read(pb_hub_channel_t ch, pb_hub_io_t io);
void pb_hub_digital_write(pb_hub_channel_t ch, pb_hub_io_t io, uint8_t value);

void pb_hub_analog_read(pb_hub_channel_t ch, uint8_t *value);
void pb_hub_analog_write(pb_hub_channel_t ch, pb_hub_io_t io, uint16_t value);

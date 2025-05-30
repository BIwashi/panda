#pragma once

#include "board_declarations.h"

// ///////////////////// //
// White Panda (STM32F4) //
// ///////////////////// //

static void white_enable_can_transceiver(uint8_t transceiver, bool enabled) {
  switch (transceiver){
    case 1U:
      set_gpio_output(GPIOC, 1, !enabled);
      break;
    case 2U:
      set_gpio_output(GPIOC, 13, !enabled);
      break;
    case 3U:
      set_gpio_output(GPIOA, 0, !enabled);
      break;
    default:
      break;
  }
}

static void white_set_usb_power_mode(uint8_t mode){
  switch (mode) {
    case USB_POWER_CLIENT:
      // B2,A13: set client mode
      set_gpio_output(GPIOB, 2, 0);
      set_gpio_output(GPIOA, 13, 1);
      break;
    case USB_POWER_CDP:
      // B2,A13: set CDP mode
      set_gpio_output(GPIOB, 2, 1);
      set_gpio_output(GPIOA, 13, 1);
      break;
    case USB_POWER_DCP:
      // B2,A13: set DCP mode on the charger (breaks USB!)
      set_gpio_output(GPIOB, 2, 0);
      set_gpio_output(GPIOA, 13, 0);
      break;
    default:
      print("Invalid usb power mode\n");
      break;
  }
}

static void white_set_can_mode(uint8_t mode){
  if (mode == CAN_MODE_NORMAL) {
    // B12,B13: disable GMLAN mode
    set_gpio_mode(GPIOB, 12, MODE_INPUT);
    set_gpio_mode(GPIOB, 13, MODE_INPUT);

    // B3,B4: disable GMLAN mode
    set_gpio_mode(GPIOB, 3, MODE_INPUT);
    set_gpio_mode(GPIOB, 4, MODE_INPUT);

    // B5,B6: normal CAN2 mode
    set_gpio_alternate(GPIOB, 5, GPIO_AF9_CAN2);
    set_gpio_alternate(GPIOB, 6, GPIO_AF9_CAN2);

    // A8,A15: normal CAN3 mode
    set_gpio_alternate(GPIOA, 8, GPIO_AF11_CAN3);
    set_gpio_alternate(GPIOA, 15, GPIO_AF11_CAN3);
  }
}

static uint32_t white_read_voltage_mV(void){
  return adc_get_mV(12) * 11U;
}

static uint32_t white_read_current_mA(void){
  // This isn't in mA, but we're keeping it for backwards compatibility
  return adc_get_raw(13);
}

static bool white_check_ignition(void){
  // ignition is on PA1
  return !get_gpio_input(GPIOA, 1);
}

static void white_grey_init(void) {
  common_init_gpio();

  // C3: current sense
  set_gpio_mode(GPIOC, 3, MODE_ANALOG);

  // A1: started_alt
  set_gpio_pullup(GPIOA, 1, PULL_UP);

  // A2, A3: USART 2 for debugging
  set_gpio_alternate(GPIOA, 2, GPIO_AF7_USART2);
  set_gpio_alternate(GPIOA, 3, GPIO_AF7_USART2);

  // A4, A5, A6, A7: SPI
  set_gpio_alternate(GPIOA, 4, GPIO_AF5_SPI1);
  set_gpio_alternate(GPIOA, 5, GPIO_AF5_SPI1);
  set_gpio_alternate(GPIOA, 6, GPIO_AF5_SPI1);
  set_gpio_alternate(GPIOA, 7, GPIO_AF5_SPI1);

  // B12: GMLAN, ignition sense, pull up
  set_gpio_pullup(GPIOB, 12, PULL_UP);

  /* GMLAN mode pins:
      M0(B15)  M1(B14)  mode
      =======================
      0        0        sleep
      1        0        100kbit
      0        1        high voltage wakeup
      1        1        33kbit (normal)
  */
  set_gpio_output(GPIOB, 14, 0);
  set_gpio_output(GPIOB, 15, 0);

  // B7: K-line enable
  set_gpio_output(GPIOB, 7, 1);

  // C12, D2: Setup K-line (UART5)
  set_gpio_alternate(GPIOC, 12, GPIO_AF8_UART5);
  set_gpio_alternate(GPIOD, 2, GPIO_AF8_UART5);
  set_gpio_pullup(GPIOD, 2, PULL_UP);

  // L-line enable
  set_gpio_output(GPIOA, 14, 1);

  // C10, C11: L-Line setup (USART3)
  set_gpio_alternate(GPIOC, 10, GPIO_AF7_USART3);
  set_gpio_alternate(GPIOC, 11, GPIO_AF7_USART3);
  set_gpio_pullup(GPIOC, 11, PULL_UP);

  // Init usb power mode
  // Init in CDP mode only if panda is powered by 12V.
  // Otherwise a PC would not be able to flash a standalone panda
  if (white_read_voltage_mV() > 8000U) {  // 8V threshold
    white_set_usb_power_mode(USB_POWER_CDP);
  } else {
    white_set_usb_power_mode(USB_POWER_CLIENT);
  }

  // ESP/GPS off
  set_gpio_output(GPIOC, 5, 0);
  set_gpio_output(GPIOC, 14, 0);
}

static void white_grey_init_bootloader(void) {
  // ESP/GPS off
  set_gpio_output(GPIOC, 5, 0);
  set_gpio_output(GPIOC, 14, 0);
}

static harness_configuration white_harness_config = {
  .has_harness = false
};

board board_white = {
  .set_bootkick = unused_set_bootkick,
  .harness_config = &white_harness_config,
  .has_spi = false,
  .has_canfd = false,
  .fan_max_rpm = 0U,
  .fan_max_pwm = 100U,
  .avdd_mV = 3300U,
  .fan_stall_recovery = false,
  .fan_enable_cooldown_time = 0U,
  .init = white_grey_init,
  .init_bootloader = white_grey_init_bootloader,
  .enable_can_transceiver = white_enable_can_transceiver,
  .led_GPIO = {GPIOC, GPIOC, GPIOC},
  .led_pin = {9, 7, 6},
  .set_can_mode = white_set_can_mode,
  .check_ignition = white_check_ignition,
  .read_voltage_mV = white_read_voltage_mV,
  .read_current_mA = white_read_current_mA,
  .set_fan_enabled = unused_set_fan_enabled,
  .set_ir_power = unused_set_ir_power,
  .set_siren = unused_set_siren,
  .read_som_gpio = unused_read_som_gpio,
  .set_amp_enabled = unused_set_amp_enabled
};

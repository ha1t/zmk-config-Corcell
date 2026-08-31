/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define POWER_ON_LED_NODE DT_ALIAS(led1)

#if !DT_NODE_EXISTS(POWER_ON_LED_NODE)
#error "CONFIG_CORCELL_POWER_ON_LED needs an led1 alias on the board"
#endif

static const struct gpio_dt_spec power_on_led = GPIO_DT_SPEC_GET(POWER_ON_LED_NODE, gpios);

static void power_on_led_release(struct k_work *work) {
    /* Park the pin rather than driving it, so it costs nothing once dark. */
    int ret = gpio_pin_configure_dt(&power_on_led, GPIO_DISCONNECTED);
    if (ret < 0) {
        LOG_WRN("Failed to release the power-on LED: %d", ret);
    }
}

static K_WORK_DELAYABLE_DEFINE(power_on_led_work, power_on_led_release);

static int power_on_led_init(void) {
    if (!gpio_is_ready_dt(&power_on_led)) {
        LOG_WRN("Power-on LED is not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&power_on_led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_WRN("Failed to light the power-on LED: %d", ret);
        return ret;
    }

    k_work_schedule(&power_on_led_work, K_MSEC(CONFIG_CORCELL_POWER_ON_LED_MS));
    return 0;
}

SYS_INIT(power_on_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

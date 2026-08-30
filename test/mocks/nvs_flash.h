
/** @file nvs_flash.h
 *  @brief Native-test NVS compatibility shim.
 */

#pragma once

typedef int esp_err_t;
#define ESP_OK 0

/**
 * @brief Erases the mock non-volatile storage.
 * @param None.
 * @return ESP_OK.
 */
inline esp_err_t nvs_flash_erase() { return ESP_OK; }

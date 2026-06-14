#include "driver/i2c_master.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "example";

//I2C Port Configuration
#define I2C_SENSOR_BUS_PORT     1
#define I2C_SENSOR_SDA          33
#define I2C_SENSOR_SCL          32

//I2C Bus Handler and Configuration
i2c_master_bus_handle_t i2c_sensor_bus = NULL;
i2c_master_bus_config_t sensor_bus_config =
{
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .i2c_port = I2C_SENSOR_BUS_PORT,
    .sda_io_num = I2C_SENSOR_SDA,
    .scl_io_num = I2C_SENSOR_SCL,
    .flags.enable_internal_pullup = true,
};

//I2C for SMP3011 Pressure Sensor configuration and handler
i2c_master_dev_handle_t i2c_smp3011_handle = NULL;
i2c_device_config_t i2c_smp3011_config =
{
    .dev_addr_length  = I2C_ADDR_BIT_LEN_7,
    .device_address  = 0x78,
    .scl_speed_hz  = 100000,   // 100kHz: pull-ups internos (45kΩ) não sustentam 400kHz
    .scl_wait_us = 0,
    .flags =
    {
        .disable_ack_check = 0
    }
};

/*
    PROTOTYPES
*/
void smp3011Init();
void smp3011Poll();


void app_main()
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    ESP_ERROR_CHECK(i2c_new_master_bus(&sensor_bus_config, &i2c_sensor_bus));

    printf("Scanning I2C bus...\n");
    for (int i = 1; i < 127; i++)
    {
        esp_err_t err = i2c_master_probe(i2c_sensor_bus, i, 50);
        if (err == ESP_OK)
        {
            printf("Found device at 0x%02x\n", i);
        }
    }
    printf("Scan done.\n");

    smp3011Init();

    while(1)
    {
        smp3011Poll();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void smp3011Init()
{
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_sensor_bus, &i2c_smp3011_config, &i2c_smp3011_handle));

    uint8_t PressSensorCommand = 0xAC;
    esp_err_t ret = i2c_master_transmit(i2c_smp3011_handle, &PressSensorCommand, 1, 100);
    if (ret != ESP_OK)
        ESP_LOGE(TAG, "smp3011Init transmit failed: %s", esp_err_to_name(ret));

    vTaskDelay(pdMS_TO_TICKS(100));
}

void smp3011Poll()
{
    uint8_t PressSensorBuffer[6];
    esp_err_t ret = i2c_master_receive(i2c_smp3011_handle, PressSensorBuffer, sizeof(PressSensorBuffer), 50);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "smp3011Poll receive failed: %s", esp_err_to_name(ret));
        // Recupera o bus e reinicia a medição
        i2c_master_bus_reset(i2c_sensor_bus);
        vTaskDelay(pdMS_TO_TICKS(20));
        uint8_t cmd = 0xAC;
        i2c_master_transmit(i2c_smp3011_handle, &cmd, 1, 100);
        vTaskDelay(pdMS_TO_TICKS(10));
        return;
    }

    printf("Raw Data: %02X %02X %02X %02X %02X %02X\n",
           PressSensorBuffer[0], PressSensorBuffer[1], PressSensorBuffer[2],
           PressSensorBuffer[3], PressSensorBuffer[4], PressSensorBuffer[5]);

    if ((PressSensorBuffer[0] & 0x20) == 0)
    {
        uint8_t PressSensorCommand = 0xAC;
        ret = i2c_master_transmit(i2c_smp3011_handle, &PressSensorCommand, 1, 100);
        if (ret != ESP_OK)
            ESP_LOGE(TAG, "smp3011Poll transmit failed: %s", esp_err_to_name(ret));

        float pressurePercentage = (float)(((uint32_t)PressSensorBuffer[1] << 16) |
                                           ((uint32_t)PressSensorBuffer[2] << 8)  |
                                            (uint32_t)PressSensorBuffer[3]);
        pressurePercentage = (pressurePercentage / 16777215.0f);
        pressurePercentage -= 0.15f;
        pressurePercentage /= 0.7f;
        pressurePercentage *= 500000.0f;

        float temperaturePercentage = (float)(((uint32_t)PressSensorBuffer[4] << 8) |
                                               (uint32_t)PressSensorBuffer[5]);
        temperaturePercentage /= 65535.0f;
        temperaturePercentage = (190.0f * temperaturePercentage) - 40.0f;

        printf("Pressure: %.2f Pa  Temperature: %.2f C\n", pressurePercentage, temperaturePercentage);
    }
    else
    {
        ESP_LOGW(TAG, "Sensor ocupado (status=0x%02X), aguardando...", PressSensorBuffer[0]);
    }
}

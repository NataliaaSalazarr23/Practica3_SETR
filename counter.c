#include "counter.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "COUNTER";

static void counter_step(void)
{
    if (g_system.direction == COUNT_UP)
    {
        g_system.value++;

        if (g_system.value > 9)
        {
            g_system.value = 0;
        }
    }
    else
    {
        if (g_system.value == 0)
        {
            g_system.value = 9;
        }
        else
        {
            g_system.value--;
        }
    }
}


void counter_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        ESP_LOGI(
            TAG,
            "Valor=%u | Direccion=%s | Periodo=%lu ms",
            g_system.value,
            g_system.direction == COUNT_UP ? "UP" : "DOWN",
            (unsigned long)g_system.period_ms
        );

        vTaskDelay(
            pdMS_TO_TICKS(g_system.period_ms)
        );

        counter_step();
    }
}
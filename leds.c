#include "leds.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

/* Tarea encargada de controlar un LED individual.
   Cada LED recibe sus parámetros mediante pvParameters. */
void led_task(void *pvParameters)
{
    /* Se convierte el parámetro genérico recibido a la estructura
       específica de configuración del LED. */
    LedTaskParams_t *cfg = (LedTaskParams_t *)pvParameters;

    /* Configura el GPIO asignado al LED como salida digital. */
    gpio_reset_pin((gpio_num_t)cfg->gpio);
    gpio_set_direction((gpio_num_t)cfg->gpio, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)cfg->gpio, 0);

    while (1)
    {
        uint8_t bit_value;

        /* Extrae el bit correspondiente del valor global del contador.
           Cada LED representa un bit del número BCD. */
        bit_value = (g_system.value >> cfg->bit_position) & 0x01;

        /* Envía el valor del bit al GPIO del LED:
           1 enciende el LED, 0 lo apaga. */
        gpio_set_level(
            (gpio_num_t)cfg->gpio,
            bit_value
        );

        /* Pequeño retardo para actualizar el LED periódicamente
           sin consumir el CPU constantemente. */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

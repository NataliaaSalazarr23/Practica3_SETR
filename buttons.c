#include "buttons.h"
#include "app_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

/* Etiqueta utilizada para mensajes de depuración de botones. */
static const char *TAG = "BUTTON";

/* Estructura para almacenar el estado del antirrebote. */
typedef struct
{
    int stable_state;   // Estado estable confirmado del botón
    int last_raw;       // Última lectura directa del GPIO
    int count;          // Contador de lecturas iguales consecutivas

} Debounce_t;

/* Número de lecturas iguales necesarias para aceptar un cambio estable. */
#define DEBOUNCE_COUNT 3

/* Función que aplica antirrebote al botón.
   Devuelve true solo cuando detecta una pulsación válida. */
static bool debounce_update(uint8_t gpio, Debounce_t *db)
{
    int raw;
    bool pressed_event;

    /* Lee el estado actual del GPIO. */
    raw = gpio_get_level((gpio_num_t)gpio);
    pressed_event = false;

    /* Si la lectura actual es igual a la anterior,
       aumenta el contador de estabilidad. */
    if (raw == db->last_raw)
    {
        if (db->count < DEBOUNCE_COUNT)
        {
            db->count++;
        }
    }
    else
    {
        /* Si la lectura cambió, se reinicia el conteo
           y se guarda la nueva lectura. */
        db->count = 0;
        db->last_raw = raw;
    }

    /* Cuando se alcanza el número mínimo de lecturas iguales,
       se acepta el nuevo estado como estable. */
    if (db->count >= DEBOUNCE_COUNT)
    {
        if (raw != db->stable_state)
        {
            db->stable_state = raw;

            /* Botón con pull-down externo:
               no presionado = 0
               presionado    = 1 */
            if (db->stable_state == 1)
            {
                pressed_event = true;
            }
        }
    }

    return pressed_event;
}

/* Tarea encargada de leer un botón físico. */
void button_task(void *pvParameters)
{
    /* Se reciben los parámetros del botón mediante pvParameters. */
    ButtonTaskParams_t *cfg = (ButtonTaskParams_t *)pvParameters;

    /* Inicialización de la estructura de antirrebote. */
    Debounce_t db =
    {
        .stable_state = 0,
        .last_raw = 0,
        .count = 0
    };

    /* Configura el GPIO del botón como entrada. */
    gpio_reset_pin((gpio_num_t)cfg->gpio);
    gpio_set_direction((gpio_num_t)cfg->gpio, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)cfg->gpio, GPIO_FLOATING);

    while (1)
    {
        /* Si se detecta una pulsación válida, se genera un evento
           para que el Task Manager lo procese. */
        if (debounce_update(cfg->gpio, &db))
        {
            /* El botón no modifica directamente el sistema.
               Solo deja registrado un evento pendiente. */
            switch (cfg->type)
            {
                case BUTTON_START_PAUSE:
                    /* Evento para iniciar o pausar el sistema. */
                    g_system.pending_event = MANAGER_EVENT_START_PAUSE;

                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_DIRECTION:
                    /* Evento para cambiar la dirección del contador. */
                    g_system.pending_event = MANAGER_EVENT_DIRECTION;

                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                case BUTTON_SPEED:
                    /* Evento para cambiar la velocidad del contador. */
                    g_system.pending_event = MANAGER_EVENT_SPEED;

                    ESP_LOGI(TAG, "%s presionado", cfg->name);
                    break;

                default:
                    break;
            }
        }

        /* Tiempo entre lecturas del botón. */
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

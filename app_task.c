#include "app_task.h"

#include "app_config.h"
#include "system_state.h"
#include "leds.h"
#include "buttons.h"
#include "counter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

/* Etiqueta para mensajes del Task Manager. */
static const char *TAG = "MANAGER";

/* Handles de las tareas.
   Permiten suspender, reanudar y consultar el estado de cada tarea. */
static TaskHandle_t h_leds[4];
static TaskHandle_t h_btn_start;
static TaskHandle_t h_btn_dir;
static TaskHandle_t h_btn_speed;
static TaskHandle_t h_counter;
static TaskHandle_t h_manager;

/* Parámetros de configuración para las tareas de LEDs.
   Cada LED recibe su GPIO, posición de bit y nombre. */
static LedTaskParams_t led_params[4] =
{
    { LED_B0, 0, "LED_B0" },
    { LED_B1, 1, "LED_B1" },
    { LED_B2, 2, "LED_B2" },
    { LED_B3, 3, "LED_B3" }
};

/* Parámetros del botón Start/Pause. */
static ButtonTaskParams_t btn_start =
{
    .gpio = BTN_START,
    .name = "BTN_START",
    .type = BUTTON_START_PAUSE
};

/* Parámetros del botón de dirección. */
static ButtonTaskParams_t btn_dir =
{
    .gpio = BTN_DIR,
    .name = "BTN_DIR",
    .type = BUTTON_DIRECTION
};

/* Parámetros del botón de velocidad. */
static ButtonTaskParams_t btn_speed =
{
    .gpio = BTN_SPEED,
    .name = "BTN_SPEED",
    .type = BUTTON_SPEED
};

/* Convierte el estado de una tarea a texto para mostrarlo por UART. */
static const char *state_to_string(eTaskState state)
{
    switch (state)
    {
        case eRunning:
            return "RUNNING";

        case eReady:
            return "READY";

        case eBlocked:
            return "BLOCKED";

        case eSuspended:
            return "SUSPENDED";

        case eDeleted:
            return "DELETED";

        default:
            return "UNKNOWN";
    }
}

/* Pausa el sistema suspendiendo el contador y los botones
   de dirección y velocidad. */
static void manager_pause_system(void)
{
    g_system.mode = SYSTEM_PAUSED;

    /* Se suspende el contador para detener el conteo. */
    vTaskSuspend(h_counter);

    /* Se suspenden los botones de dirección y velocidad
       porque no deben funcionar mientras el sistema está pausado. */
    vTaskSuspend(h_btn_dir);
    vTaskSuspend(h_btn_speed);

    ESP_LOGW(TAG, "Sistema PAUSADO");
}

/* Reanuda el sistema. */
static void manager_run_system(void)
{
    g_system.mode = SYSTEM_RUNNING;

    /* Primero se habilitan los botones de control. */
    vTaskResume(h_btn_dir);
    vTaskResume(h_btn_speed);

    /* Después se reanuda el contador desde el último valor mostrado. */
    vTaskResume(h_counter);

    ESP_LOGW(TAG, "Sistema RUNNING");
}

/* Cambia la dirección del contador entre ascendente y descendente. */
static void manager_toggle_direction(void)
{
    if (g_system.direction == COUNT_UP)
    {
        g_system.direction = COUNT_DOWN;
    }
    else
    {
        g_system.direction = COUNT_UP;
    }

    ESP_LOGI(
        TAG,
        "Nueva direccion: %s",
        g_system.direction == COUNT_UP ? "UP" : "DOWN"
    );
}

/* Cambia la velocidad del contador entre lenta y rápida. */
static void manager_toggle_speed(void)
{
    if (g_system.period_ms == SPEED_SLOW_MS)
    {
        g_system.period_ms = SPEED_FAST_MS;
    }
    else
    {
        g_system.period_ms = SPEED_SLOW_MS;
    }

    ESP_LOGI(
        TAG,
        "Nueva velocidad: %lu ms",
        (unsigned long)g_system.period_ms
    );
}

/* Imprime el estado actual de las tareas y del sistema. */
static void manager_print_states(void)
{
    ESP_LOGI(TAG, "------ ESTADOS ------");

    ESP_LOGI(TAG, "COUNTER: %s", state_to_string(eTaskGetState(h_counter)));

    ESP_LOGI(TAG, "BTN_START: %s", state_to_string(eTaskGetState(h_btn_start)));

    ESP_LOGI(TAG, "BTN_DIR: %s", state_to_string(eTaskGetState(h_btn_dir)));

    ESP_LOGI(TAG, "BTN_SPEED: %s", state_to_string(eTaskGetState(h_btn_speed)));

    /* Recorre las tareas de LEDs para imprimir su estado. */
    for (int i = 0; i < 4; i++)
    {
        ESP_LOGI(
            TAG,
            "%s: %s",
            led_params[i].name,
            state_to_string(eTaskGetState(h_leds[i]))
        );
    }

    /* Imprime el estado general del sistema. */
    ESP_LOGI(
        TAG,
        "Valor=%u | Modo=%s | Direccion=%s | Periodo=%lu ms",
        g_system.value,
        g_system.mode == SYSTEM_RUNNING ? "RUNNING" : "PAUSED",
        g_system.direction == COUNT_UP ? "UP" : "DOWN",
        (unsigned long)g_system.period_ms
    );
}

/* Tarea principal de administración.
   Procesa eventos generados por los botones y controla el sistema. */
static void task_manager(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_print;
    last_print = xTaskGetTickCount();

    while (1)
    {
        ManagerEvent_t event;

        /* Lee el evento pendiente generado por alguna tarea de botón. */
        event = g_system.pending_event;

        if (event != MANAGER_EVENT_NONE)
        {
            /* Se consume el evento para evitar procesarlo más de una vez. */
            g_system.pending_event = MANAGER_EVENT_NONE;

            switch (event)
            {
                case MANAGER_EVENT_SPEED:
                    /* Solo permite cambiar velocidad si el sistema está corriendo. */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_speed();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Velocidad ignorada: sistema pausado");
                    }
                    break;

                case MANAGER_EVENT_DIRECTION:
                    /* Solo permite cambiar dirección si el sistema está corriendo. */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_direction();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Direccion ignorada: sistema pausado");
                    }
                    break;

                case MANAGER_EVENT_START_PAUSE:
                    /* Alterna entre pausa y ejecución. */
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_pause_system();
                    }
                    else
                    {
                        manager_run_system();
                    }
                    break;

                default:
                    break;
            }
        }

        /* Cada 2 segundos imprime los estados de las tareas. */
        if ((xTaskGetTickCount() - last_print) >= pdMS_TO_TICKS(2000))
        {
            last_print = xTaskGetTickCount();
            manager_print_states();
        }

        /* Retardo pequeño para evitar que el manager consuma CPU continuamente. */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* Función encargada de crear todas las tareas de la aplicación. */
void app_tasks_create(void)
{
    /* Crea una tarea por cada LED.
       Todas usan la misma función led_task, pero con parámetros diferentes. */
    for (int i = 0; i < 4; i++)
    {
        xTaskCreate(
            led_task,
            led_params[i].name,
            2048,
            &led_params[i],
            1,
            &h_leds[i]
        );
    }

    /* Crea la tarea del botón Start/Pause. */
    xTaskCreate(
        button_task,
        "BTN_START",
        2048,
        &btn_start,
        2,
        &h_btn_start
    );

    /* Crea la tarea del botón de dirección. */
    xTaskCreate(
        button_task,
        "BTN_DIR",
        2048,
        &btn_dir,
        2,
        &h_btn_dir
    );

    /* Crea la tarea del botón de velocidad. */
    xTaskCreate(
        button_task,
        "BTN_SPEED",
        2048,
        &btn_speed,
        2,
        &h_btn_speed
    );

    /* Crea la tarea del contador. */
    xTaskCreate(
        counter_task,
        "COUNTER",
        2048,
        NULL,
        3,
        &h_counter
    );

    /* Crea la tarea del Task Manager, con mayor prioridad
       porque administra los eventos del sistema. */
    xTaskCreate(
        task_manager,
        "MANAGER",
        4096,
        NULL,
        4,
        &h_manager
    );

    /* Estado inicial:
       - El contador queda pausado.
       - Dirección y velocidad quedan deshabilitadas.
       - El botón Start/Pause permanece activo.
       - Los LEDs siguen activos mostrando el valor inicial. */
    manager_pause_system();
}

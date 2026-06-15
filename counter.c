#include "counter.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

/* Etiqueta utilizada para mostrar mensajes del contador en la terminal. */
static const char *TAG = "COUNTER";

/* Función interna que actualiza el valor del contador
   dependiendo de la dirección seleccionada. */
static void counter_step(void)
{
    /* Si la dirección es ascendente, incrementa el contador. */
    if (g_system.direction == COUNT_UP)
    {
        g_system.value++;

        /* Si pasa de 9, regresa a 0 porque es un contador BCD. */
        if (g_system.value > 9)
        {
            g_system.value = 0;
        }
    }
    else
    {
        /* Si la dirección es descendente y el valor es 0,
           el contador vuelve a 9. */
        if (g_system.value == 0)
        {
            g_system.value = 9;
        }
        else
        {
            /* Decrementa el valor del contador. */
            g_system.value--;
        }
    }
}

/* Tarea encargada de ejecutar el conteo. */
void counter_task(void *pvParameters)
{
    /* No se utilizan parámetros en esta tarea. */
    (void)pvParameters;

    while (1)
    {
        /* Muestra por UART el valor actual, dirección y periodo. */
        ESP_LOGI(
            TAG,
            "Valor=%u | Direccion=%s | Periodo=%lu ms",
            g_system.value,
            g_system.direction == COUNT_UP ? "UP" : "DOWN",
            (unsigned long)g_system.period_ms
        );

        /* Espera el tiempo configurado antes de cambiar el contador. */
        vTaskDelay(
            pdMS_TO_TICKS(g_system.period_ms)
        );

        /* Actualiza el contador después del retardo. */
        counter_step();
    }
}

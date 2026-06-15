/*
 *
 * Este proyecto implementa un contador BCD ascendente y descendente en un ESP32
 * utilizando FreeRTOS. El sistema permite iniciar o pausar el conteo, cambiar
 * la dirección y modificar la velocidad mediante tres botones físicos. La
 * aplicación emplea tareas parametrizadas con pvParameters y un Task Manager
 * que controla el comportamiento del sistema mediante TaskHandle_t y los
 * diferentes estados de ejecución de FreeRTOS.
 */

#include "system_state.h"
#include "app_task.h"

void app_main(void)
{
    /* Inicializa el estado global del sistema:
       valor inicial, modo, dirección, velocidad y eventos pendientes. */
    system_state_init();

    /* Crea todas las tareas de la aplicación:
       LEDs, botones, contador y Task Manager. */
    app_tasks_create();
}

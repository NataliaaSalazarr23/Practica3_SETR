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

#include "system_state.h"
#include "app_task.h"

void app_main(void)
{
	
	system_state_init();

    app_tasks_create();

} 

# Practica 3 

Emilio Hernández Santana 10095
Natalia Marian Salazar Dompinguez 10073

## Preguntas  

1. ¿Qué diferencia existe entre BLOCKED y SUSPENDED?

Una tarea en BLOCKED está esperando que ocurra algún evento o que termine un tiempo de espera (vTaskDelay). Una tarea en SUSPENDED está detenida explícitamente mediante vTaskSuspend() y no volverá a ejecutarse hasta que se llame a vTaskResume().

2. ¿Por qué vTaskDelay() coloca una tarea en estado BLOCKED?

Porque la tarea debe esperar un tiempo determinado antes de continuar ejecutándose. Mientras espera, FreeRTOS la coloca en estado BLOCKED para liberar el procesador y permitir que otras tareas se ejecuten.

3. ¿Qué diferencia existe entre vTaskDelay() y un Software Timer?

vTaskDelay() detiene una tarea específica durante un tiempo. Un Software Timer ejecuta una función de callback cuando vence el tiempo sin necesidad de bloquear una tarea completa.

4. ¿Qué función cumple el Idle Task?

El Idle Task es una tarea especial de FreeRTOS que se ejecuta cuando ninguna otra tarea está lista para correr. También ayuda a liberar recursos de tareas eliminadas.

5. ¿Cómo decide FreeRTOS cuál tarea ejecutar cuando varias tienen la misma prioridad?

Si varias tareas tienen la misma prioridad y están listas para ejecutarse, FreeRTOS utiliza Round Robin, repartiendo el tiempo de CPU entre ellas.

6. ¿Qué ventajas aporta pvParameters?

Permite reutilizar una misma función de tarea para diferentes dispositivos o configuraciones, enviando parámetros personalizados al momento de crear la tarea.

7. ¿Qué ventajas aporta TaskHandle_t?

Permite identificar y controlar tareas específicas, por ejemplo para suspenderlas, reanudarlas, eliminarlas o consultar su estado.

8. ¿Qué ocurriría si el contador se implementara con variables globales únicamente?

El código sería menos modular y más difícil de mantener. Además, varias tareas podrían modificar las mismas variables al mismo tiempo, provocando comportamientos inesperados o errores de sincronización.

## Conclusión

En esta práctica se implementó un contador BCD ascendente y descendente utilizando FreeRTOS en el ESP32. Se aplicaron conceptos importantes como la creación de tareas, el uso de pvParameters para reutilizar código, y TaskHandle_t para controlar el estado de las tareas mediante un Task Manager. Además, se utilizó la suspensión y reanudación de tareas para implementar las funciones de Start/Pause, permitiendo comprender el funcionamiento de los estados de ejecución de FreeRTOS. Con esta práctica se reforzó el diseño modular de aplicaciones embebidas y la administración de múltiples tareas en tiempo real.

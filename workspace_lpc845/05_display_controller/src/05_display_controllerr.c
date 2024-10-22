#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"


/**
 * @brief Programa principal
 */
int main(void) {
	// Clock del sistema a 30 MHz
	BOARD_BootClockFRO30M();

	// Creacion de tareas

	xTaskCreate(
		task_init,
		"Init",
		tskINIT_STACK,
		NULL,
		tskINIT_PRIORITY,
		NULL
	);



	xTaskCreate(
		task_display_write,
		"Write",
		tskDISPLAY_WRITE_STACK,
		NULL,
		tskDISPLAY_WRITE_PRIORITY,
		&handle_display
	);

	xTaskCreate(
		task_btn,
		"Button",
		tskBTN_STACK,
		NULL,
		tskBTN_PRIORITY,
		NULL
	);

	vTaskStartScheduler();

    while(1);
    return 0;
}

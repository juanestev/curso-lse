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
		task_adc_read_1,
		"ADC",
		tskADC_READ_STACK,
		NULL,
		tskADC_READ_PRIORITY,
		NULL
	);

	xTaskCreate(
			task_adc_read_2,
			"ADC2",
			tskADC_READ_STACK,
			NULL,
			tskADC_READ_PRIORITY,
			NULL
	);

	xTaskCreate(
			task_pwm,
			"PWM",
			tskPWM_STACK,
			NULL,
			tskPWM_PRIORITY,
			NULL
	);

	vTaskStartScheduler();

    while(1);
    return 0;
}

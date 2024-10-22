#include "app_tasks.h"

// Cola para selecion de valor para el display
xQueueHandle queue_display_variable;
// Cola para datos de temperatura
xQueueHandle queue_temp;

// Handler para la tarea de display write
TaskHandle_t handle_display;


/**
 * @brief Inicializa todos los perifericos y colas
 */
void task_init(void *params) {
	// Inicializacion de GPIO
	wrapper_gpio_init(0);
	// Configuro el ADC
	wrapper_adc_init();
	// Configuro el display
	wrapper_display_init();
	// Configuro botones
	wrapper_btn_init();
	// Inicializo el PWM
	wrapper_pwm_init();
	// Inicializo I2C y Bh1750

	// Inicializo colas
	queue_display_variable = xQueueCreate(1, sizeof(display_variable_t));
	// Elimino tarea para liberar recursos
	vTaskDelete(NULL);
}


/**
 * @brief Lee los valores de los botones para definir que valor mostrar
 */
void task_btn(void *params) {

	display_variable_t val = 0;

	while(1) {
		// Veo que boton se presiono
		if(!wrapper_btn_get(USR_BTN)) {
			// Antirebote
			vTaskDelay(20);
			if(!wrapper_btn_get(USR_BTN)) {
				// USR Button para temperatura
				val = 0;
			}
		}
		else if(!wrapper_btn_get(S1_BTN)) {
			// Antirebote
			vTaskDelay(20);
			if(!wrapper_btn_get(S1_BTN) && val < 99) {
				// ISP Button para la referencia
				val++;
			}
		}
		else if(!wrapper_btn_get(S2_BTN)) {
			// Antirebote
			vTaskDelay(20);
			if(!wrapper_btn_get(S2_BTN) && val > 0) {
				// ISP Button para la referencia
				val--;
			}
		}
		// Mando datos en la cola
		xQueueOverwrite(queue_display_variable, &val);
		vTaskDelay(200);
	}
}

/**
 * @brief Escribe valores en el display
 */
void task_display_write(void *params) {
	// Variable a mostrar
	display_variable_t val = 0;

	uint8_t num = 0;

	while(1) {
		// Veo que variable hay que mostrar
		xQueuePeek(queue_display_variable, &val, portMAX_DELAY);
		num = val;
		// Muestro el numero
		wrapper_display_off();
		wrapper_display_write((uint8_t)(num / 10));
		wrapper_display_on(COM_1);
		vTaskDelay(10);
		wrapper_display_off();
		wrapper_display_write((uint8_t)(num % 10));
		wrapper_display_on(COM_2);
		vTaskDelay(10);
	}
}




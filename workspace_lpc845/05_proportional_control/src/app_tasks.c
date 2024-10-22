#include "app_tasks.h"

// Cola para datos del ADC
xQueueHandle queue_adc_1;
xQueueHandle queue_adc_2;
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

	queue_adc_1 = xQueueCreate(1, sizeof(adc_data_t));
	queue_adc_2 = xQueueCreate(1, sizeof(adc_data_t));
	// Inicializo colas
	queue_display_variable = xQueueCreate(1, sizeof(display_variable_t));
	// Elimino tarea para liberar recursos
	vTaskDelete(NULL);
}


void task_adc_read_1(void *params) {

	float adc_1 = 0.0;

	while(1) {
		adc_result_info_t adc_info;
		// Inicio una conversion
		ADC_DoSoftwareTriggerConvSeqA(ADC0);
		// Espero a terminar la conversion
		while(!ADC_GetChannelConversionResult(ADC0, REF_POT_CH_1, &adc_info));

		adc_1 = adc_info.result;

		xQueueOverwrite(queue_adc_1, &adc_1);
		// Bloqueo la tarea por 250 ms
		vTaskDelay(250);
	}
}

void task_adc_read_2(void *params) {

	float adc_2 = 0.0;

	while(1) {
		adc_result_info_t adc_info;
		// Inicio una conversion
		ADC_DoSoftwareTriggerConvSeqB(ADC0);
		// Espero a terminar la conversion
		while(!ADC_GetChannelConversionResult(ADC0, REF_POT_CH_2, &adc_info));

		adc_2 = adc_info.result;


		xQueueOverwrite(queue_adc_2, &adc_2);
		// Bloqueo la tarea por 250 ms
		vTaskDelay(250);
	}
}

void task_pwm(void *params) {
	// Variable para guardar los datos del ADC
	float adc_1 = 0.0;
	float adc_2 = 0.0;

	while(1) {

		xQueueReceive(queue_adc_1, &adc_1, portMAX_DELAY);
		xQueueReceive(queue_adc_2, &adc_2, portMAX_DELAY);

		float diferencia = (adc_1 - adc_2);
		// Actualizo el duty
		wrapper_pwm_update((int16_t)diferencia);
	}
}





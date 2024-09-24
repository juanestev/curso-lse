#include "board.h"
#include "fsl_swm.h"
#include "fsl_i2c.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "fsl_sctimer.h"

// Direccion del BH1750
#define BH1750_ADDR	0x5c

// Frecuencia de PWM
#define PWM_FREQ	1000

/**
 * @brief Programa principal
 */
int main(void) {
	// Inicializacion de consola
	BOARD_InitDebugConsole();
	// Arranco de 30 MHz
	BOARD_BootClockFRO30M();

	// Conecto la salida 4 del SCT al LED azul
    CLOCK_EnableClock(kCLOCK_Swm);
    SWM_SetMovablePinSelect(SWM0, kSWM_SCT_OUT4, kSWM_PortPin_P1_1);
    CLOCK_DisableClock(kCLOCK_Swm);

    // Eligo el clock para el Timer
    uint32_t sctimer_clock = CLOCK_GetFreq(kCLOCK_Fro);
    // Configuracion del SCT Timer
    sctimer_config_t sctimer_config;
    SCTIMER_GetDefaultConfig(&sctimer_config);
    SCTIMER_Init(SCT0, &sctimer_config);

    // Configuro el PWM
    sctimer_pwm_signal_param_t pwm_config = {
		.output = kSCTIMER_Out_4,		// Salida del Timer
		.level = kSCTIMER_LowTrue,		// Logica negativa
		.dutyCyclePercent = 50			// 50% de ancho de pulso
    };

    // Variable para guardar el evento al quese asigna el PWM
    uint32_t event;
    // Inicializo el PWM
    SCTIMER_SetupPwm(
		SCT0,
		&pwm_config,
		kSCTIMER_CenterAlignedPwm,
		PWM_FREQ,
		sctimer_clock,
		&event
	);
    // Inicializo el Timer
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);
	// Inicializo el clock del I2C1
	CLOCK_Select(kI2C1_Clk_From_MainClk);
    // Asigno las funciones de I2C1 a los pines 26 y 27
	CLOCK_EnableClock(kCLOCK_Swm);
    SWM_SetMovablePinSelect(SWM0, kSWM_I2C1_SDA, kSWM_PortPin_P0_27);
    SWM_SetMovablePinSelect(SWM0, kSWM_I2C1_SCL, kSWM_PortPin_P0_26);
    CLOCK_DisableClock(kCLOCK_Swm);

    // Configuracion de master para el I2C con 400 KHz de clock
    i2c_master_config_t config;
    I2C_MasterGetDefaultConfig(&config);
    config.baudRate_Bps = 400000;
    // Usa el clock del sistema de base para generar el de la comunicacion
    I2C_MasterInit(I2C1, &config, SystemCoreClock);

	if(I2C_MasterStart(I2C1, BH1750_ADDR, kI2C_Write) == kStatus_Success) {
		// Comando de power on
		uint8_t cmd = 0x01;
		I2C_MasterWriteBlocking(I2C1, &cmd, 1, kI2C_TransferDefaultFlag);
		I2C_MasterStop(I2C1);
	}
	if(I2C_MasterStart(I2C1, BH1750_ADDR, kI2C_Write) == kStatus_Success) {
		// Comando de medicion continua a 1 lux de resolucion
		uint8_t cmd = 0x10;
		I2C_MasterWriteBlocking(I2C1, &cmd, 1, kI2C_TransferDefaultFlag);
		I2C_MasterStop(I2C1);
	}

	while(1) {
		// Lectura del sensor
		if(I2C_MasterStart(I2C1, BH1750_ADDR, kI2C_Read) == kStatus_Success) {
			// Resultado
			uint8_t res[2] = {0};
			I2C_MasterReadBlocking(I2C1, res, 2, kI2C_TransferDefaultFlag);
			I2C_MasterStop(I2C1);
			// Devuelvo el resultado
			float lux = ((res[0] << 8) + res[1]) / 1.2;
			PRINTF("LUX : %d \r\n",(uint16_t) lux);
			// Variable para guardar el duty

			lux = (lux * 100) / 30000;
			uint32_t duty = 100 - lux;

			if(duty < 100 && duty > 0){
			    // Actualizo el ancho de pulso
			    SCTIMER_UpdatePwmDutycycle(SCT0, kSCTIMER_Out_4, duty, event);
			}
		}
    }
    return 0;
}

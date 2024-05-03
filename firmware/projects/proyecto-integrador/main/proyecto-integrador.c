/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * Este proyeto es un simulador de un osciloscopio. 
 * Tiene la funcionalidad de converción A/D y D/A
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/04/2024 | Document creation		                         |
 *
 * @author Matias Madrid (pablo.madrid@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"


/*==================[macros and definitions]=================================*/

/** @def TIME_PERIOD2
 *  @brief 
*/
#define TIME_PERIOD2 4000

/*==================[internal data definition]===============================*/

TaskHandle_t task_handle = NULL;
TaskHandle_t task_handle1 = NULL;

static void Tarea_leer_enviar(void *pvParameter){
	uint16_t A_valor;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */

		AnalogInputReadSingle(CH1, &A_valor);
		UartSendString(UART_PC, (char*) UartItoa(A_valor,10));
		UartSendString(UART_PC, " \n\r");
		
	}
	
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}


/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){

	timer_config_t timer_2 = {
        .timer = TIMER_B,
        .period = TIME_PERIOD2,
        .func_p = funcTimer,
        .param_p = NULL
    };
	
	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL
	};

	serial_config_t serial_global = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };

	UartInit(&serial_global);
	
	TimerInit(&timer_2);
	AnalogInputInit(&analog_input);
	AnalogOutputInit();

	xTaskCreate(&Tarea_leer_enviar, "leer y enviar datos", 4096, NULL, 5, &task_handle);

	TimerStart(timer_2.timer);
}
/*==================[end of file]============================================*/
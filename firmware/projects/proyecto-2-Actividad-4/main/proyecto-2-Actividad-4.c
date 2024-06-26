/*! @mainpage Proyecto 2 / Actividad 4
 *
 * @section genDesc General Description
 *
 * Este proyeto es un simulador de un osciloscopio.
 * Tiene la funcionalidad de converción A/D y D/A
 *
 *
 * @section hardConn Hardware Connection
 *
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
/** @def BUFFER_SIZE
 *  @brief
 */
#define BUFFER_SIZE 231

/** @def TIME_PERIOD1
 *  @brief
 */
#define TIME_PERIOD1 2000

/** @def TIME_PERIOD2
 *  @brief
 */
#define TIME_PERIOD2 4000

/** @brief Indice para recorrer la señal digital del ECG*/
uint8_t indice = 0;

/*==================[internal data definition]===============================*/

/** @brief Señal digital de un ECG*/
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de leer y enviar datos*/
TaskHandle_t task_handle = NULL;

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de levantar datos transformar con pwm*/
TaskHandle_t task_handle1 = NULL;

/** @fn Tarea_leer_enviar(void *pvParameter)
 * @brief Tarea transformar los datos analogicos a digital, recibidos por CH1  y enviarlos por el puerto serie.
 */
static void Tarea_leer_enviar(void *pvParameter)
{
	uint16_t A_valor;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */

		AnalogInputReadSingle(CH1, &A_valor);
		UartSendString(UART_PC, (char *)UartItoa(A_valor, 10));
		UartSendString(UART_PC, "\r");
	}
}

/** @fn Tarea_pwm(void *pvParameter){
 * @brief Tarea transformar los datos digitales (del ECG) a analogicos.
 */
static void Tarea_pwm(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		AnalogOutputWrite(ecg[indice]);
		indice++;
		if (indice >= sizeof(ecg))
		{
			indice = 0;
		}
	}
}

/**
 * @brief Función invocada en la interrupción del timer A y timer B
 */
void funcTimerMuestreo(void *param)
{
	vTaskNotifyGiveFromISR(task_handle, pdFALSE);
}

void funcTimer2(void *param)
{
	vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD1,
		.func_p = funcTimerMuestreo,
		.param_p = NULL};

	timer_config_t timer_2 = {
		.timer = TIMER_B,
		.period = TIME_PERIOD2,
		.func_p = funcTimer2,
		.param_p = NULL};

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};

	serial_config_t serial_global = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};
		
	UartInit(&serial_global);

	TimerInit(&timer_1);
	TimerInit(&timer_2);
	AnalogInputInit(&analog_input);
	AnalogOutputInit();

	xTaskCreate(&Tarea_leer_enviar, "leer y enviar datos", 4096, NULL, 5, &task_handle);
	xTaskCreate(&Tarea_pwm, "levantar datos transformar con pwm", 4096, NULL, 5, &task_handle1);

	TimerStart(timer_1.timer);
	TimerStart(timer_2.timer);
}
/*==================[end of file]============================================*/
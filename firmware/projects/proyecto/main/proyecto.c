/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * Proyecto fachero
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Placa de    |     	 		|
 * | biopotenciales |	ESP32		|
 * |:--------------:|:--------------|
 * | 	ECG	 		| 	  CH1		|
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
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "buzzer.h"
#include "ble_mcu.h"
#include "iir_filter.h"

/*==================[macros and definitions]=================================*/

#define BUFFER_SIZE 231

/** @def TIME_PERIOD1
 *  @brief
 */
#define TIME_PERIOD1 10000 // Tiempo de muestreo 4[us] milisegundos
// #define TIME_PERIOD2 2000 //2 milisegundo
//  1000000us -> 1s
#define TIME_PERIOD2 20000		   // se usa para el
#define PERIODO_REFRACTARIO 250000 // VALOR FISIOLOGICO DEL PERIODO REFRACTARIO
#define TIME_BUZZER 500000

#define BUZZER GPIO_3

#define SAMPLE_FREQ 100 // 250Hz -> 4ms
#define CHUNK 8			// cant de muestras
uint8_t sonarBuzzer = 0;
uint8_t indice = 0;

// para la FC
uint8_t muestras_en_periodo_refractario = PERIODO_REFRACTARIO / TIME_PERIOD1; //

int16_t UMBRAL = 200; // VER QUE VALOR CONVIENE (FALTA DEFINIR)
uint8_t FC = 0;
uint16_t periodo_RR = 0;
uint16_t valor_actual = 0;

/*==================[internal data definition]===============================*/
const char ecg[BUFFER_SIZE] = {
	76,
	77,
	78,
	77,
	79,
	86,
	81,
	76,
	84,
	93,
	85,
	80,
	89,
	95,
	89,
	85,
	93,
	98,
	94,
	88,
	98,
	105,
	96,
	91,
	99,
	105,
	101,
	96,
	102,
	106,
	101,
	96,
	100,
	107,
	101,
	94,
	100,
	104,
	100,
	91,
	99,
	103,
	98,
	91,
	96,
	105,
	95,
	88,
	95,
	100,
	94,
	85,
	93,
	99,
	92,
	84,
	91,
	96,
	87,
	80,
	83,
	92,
	86,
	78,
	84,
	89,
	79,
	73,
	81,
	83,
	78,
	70,
	80,
	82,
	79,
	69,
	80,
	82,
	81,
	70,
	75,
	81,
	77,
	74,
	79,
	83,
	82,
	72,
	80,
	87,
	79,
	76,
	85,
	95,
	87,
	81,
	88,
	93,
	88,
	84,
	87,
	94,
	86,
	82,
	85,
	94,
	85,
	82,
	85,
	95,
	86,
	83,
	92,
	99,
	91,
	88,
	94,
	98,
	95,
	90,
	97,
	105,
	104,
	94,
	98,
	114,
	117,
	124,
	144,
	180,
	210,
	236,
	253,
	227,
	171,
	99,
	49,
	34,
	29,
	43,
	69,
	89,
	89,
	90,
	98,
	107,
	104,
	98,
	104,
	110,
	102,
	98,
	103,
	111,
	101,
	94,
	103,
	108,
	102,
	95,
	97,
	106,
	100,
	92,
	101,
	103,
	100,
	94,
	98,
	103,
	96,
	90,
	98,
	103,
	97,
	90,
	99,
	104,
	95,
	90,
	99,
	104,
	100,
	93,
	100,
	106,
	101,
	93,
	101,
	105,
	103,
	96,
	105,
	112,
	105,
	99,
	103,
	108,
	99,
	96,
	102,
	106,
	99,
	90,
	92,
	100,
	87,
	80,
	82,
	88,
	77,
	69,
	75,
	79,
	74,
	67,
	71,
	78,
	72,
	67,
	73,
	81,
	77,
	71,
	75,
	84,
	79,
	77,
	77,
	76,
	76,
};

TaskHandle_t task_handle = NULL;
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle3 = NULL;

static float ecg_filt[CHUNK] = {0}; // arregla para aplicar el filtro
static float ecg_muestra[CHUNK] = {0};
float last_valor = 0;

static void Sumilador_ECG(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		AnalogOutputWrite(ecg[indice]);
		indice++;
		if (indice == BUFFER_SIZE)
		{
			indice = 0;
		}
	}
}

void detectar_ondaR(float p_valor)
{
	if ((p_valor > UMBRAL) && (periodo_RR > muestras_en_periodo_refractario))
	{
		periodo_RR = 0;
		sonarBuzzer = 1;
	}
}

static void sonar(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// if (sonarBuzzer == 1){
		BuzzerPlayTone(600, 200);
		// sonarBuzzer = 0;
		//}
	}
}

static void Tarea_leer_enviar(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		AnalogInputReadSingle(CH1, &valor);
		// UartSendString(UART_PC, (char *)UartItoa(ecg_muestra[j], 10));
		// UartSendString(UART_PC, "\r");
		ecg_muestra[i] = valor;
		valor_actual = valor;
		periodo_RR++;
		i++;

		if (i == CHUNK)
		{
			HiPassFilter(&ecg_muestra[0], ecg_filt, CHUNK);
			LowPassFilter(ecg_filt, ecg_filt, CHUNK);
			strcpy(msg, ""); // msg[0] = 0;
			for (uint8_t j = 0; j < CHUNK; j++)
			{
				detectar_ondaR(ecg_filt[j]);
				sprintf(msg_chunk, "*G%.2f*", ecg_filt[j]);
				strcat(msg, msg_chunk);
				// UartSendString(UART_PC, (char *)UartItoa(ecg_filt[j], 10));
				// UartSendString(UART_PC, "\r");
			}

			BleSendString(msg);
			i = 0;
		}
	}
}

/*
void calcularFC(float *signal, uint8_t signal_length){ //hacer de otra forma, sin diff, solo con valor y umbral
	if (periodo_refractario == 0){
		diff = signal[0] - last_valor;
		//diff = diff*diff;
		if (diff < UMBRAL){
			FC = 60000/(periodo_RR*TIME_PERIOD1);
			periodo_RR = 0;
			periodo_refractario = 250/TIME_PERIOD1;
		}

		for(uint8_t i=1; i<signal_length; i++){
			diff = signal[i] - signal[i-1];
			//diff = diff*diff;
			if (diff < UMBRAL && periodo_refractario == 0){
				FC = 60000/(periodo_RR*TIME_PERIOD1);
				periodo_RR = 0;
				periodo_refractario = 250/TIME_PERIOD1;
			}
		}

	} else {
		periodo_refractario--;
	}
}*/

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer1(void *param)
{
	vTaskNotifyGiveFromISR(task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
												  // vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
}

void funcTimer2(void *param)
{
	vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
}

void funcTimerBUZZER(void *param)
{
	vTaskNotifyGiveFromISR(task_handle3, pdFALSE);
}

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void)
{

	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD1, // sacar *chunk
		.func_p = funcTimer1,
		.param_p = NULL};

	timer_config_t timer_2 = {
		.timer = TIMER_B,
		.period = TIME_PERIOD2, // poner TIME_PERIOD2
		.func_p = funcTimer2,
		.param_p = NULL};

	timer_config_t timer_3 = {
		.timer = TIMER_C,
		.period = TIME_BUZZER,
		.func_p = funcTimerBUZZER,
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

	ble_config_t ble_configuration = {
		"ESP_ECG",
		NULL};

	UartInit(&serial_global);
	TimerInit(&timer_1);
	TimerInit(&timer_2);
	TimerInit(&timer_3);
	AnalogInputInit(&analog_input);
	AnalogOutputInit();
	BleInit(&ble_configuration);

	LowPassInit(SAMPLE_FREQ, 45, ORDER_4);
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);

	BuzzerInit(BUZZER);

	xTaskCreate(&Tarea_leer_enviar, "leer y enviar datos", 4096, NULL, 5, &task_handle);
	// xTaskCreate(&Tarea_sonido, "hacer sonar el buzzer", 1024, NULL, 5, &task_handle1); //VER SI BORRAR O DEJAR
	xTaskCreate(&Sumilador_ECG, "Envía la señal ECG analógica por CH0", 4096, NULL, 5, &task_handle1);
	xTaskCreate(&sonar, "sonar Buzzer", 2048, NULL, 5, &task_handle3);
	TimerStart(timer_1.timer);
	TimerStart(timer_2.timer); // sumilador
	TimerStart(timer_3.timer);
}
/*==================[end of file]============================================*/
/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * Este proyecto monitoriza un ECG de un paciente, sensa los datos con una placa de biopotencial, los datos
 * son procesados por la placa, se le aplica un filtro pasabanda, se detenctan los picos R, se calcula la frecuancia cardiaca.
 * Después los datos procesados son enviados por Bluetooth para ser visualizados para el usuario.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |   Perifericos  |	ESP32		|
 * |:--------------:|:--------------|
 * | 	ECG	 		| 	  CH1		|
 * |	Buzzer		|  	  CH3		|
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

/** @def PERIODO_MUESTREO
 *  @brief Tiempo (us) que establece el periodo de muestreo de la señal ECG
 */
#define PERIODO_MUESTREO 10000

/** @def PERIODO_REFRACTARIO
 *  @brief Tiempo (us) de la duración del periodo refractario fisiológico
 */
#define PERIODO_REFRACTARIO 250000

/** @def TIME_BUZZER
 *  @brief Tiempo (us) que establece el tiempo
 */
#define TIME_BUZZER 20000

/** @def BUZZER
 *  @brief Se define la variable BUZZER al GPIO3 donde estará conectado el buzzer
 */
#define BUZZER GPIO_3

/** @def SAMPLE_FREQ
 *  @brief Se define la variable SAMPLE_FREQ la frecuencia del periodo de muestreo para aplicar el filtro
 */
#define SAMPLE_FREQ 100

/** @def CHUNK
 *  @brief Cantidad de muestras que se toman para procesar la señal
 */
#define CHUNK 8

/*==================[internal data definition]===============================*/

/** @brief sonarBuzzer Badera que se utiliza para verificar si se debe activar el buzzer */
uint8_t sonarBuzzer = 0;

/** @brief muestras_en_periodo_refractario Cantiad de muestras que abarca la duración del perido refractario */
uint8_t muestras_en_periodo_refractario = PERIODO_REFRACTARIO / PERIODO_MUESTREO;

/** @brief UMRAL Cantiad de muestras que abarca la duración del perido refractario */
int16_t UMBRAL = 400; // VER QUE VALOR CONVIENE (FALTA DEFINIR) | RECIBIR POR BLUETOOTH

/** @brief FC Variable que guarda la frecuancia cardiaca momentanea */
uint16_t FC = 0;

/** @brief periodo_RR Variable que guarda cantidad de muestras entre dos picos R*/
uint16_t periodo_RR = 0;

/** @brief valor_actual Variable que guarda el valor actual muestreado */
uint16_t valor_actual = 0;

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de procesar y enviar datos*/
TaskHandle_t task_procesar_enviar = NULL;

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de sonar el buzzer*/
TaskHandle_t task_sonar = NULL;

/** @brief ecg_filt[CHUNK] Vector que guarda los datos filtrados del ECG */
static float ecg_filt[CHUNK] = {0};

/** @brief ecg_filt[CHUNK] Vector que guarda los datos sin filtrar del ECG */
static float ecg_muestra[CHUNK] = {0};

/*==================[internal functions declaration]=========================*/

/** @fn detectar_ondaR(float p_valor)
 * @brief Función para detectar los picos R y calcular la frecuencia cardiaca
 * @param p_valor valor flotante para verificar si es un pico R
 */
void detectar_ondaR(float p_valor)
{
	if ((p_valor > UMBRAL) && (periodo_RR > muestras_en_periodo_refractario))
	{
		FC = 60000000 / (periodo_RR * PERIODO_MUESTREO);
		periodo_RR = 0;
		sonarBuzzer = 1;
	}
}

/** @fn sonar(void *pvParameter)
 * @brief Tarea para encender el buzzer si se detectó un pico R
 */
static void sonar(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // VER SI LO PUEDO SOLUCIONAR USANDO EL MISMO TIEMPO QUE EL PERIODO_MUESTREO
		if (sonarBuzzer == 1)
		{
			BuzzerPlayTone(200, 150);
			sonarBuzzer = 0;
		}
	}
}

/** @fn procesar_y_enviar_ECG(void *pvParameter)
 * @brief Tarea diseñada para leer los datos analógicos de la placa de biopotencial, procesa los datos y los envía vía Bluetooth
 */
static void procesar_y_enviar_ECG(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valor);
		ecg_muestra[i] = valor;
		valor_actual = valor;
		periodo_RR++;
		i++;

		if (i == CHUNK)
		{
			HiPassFilter(&ecg_muestra[0], ecg_filt, CHUNK);
			LowPassFilter(ecg_filt, ecg_filt, CHUNK);
			strcpy(msg, "");

			for (uint8_t j = 0; j < CHUNK; j++)
			{
				detectar_ondaR(ecg_filt[j]);
				sprintf(msg_chunk, "*G%.2f*", ecg_filt[j]);
				strcat(msg, msg_chunk);
			}

			BleSendString(msg);
			i = 0;
		}
	}
}

/** @fn informar(void *pvParameter)
 * @brief Tarea creada para enviar la frecuencia cardiaca vía Bluetooth
 */
static void informar(void *pvParameter)
{
	char msg_fc[30];
	while (1)
	{
		strcpy(msg_fc, "");
		sprintf(msg_fc, "F%u\n", FC);
		FC = 0;
		BleSendString(msg_fc);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Función invocada en la interrupción del timer de la tarea SONAR
 */
void funcTimerBUZZER(void *param)
{
	vTaskNotifyGiveFromISR(task_sonar, pdFALSE);
}

/**
 * @brief Función invocada en la interrupción del la tarea procesar y enviar
 */
void funcTimer1(void *param)
{
	vTaskNotifyGiveFromISR(task_procesar_enviar, pdFALSE);
}

/**
 * @brief Función que convierte el dato recibido por Bluetooth, y se lo asigna al umbral para la deteccion de las ondas R
 * @param data dato recibido por Bluetooth
 */
void read_data(uint8_t *data, uint8_t length)
{
	uint8_t i = 1;
	static uint16_t umbral_aux = 0;
	char msg[60];
	if (data[0] == 'A')
	{
		strcpy(msg, "");
		umbral_aux = 0;
		while (data[i] != 'B')
		{
			umbral_aux = umbral_aux * 10;
			umbral_aux = umbral_aux + (data[i] - '0');
			i++;
		}
	}
	UMBRAL = umbral_aux;
	sprintf(msg, "U%u\n", umbral_aux);
	printf("%u\n", umbral_aux);
	BleSendString(msg);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO,
		.func_p = funcTimer1,
		.param_p = NULL};

	timer_config_t timer_2 = {
		.timer = TIMER_B,
		.period = TIME_BUZZER,
		.func_p = funcTimerBUZZER,
		.param_p = NULL};

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};

	ble_config_t ble_configuration = {
		"ESP_ECG",
		read_data};

	TimerInit(&timer_1);
	TimerInit(&timer_2);
	AnalogInputInit(&analog_input);
	AnalogOutputInit();
	BleInit(&ble_configuration);

	LowPassInit(SAMPLE_FREQ, 45, ORDER_4);
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);

	BuzzerInit(BUZZER);

	xTaskCreate(&procesar_y_enviar_ECG, "lee, aplica filtros y envia datos", 4096, NULL, 5, &task_procesar_enviar);
	xTaskCreate(&sonar, "hace sonar el Buzzer en cada onda R", 4096, NULL, 5, &task_sonar); // NO SÉ SI VA POR CUESTION DE TIEMPO
	xTaskCreate(&informar, "informa la frecuancia cardiaca", 4096, NULL, 5, NULL);
	TimerStart(timer_1.timer);
	TimerStart(timer_2.timer);
}
/*==================[end of file]============================================*/
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



/** @def TIME_PERIOD1
 *  @brief
 */
#define PERIODO_MUESTREO 10000 // Tiempo de muestreo 10[ms] milisegundos -> 100[Hz]
//  1000000us -> 1s
#define PERIODO_REFRACTARIO 250000 // VALOR FISIOLOGICO DEL PERIODO REFRACTARIO
#define TIME_BUZZER 20000 // (VER DE USAR EL MISMO PERIODO QUE EL MUESTREO)

#define BUZZER GPIO_3 

#define SAMPLE_FREQ 100 // 100Hz -> 10[ms] Para el filtro
#define CHUNK 8			// cant de muestras para procesar



/*==================[internal data definition]===============================*/

uint8_t sonarBuzzer = 0; //bandera
uint8_t indice = 0; //para recorrer el vector de ECG digital

// para la FC
uint8_t muestras_en_periodo_refractario = PERIODO_REFRACTARIO / PERIODO_MUESTREO; //

int16_t UMBRAL = 400; // VER QUE VALOR CONVIENE (FALTA DEFINIR) | RECIBIR POR BLUETOOTH
uint16_t FC = 0;
uint16_t periodo_RR = 0; 
uint16_t valor_actual = 0;

TaskHandle_t task_procesar_enviar = NULL;
TaskHandle_t task_sonar = NULL;

static float ecg_filt[CHUNK] = {0}; // PARA FITLRO
static float ecg_muestra[CHUNK] = {0}; //PARA FILTRO
//static float vector_fc[16] = {0};


/*==================[internal functions declaration]=========================*/

void detectar_ondaR(float p_valor)
{
	if ((p_valor > UMBRAL) && (periodo_RR > muestras_en_periodo_refractario))
	{
		FC = 60000000/(periodo_RR*PERIODO_MUESTREO);
		periodo_RR = 0;
		sonarBuzzer = 1;
		//BuzzerPlayTone(200, 200);
	}
}

static void sonar(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //VER SI LO PUEDO SOLUCIONAR USANDO EL MISMO TIEMPO QUE EL PERIODO_MUESTREO
		if (sonarBuzzer == 1){
			BuzzerPlayTone(200, 200); 
			sonarBuzzer = 0;
		}
	}
}

static void procesar_y_enviar_ECG(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
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

static void informar(void *pvParameter){
	//enviar la FC (con caracter *F)
	char msg_fc[30];
	while (1){
		strcpy(msg_fc, "");
		sprintf(msg_fc, "F%u\n", FC);
		FC=0;
		BleSendString(msg_fc);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
}


void funcTimerBUZZER(void *param)
{
	vTaskNotifyGiveFromISR(task_sonar, pdFALSE);
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer1(void *param)
{
	vTaskNotifyGiveFromISR(task_procesar_enviar, pdFALSE);
	//vTaskNotifyGiveFromISR(task_sonar, pdFALSE);
}

void read_data(uint8_t * data, uint8_t length){
	uint8_t i = 1;
	static uint16_t umbral_aux = 0;
	char msg[60];
	if(data[0] == 'A'){
		strcpy(msg, "");
		umbral_aux = 0;
		while(data[i] != 'B'){
			umbral_aux = umbral_aux * 10;
			umbral_aux = umbral_aux + (data[i] - '0');
			i++;
		}
	}
	UMBRAL = umbral_aux;
	sprintf(msg,"U%u\n", umbral_aux);
	printf("%u\n",umbral_aux);
	BleSendString(msg);
}


/*==================[external functions definition]==========================*/
void app_main(void)
{

	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO,
		.func_p = funcTimer1,
		.param_p = NULL
	};


	timer_config_t timer_2 = {
		.timer = TIMER_B,
		.period = TIME_BUZZER,
		.func_p = funcTimerBUZZER,
		.param_p = NULL
	};

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL
	};

	ble_config_t ble_configuration = {
		"ESP_ECG",
		read_data
	};

	TimerInit(&timer_1);
	TimerInit(&timer_2);
	AnalogInputInit(&analog_input);
	AnalogOutputInit();
	BleInit(&ble_configuration);

	LowPassInit(SAMPLE_FREQ, 45, ORDER_4);
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);

	BuzzerInit(BUZZER);

	xTaskCreate(&procesar_y_enviar_ECG, "lee, aplica filtros y envia datos", 4096, NULL, 5, &task_procesar_enviar);
	xTaskCreate(&sonar, "hace sonar el Buzzer en cada onda R", 4096, NULL, 5, &task_sonar);//NO SÉ SI VA POR CUESTION DE TIEMPO
	xTaskCreate(&informar, "informa la frecuancia cardiaca", 4096, NULL, 5, NULL);
	TimerStart(timer_1.timer);
	TimerStart(timer_2.timer);
}
/*==================[end of file]============================================*/
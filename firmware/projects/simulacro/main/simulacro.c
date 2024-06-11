/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "gpio_mcu.h"
#include "hc_sr04.h"

/*/==================[macros and definitions]=================================/*/
#define PERIODO_MUESTREO 5000000


/*==================[internal data definition]===============================*/
TaskHandle_t task_handle_agua = NULL;
TaskHandle_t task_handle_comida = NULL;
bool medir=true;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(task_handle_agua, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
	vTaskNotifyGiveFromISR(task_handle_comida, pdFALSE);

}

void read_switch1(void){
	medir = !medir;
}
/**
 * @brief Tarea encargada de blinkear el LED_1
 */
static void control_agua(void *pvParameter){
	float distancia;
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		if(medir){
        distancia = HcSr04ReadDistanceInCentimeters();
		if(distancia>28.4){
			GPIOOn(GPIO_20);
		}
		else if(distancia=22.0){
			GPIOOff(GPIO_20);
		}
    	}
	}
}

static void control_comida(void *pvParameter){
	uint16_t distancia;
	float valor;
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		if(medir){
        AnalogInputReadSingle(CH1, &valor);

		if(valor>28.4){
			GPIOOn(GPIO_20);
		}
		else if(distancia=22.0){
			GPIOOff(GPIO_20);
		}
    	}
	}
}

/**
 * @brief Tarea encargada de blinkear el LED_2
 */
static void Led2Task(void *pvParameter){
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        printf("LED_2 Toggle\n");
        LedToggle(LED_2);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
	GPIOInit(GPIO_20, GPIO_OUTPUT);
    /* Inicialización de timers */
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = PERIODO_MUESTREO,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL
	};

	AnalogInputInit(&analog_input);
    TimerInit(&timer_1);
    /* Creación de tareas */
    xTaskCreate(&control_agua, "agua", 512, NULL, 5, &task_handle_agua);
    xTaskCreate(&control_comida, "LED_2", 512, NULL, 5, &task_handle_comida);
    /* Inicialización del conteo de timers */
    TimerStart(timer_1.timer);

}
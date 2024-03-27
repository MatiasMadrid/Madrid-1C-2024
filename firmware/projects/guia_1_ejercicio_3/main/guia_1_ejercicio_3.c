/*! @mainpage guia_1_ejercicio_3
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
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
#include "freertos/FreeRTOS.h" //OS
#include "freertos/task.h"
#include "led.h" 
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 2
#define TOOGLE 3
/*==================[internal data definition]===============================*/

struct leds
{
	uint8_t n_led ;     //  indica el número de led a controlar
	uint8_t n_ciclos;  // indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;   // indica el tiempo de cada ciclo
	uint8_t mode;      // ON, OFF, TOGGLE
} my_leds; 	

/*==================[internal functions declaration]=========================*/

void modificarEstado(struct leds *ptro_led){
	if (ptro_led->mode == ON){
		if (ptro_led->n_led == 1){
			LedOn(LED_1);
		}
		if (ptro_led->n_led == 2){
			LedOn(LED_2);
		}
		if (ptro_led->n_led == 3){
			LedOn(LED_3);
		}
	}
	else if(ptro_led->mode == OFF){
		if (ptro_led->n_led == 1){
			LedOff(LED_1);
		}
		if (ptro_led->n_led == 2){
			LedOff(LED_2);
		}
		if (ptro_led->n_led == 3){
			LedOff(LED_3);
		}
	} 
	else if (ptro_led->mode == TOOGLE){
		uint j = 0;
		uint i = 0;
		while(j < ptro_led->n_ciclos){
			j++;
			uint i = 0;
			printf("j: %d\n", j);
			if (ptro_led->n_led == 1){
				LedToggle(LED_1);
			}
			if (ptro_led->n_led == 2){
				LedToggle(LED_2);
			}
			if (ptro_led->n_led == 3){
				LedToggle(LED_3);
			}
			while (i < ptro_led->periodo/100)
			{
				i++;
				printf("i: %d\n", i);
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); 
			}
			
			
		}
		
	}
	
};

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	struct leds miLed1, miLed2, miLed3;
	miLed1.mode = ON;
	miLed1.n_led = 2;

	miLed2.mode = ON;
	miLed2.n_led = 1;

	miLed3.mode = TOOGLE;
	miLed3.n_led = 3;
	miLed3.n_ciclos = 10;
	miLed3.periodo = 500;


	//modificarEstado(&miLed1);
	//modificarEstado(&miLed2);
	modificarEstado(&miLed3);

	//miLed1.mode = OFF;
	//modificarEstado(&miLed1);
	

	//struct leds *ptro_led;
	//modificarEstado(ptro_led);
}

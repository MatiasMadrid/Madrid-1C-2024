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
/*==================[internal data definition]===============================*/

struct leds
{
	uint8_t n_led ;     //  indica el número de led a controlar
	uint8_t n_ciclos;  // indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;   // indica el tiempo de cada ciclo
	uint8_t mode;      // ON, OFF, TOGGLE
} my_leds; 	

/*==================[internal functions declaration]=========================*/

void mosificarEstado(struct *ptro_led){

};

/*==================[external functions definition]==========================*/
void app_main(void){
	struct leds miLed;
	miLed.mode = ON;
	miLed.n_led = LED_1;
	miLed.n_ciclos = 2;
	miLed.periodo = 1;

	modificarEstado(&miLed);

	//struct leds *ptro_led;
	//modificarEstado(ptro_led);
}

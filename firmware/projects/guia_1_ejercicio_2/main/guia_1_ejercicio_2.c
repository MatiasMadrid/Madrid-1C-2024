/** @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * En este código se modificó el ejemplo 1_blinking_switch otrogado por la cátedra, con la finalidad de hacer titilar 
 * los leds 1 y 2 al mantener presionadas las teclas 1 y 2 correspondientemente, además de hacer titilar el led3 al mantener
 * ambas teclas presionadas. 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/04/2024 | Document creation		                         |
 *
 * @author Madrid Matias
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

/** @def CONFIG_BLINK_PERIOD
 *  @brief Valor del período que tienen los leds para titilar
*/
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/


void app_main(void){
	uint8_t teclas; //entero sin signo de 8 bits
	LedsInit();		//incializa las teclas
	SwitchesInit();	//inicializa los switch
    while(1)    {
    	teclas  = SwitchesRead(); //
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1); //invierte el estado 
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;
			case SWITCH_1 | SWITCH_2:
				LedToggle(LED_3);
			break;
    	}
	    //LedToggle(LED_3);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

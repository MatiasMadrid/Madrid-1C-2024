/*! @mainpage Proyecto 2 / Actividad 1
 *
 * @section genDesc General Description 
 *
 * Código diseñado para medir distancia y mostrarla por un display LCD.
 * Además enciende LEDs dependiendo que distancia esté midiendo.
 * Tiene la funcionalidad (mediente tareas) de encender y detener la medición, y mantener el resultado por pantalla.
 *
 * <a href="https://drive.google.com/file/d/1yIPn12GYl-s8fiDQC3_fr2C4CjTvfixg/view/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * |    display LCD |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1	     	| 	GPIO_20		|
 * | 	D2	     	| 	GPIO_21  	|
 * | 	D3 	     	| 	GPIO_22  	|
 * | 	D4	     	| 	GPIO_23  	|
 * | 	SEL_1	    | 	GPIO_19  	|
 * | 	SEL_2	    | 	GPIO_18  	|
 * | 	SEL_3     	|  	GPIO_9  	|
 * | 	+5V	     	| 	+5V    		|
 * | 	GND	     	| 	GND    		|
 * 
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
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h" 
#include "switch.h"
#include "gpio_mcu.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"


/*==================[macros and definitions]=================================*/
/** @def CONFIG_BLINK_PERIOD_LED_1
 *  @brief Tiempo de espera (ms) para la lectura y encendido/apagado de LEDs
*/
#define CONFIG_BLINK_PERIOD_LED_1 1000

/** @def PERDIOD_SWITCH
 *  @brief Tiempo de espera (ms) para la lectura del estado de botones de la placa
*/
#define PERDIOD_SWITCH 50

/** @def ECHO
 *  @brief Constante inicializar el ECHO con GPIO_3
*/
#define ECHO GPIO_3

/** @def TRIGGER
 *  @brief Constante inicializar el TRIGGER con GPIO_2
*/
#define TRIGGER GPIO_2


/*==================[internal data definition]===============================*/

/** @brief Variable global que indica si se debe realizar la medición */
bool MedirON = true;

/** @brief Variable global que indica si se debe mantener la última medición en pantalla */
bool hold = true;

/** @fn modificarLed (uint16_t distancia)
 * @brief Funcion que recibe la distancia y según su valor enciende o paga LEDs.
 * @param distancia valor de la distancia
*/
void modificarLed (uint16_t distancia) {
    if (distancia < 10){
        LedOff(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);

    } else if (distancia > 10 && distancia < 20){
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    } else if (distancia > 20 && distancia < 30){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    } else if (distancia > 30){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    };
}

/** @fn DistanceTask(void *pvParameter){
 * @brief Tarea para medir distancia y controlar el estado de los LEDs.
*/
static void LedTask(void *pvParameter){
	uint16_t distancia;
    while(1){
        if (MedirON == true){
            distancia = HcSr04ReadDistanceInCentimeters();
            modificarLed(distancia);
            if (hold){
                LcdItsE0803Write(distancia);
            }
            
        } else if (MedirON == false) {
            LedsOffAll();
            LcdItsE0803Off();
        }


        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);

    }
}

/** @fn OnOffTask(void *pvParameter)
 * @brief Tarea para controlar el estado de los LEDs.
*/
static void OnOffTask(void *pvParameter){
    uint8_t teclas;
    while (1)
    {
        teclas  = SwitchesRead();
        if (teclas == SWITCH_1){
            MedirON = !MedirON;
        } else if (teclas == SWITCH_2){
            hold = !hold;
        }
        vTaskDelay(PERDIOD_SWITCH / portTICK_PERIOD_MS);
    } 
}

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/


void app_main(void){
	HcSr04Init(ECHO, TRIGGER); // inicializa HcSr04Init(eco, triger)
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    xTaskCreate(&LedTask, "medir", 2048, NULL, 5, NULL);
    xTaskCreate(&OnOffTask, "encender y apagar la medicion", 512, NULL, 5, NULL);
    
}
/*==================[end of file]============================================*/
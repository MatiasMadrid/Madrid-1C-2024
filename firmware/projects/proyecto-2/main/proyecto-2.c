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

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define PERDIOD_SWITCH 50
#define ECHO GPIO_3
#define TRIGGER GPIO_2

//uint16_t distancia_medida = 0


/*==================[internal data definition]===============================*/

bool MedirON = true;
bool hold = true;

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

static void LedTask(void *pvParameter){
	uint16_t distancia;
    while(1){
        if (MedirON == true){

            //LcdItsE0803
            distancia = HcSr04ReadDistanceInCentimeters();
            //distancia_medida = distancia;
            modificarLed(distancia);
            //printf("distancia %d \n",distancia);
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
        //printf("tecla %d \n",MedirON);
    } 
}

/*==================[internal functions declaration]=========================*/

TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;

/*==================[external functions definition]==========================*/


void app_main(void){
	HcSr04Init(ECHO, TRIGGER); // inicializa HcSr04Init(eco, triger)
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    xTaskCreate(&LedTask, "medir", 2048, NULL, 5, NULL);
    xTaskCreate(&OnOffTask, "encender y apagar la medicion", 512, NULL, 5, NULL);
    //xTaskCreate(&mostrar-por-display,"mostrar por display", 2048, NULL, 5, NULL);

   // xTaskCreate(&Led1Task, "apagar-y-prender", 2048, NULL, 5, NULL);
    //xTaskCreate(&Led2Task, "LED_2", 512, NULL, 5, &led2_task_handle);
    //xTaskCreate(&Led3Task, "LED_3", 512, NULL, 5, &led3_task_handle);
	
}
/*==================[end of file]============================================*/
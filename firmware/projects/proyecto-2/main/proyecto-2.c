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

/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500

#define ON 1
#define OFF 2
#define ECHO GPIO_3
#define TRIGGER GPIO_2


/*==================[internal data definition]===============================*/

struct leds
{
	uint8_t n_led ;     //  indica el número de led a controlar
	uint8_t mode;      // ON, OFF, TOGGLE

} my_leds; 	

void modificarLed (uint16_t distancia, struct leds *ptro_led){
    if (distancia < 10){
        switch (ptro_led->n_led)
        {
        case 1:
            LedOn()
            break;
        
        case 2:
            /* code */
            break;
        
        case 3:
            /* code */
            break;
        
        default:
            break;
        }
        
    }
    if (distancia > 10 && distancia < 20){
        ptro_led->mode = ON;
    }
    if (distancia > 20 && distancia < 30)
}

static void Led1Task(void *pvParameter){
	uint16_t distancia;
    while(1){
		distancia = HcSr04ReadDistanceInCentimeters();
		
		printf("distancia %d \n",distancia);
		LedOn(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);

    }
}

static void Led2Task(void *pvParameter){
    while(1){
        printf("LED_2 ON\n");
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
        printf("LED_2 OFF\n");
        LedOff(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
    }
}

static void Led3Task(void *pvParameter){
    while(true){
        printf("LED_3 ON\n");
        LedOn(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
        printf("LED_3 OFF\n");
        LedOff(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
    }
}
/*==================[internal functions declaration]=========================*/

TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;

/*==================[external functions definition]==========================*/

// inicializa HcSr04Init(eco, triger)
void app_main(void){
	LedsInit();
    struct leds miLed1, miLed2, miLed3;
	miLed1.mode = OFF;
	miLed1.n_led = 1;

	miLed2.mode = OFF;
	miLed2.n_led = 2;

	miLed3.mode = OFF;
	miLed3.n_led = 3;

	HcSr04Init(ECHO, TRIGGER);

    xTaskCreate(&Led1Task, "medir", 2048, NULL, 5, NULL);
    xTaskCreate(&Led1Task, "apagar-y-prender", 2048, NULL, 5, NULL);
    //xTaskCreate(&Led2Task, "LED_2", 512, NULL, 5, &led2_task_handle);
    //xTaskCreate(&Led3Task, "LED_3", 512, NULL, 5, &led3_task_handle);
	
}
/*==================[end of file]============================================*/
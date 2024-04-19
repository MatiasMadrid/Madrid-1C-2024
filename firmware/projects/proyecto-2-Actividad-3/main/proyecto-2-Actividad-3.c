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
#include "timer_mcu.h"

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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        if (MedirON == true){
            distancia = HcSr04ReadDistanceInCentimeters();
            //distancia_medida = distancia;
            modificarLed(distancia);
            printf("distancia %d \n",distancia);
            if (hold){
                LcdItsE0803Write(distancia);
            }
            
        } else if (MedirON == false) {
            LedsOffAll();
            LcdItsE0803Off();
        }


        //vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS); //borrar 

    }
}

static void OnOffTask(void *pvParameter){
    uint8_t teclas;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        teclas  = SwitchesRead();
        if (teclas == SWITCH_1){
            MedirON = !MedirON;
        } else if (teclas == SWITCH_2){
            hold = !hold;
        }
        //
        //printf("tecla %d \n",MedirON);
        //vTaskDelay(PERDIOD_SWITCH / portTICK_PERIOD_MS); 
    } 
    
}

static void Led3Task(void *pvParameter){

    
}
/*==================[internal functions declaration]=========================*/

TaskHandle_t task_handle_medir_y_on_off = NULL;
TaskHandle_t task_handle_OnOff_medir = NULL;

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimerMedir(void* param){
    vTaskNotifyGiveFromISR(task_handle_medir_y_on_off, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

void funcTimerOnOffMedir(void* param){
    vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE);
}



/*==================[external functions definition]==========================*/

// inicializa HcSr04Init(eco, triger)
void app_main(void){
    //inicialización de timers
    timer_config_t timer_task_1 = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_LED_1,
        .func_p = funcTimerMedir,
        .param_p = NULL
    };
    timer_config_t timer_task_2 = {
        .timer = TIMER_B,
        .period = PERDIOD_SWITCH,
        .func_p = funcTimerOnOffMedir,
        .param_p = NULL
    };

    //inicialización de perifericos
	HcSr04Init(ECHO, TRIGGER);
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();

    //creación de tareas
    xTaskCreate(&LedTask, "medir", 2048, NULL, 5, &task_handle_medir_y_on_off);
    xTaskCreate(&OnOffTask, "encender y apagar la medicion", 512, NULL, 5, &task_handle_OnOff_medir);

	
}
/*==================[end of file]============================================*/
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

#define TIME_PERIOD 1000000 //1000000 ???
#define ECHO GPIO_3
#define TRIGGER GPIO_2
uint8_t ACTUAL_SWITCH; //en caso de borrar tarea, borrar

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

static void DistanceTask(void *pvParameter){
	uint16_t distancia;
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */

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


    }
}

static void OnOffTask(void *pvParameter){

    //uint8_t teclas;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */
        //teclas  = SwitchesRead();
        if (ACTUAL_SWITCH == SWITCH_1){
            MedirON = !MedirON;
        } else if (ACTUAL_SWITCH == SWITCH_2){
            hold = !hold;
        }
    }
    
}

void read_switch(void){
    ACTUAL_SWITCH = SwitchesRead();
}

/*
void on_off(void){
    uint8_t tecla;
    tecla  = SwitchesRead();
    if (tecla == SWITCH_1){
        MedirON = !MedirON;
    }
}

void hold(void){
    uint8_t tecla;
    tecla  = SwitchesRead();
    if (tecla == SWITCH_2){
        hold = !hold;
    }
}
*/

/*==================[internal functions declaration]=========================*/

TaskHandle_t task_handle_medir = NULL;
TaskHandle_t task_handle_OnOff_medir = NULL;

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle_medir, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
    vTaskNotifyGiveFromISR(task_handle_OnOff_medir, pdFALSE); //BORRAR
}



/*==================[external functions definition]==========================*/

// inicializa HcSr04Init(eco, triger)
void app_main(void){
    //inicialización de timers
    timer_config_t timer_global = {
        .timer = TIMER_A,
        .period = TIME_PERIOD,
        .func_p = funcTimer,
        .param_p = NULL
    };


    //inicialización de perifericos
	HcSr04Init(ECHO, TRIGGER);
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();

    //creación de tareas


    SwitchActivInt(SWITCH_1, &read_switch, NULL); //on_off
    SwitchActivInt(SWITCH_2, &read_switch, NULL); //hold

    xTaskCreate(&DistanceTask, "medir", 2048, NULL, 5, &task_handle_medir);
    xTaskCreate(&OnOffTask, "encender y apagar la medicion", 512, NULL, 5, &task_handle_OnOff_medir); //borrar tarea? esta controla las teclas, cambiarla por funciones
                                                                                                    //en el switchactivint

    TimerStart(timer_global.timer);
	
}
/*==================[end of file]============================================*/
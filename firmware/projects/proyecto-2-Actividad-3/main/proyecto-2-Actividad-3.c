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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/

#define TIME_PERIOD 1000000
#define ECHO GPIO_3
#define TRIGGER GPIO_2


/*==================[internal data definition]===============================*/

bool MedirON = true;
bool hold = false;
bool SI = true;

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

void revisarTeclas(){
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);    

    if (tecla == 'O'){
        MedirON = !MedirON;
    } else if (tecla == 'H'){
        hold = !hold;
    }

   /*
    switch (tecla)
    {
    case 'O':
        MedirON = !MedirON;
        break;
    case 'H':
        hold = !hold;
        break;
    case 'I':
        SI = !SI;
        break;
    default:
        break;
    }*/
}

static void DistanceTask(void *pvParameter){
	uint16_t distancia;
    while(1){

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* La tarea espera en este punto hasta recibir una notificación */
    

        if (MedirON == true){
            distancia = HcSr04ReadDistanceInCentimeters();
            modificarLed(distancia);
            UartSendString(UART_PC, (char*) UartItoa(distancia,10));
            UartSendString(UART_PC, " cm\n\r");
            if (hold){
                LcdItsE0803Write(distancia);
            }
            
        } else if ((MedirON == false)) {
            LedsOffAll();
            LcdItsE0803Off();
        }

    }
}

void read_switch1(void){
    MedirON = !MedirON;

}

void read_switch2(void){
    hold = !hold;

}


/*==================[internal functions declaration]=========================*/

TaskHandle_t task_handle_medir = NULL;

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle_medir, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */

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

    serial_config_t serial_global = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = revisarTeclas,
        .param_p = NULL
    };

    UartInit(&serial_global);
    TimerInit(&timer_global);
    
	HcSr04Init(ECHO, TRIGGER);
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();

    SwitchActivInt(SWITCH_1, &read_switch1, NULL); 
    SwitchActivInt(SWITCH_2, &read_switch2, NULL); 

    xTaskCreate(&DistanceTask, "medir", 4096, NULL, 5, &task_handle_medir);

    TimerStart(timer_global.timer);
	
}
/*==================[end of file]============================================*/
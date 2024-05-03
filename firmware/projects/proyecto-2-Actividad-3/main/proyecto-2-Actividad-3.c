/*! @mainpage Proyecto 2 / Actividad 3
 *
 * @section genDesc General Description
 *
 * Código diseñado para medir distancia, mostrarla por un display LCD y por puerto serie.
 * Además enciende LEDs dependiendo que distancia esté midiendo.
 * Tiene la funcionalidad (mediente interrupciones) de encender y detener la medición, y mantener el resultado por pantalla, 
 * mediante botones de la placa o por puerto serie
 *
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
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 39/04/2024 | Document creation		                         |
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
#include "timer_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def TIME_PERIOD
 *  @brief Periodo del timer (en us)
*/
#define TIME_PERIOD 1000000

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

/** @fn revisarTeclas()
 * @brief Función para cambiar el estado de las banderas por puerto serie
*/
void revisarTeclas(){
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);    

    if (tecla == 'O'){
        MedirON = !MedirON;
    } else if (tecla == 'H'){
        hold = !hold;
    }
}

/** @fn DistanceTask(void *pvParameter)
 * @brief Tarea para medir distancia y controlar el estado de los LEDs.
*/
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

/** @fn read_switch1(void)
 * @brief Función para cambiar el estado de la bandera MedirON al presionar la tecla switch1.
*/
void read_switch1(void){
    MedirON = !MedirON;
}

/** @fn read_switch2(void)
 * @brief Función para cambiar el estado de la bandera hold al presionar la tecla switch2.
*/
void read_switch2(void){
    hold = !hold;
}


/*==================[internal functions declaration]=========================*/

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea*/
TaskHandle_t task_handle_medir = NULL;

/**
 * @brief Función invocada en la interrupción del timer A
 */
void funcTimer(void* param){
    vTaskNotifyGiveFromISR(task_handle_medir, pdFALSE);  
}

/*==================[external functions definition]==========================*/

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
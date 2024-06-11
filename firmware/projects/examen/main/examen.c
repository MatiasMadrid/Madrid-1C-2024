/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * Examen parcial 1C 2024
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    periferico		 |   ESP32   	|
 * |:-------------------:|:--------------|
 * | 	sensor de humedad	 |  GPIO1		|
 * | 	sensor pH		 	 | 	GPIO2		|
 * | 	bomba de agua	 	 | 	GPIO10		|
 * |    bomba acida 		 |  GPIO11	 	|
 * | 	bomba basica 		 |  GPIO12 		|
 * 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Madrid Pablo Matias (pablo.madrid@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "gpio_mcu.h"
#include "switch.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/** @def PERIODO_LECTURA
 *  @brief Tiempo de espera (ms) para la lectura de datos por parte de los sensores
*/
#define PERIODO_LECTURA 3000

/** @def PERIODO_ENVIO_DE_DATOS
 *  @brief Tiempo de espera (ms) para enviar datos por la UART
*/
#define PERIODO_ENVIO_DE_DATOS 5000

/*==================[internal data definition]===============================*/
/** @brief Variable global que almacena el valor actual de ph */
uint8_t valor_ph = 0;

/** @brief Variable global que almacena el valor actual de humedad medido */
uint8_t valor_humedad = 0; 

/** @brief Variable global que indica si el sistema está encendido */
bool sistemaON = true;

/** @brief Variable global que indica si el pH esta en un rango correcto */
bool pHCorrecto = true;

/** @brief Variable global que indica el umbral de humedad del sistema */
uint8_t umbral_humedad = 80; //se settea el valor de humedad en 80 (a preferencia del ususario)


/** @struct gpioConf_t
 *  @brief Estructura de GPIOs, que contiene numero de pin y dir que representa el estado (IN, OUT)
*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/** @fn read_switch1(void)
 * @brief Función para cambiar el estado de la bandera MedirON al presionar la tecla switch1.
*/
void read_switch1(void){
    sistemaON = true;
}

/** @fn read_switch2(void)
 * @brief Función para cambiar el estado de la bandera hold al presionar la tecla switch2.
*/
void read_switch2(void){
    sistemaON = false;
}

/** @fn control_agua(void *pvParameter)
 * @brief Tarea para medir la humedad y controlar la bomba de agua.
*/
static void control_agua(void *pvParameter){
	
	while (1){
		if (sistemaON == true){
			valor_humedad = sensorHumedadrelativaread(); //devuelve un valor de humedad entre 0 - 100

			if (valor_humedad < umbral_humedad){
				GPIOOn(bomba_agua.pin); //pone en ON el GPIO de la bomba de agua si la humedad está por debajo del valor umbral
			} else {
				GPIOOff(bomba_agua.pin);//pone en OFF el GPIO de la bomba de agua si la humedad está por arriba del valor umbral
			}
		}
		vTaskDelay(PERIODO_LECTURA / portTICK_PERIOD_MS);
	}
	
}

/** @fn control_ph(void *pvParameter)
 * @brief Tarea para medir el pH y controlar las bombas de pH
*/
static void control_ph(void *pvParameter){
	uint16_t valorAnalogico_ph = 0; 
	//se declaran los umbrales inferiores y superiores de acuerdo a los valor que entrega el sensor de ph
	float umbral_inferior = (6.0 * 3.0)/14; 
	float umbral_superior = (6.7 * 3.0)/14;
	while (1){

		if (sistemaON == true){
			AnalogInputReadSingle(CH2, &valorAnalogico_ph); // lee el valor analógico que entra por CH2 del sensor de ph
			//enciende la bomba de ph basico
			if (valor_ph < umbral_inferior){
				GPIOOn(bomba_phb.pin);
				pHCorrecto = false;
			}
			//enciende la bomba de ph acido
			if (valor_ph > umbral_superior){
				GPIOOn(bomba_pha.pin);
				pHCorrecto = false;
			}

			//apaga ambas bombas (estado deseable)
			if ((valor_ph < umbral_superior) && (valor_ph > umbral_inferior)){
				GPIOOff(bomba_phb.pin);
				GPIOOff(bomba_pha.pin);
				pHCorrecto = true;
			}

		}
		valor_ph = (valorAnalogico_ph * 14) / 3;
		vTaskDelay(PERIODO_LECTURA / portTICK_PERIOD_MS);
	}
	
}

/** @fn informar(void *pvParameter)
 * @brief Tarea para medir informar via UART cada 5 segundos el estado del sistema al usuario
*/
static void informar(void *pvParameter){
	while (1){

		if (sistemaON == true){
			UartSendString(UART_PC, " humedad: ");
			UartSendString(UART_PC, (char*) UartItoa(valor_humedad,10));
			UartSendString(UART_PC, " \n\r");

			UartSendString(UART_PC, " pH: ");
			UartSendString(UART_PC, (char*) UartItoa(valor_ph,10));
			UartSendString(UART_PC, ", ");
			if (pHCorrecto == true){
				UartSendString(UART_PC, " humedad correcta  \n\r");
			} else {
				UartSendString(UART_PC, " humedad incorrecta  \n\r");
			}
		} else {
			UartSendString(UART_PC, " Bombas apagadas\n\r");
		}

		vTaskDelay(PERIODO_ENVIO_DE_DATOS / portTICK_PERIOD_MS);
	}
	
}

/*==================[external functions definition]==========================*/
void app_main(void){

	gpioConf_t bomba_agua = {
		.dir GPIO_10,
		.pin GPIO_OUTPUT
	};

	gpioConf_t bomba_pha = {
		.dir GPIO_11,
		.pin GPIO_OUTPUT
	};
		
	gpioConf_t bomba_phb = {
		.dir GPIO_12,
		.pin GPIO_OUTPUT
	};
	//se inicializan los GPIO de las bombas
	GPIOInit(bomba_agua.pin, bomba_agua.dir);
	GPIOInit(bomba_pha.pin, bomba_pha.dir);
	GPIOInit(bomba_phb.pin, bomba_phb.dir);

	analog_input_config_t analog_input = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL
	};
	//se iniciliza la entrada analogica del sensor de ph
	AnalogInputInit(&analog_input);

	serial_config_t serial_global = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = NULL,
        .param_p = NULL
    };
	//se inicia puerto UART
	UartInit(&serial_global); 

	//funcion para inicializar el sensor de humedad en el GPIO1 propia del drive del sensor de humedad (supuesta)
	Sensor_humedadInit(GPIO_1); 

	//Se inicilizan los swhitches
	SwitchesInit();

    SwitchActivInt(SWITCH_1, &read_switch1, NULL); 
    SwitchActivInt(SWITCH_2, &read_switch2, NULL); 

	xTaskCreate(&control_agua, "mide la humedad y controla la bomba de agua", 4096, NULL, 5, NULL);
	xTaskCreate(&control_ph, "mide el ph y controla las bombas de pH", 4096, NULL, 5, NULL);
	xTaskCreate(&informar, "informa via UART", 1024, NULL, 5, NULL);
}
/*==================[end of file]============================================*/
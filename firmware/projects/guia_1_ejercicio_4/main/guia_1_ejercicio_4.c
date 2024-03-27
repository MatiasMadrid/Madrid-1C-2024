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
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h" //OS
#include "freertos/task.h"
#include "led.h" 
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

#define ON = 1
#define OFF = 0

/*==================[internal functions declaration]=========================*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{ 
	for (uint i = digits; i>0; i--){
		bcd_number[i-1] = data % 10;
		data = data / 10;
	}
}

void modificarStatus(uint32_t data, gpioConf_t *vectorGPIO){
	for (uint8_t i=0; i<4; i++){
		if ((data&(1<<i)) == 0){
			GPIOOff(vectorGPIO[i].pin);
		}
		else{
			GPIOOn(vectorGPIO[i].pin);
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){


	uint32_t numero = 13;
	uint digitos = 2;
	uint8_t arreglo[digitos];
	convertToBcdArray (numero, digitos, arreglo); //se convierte de decimal a BCD
	for (int i = 0; i < digitos; i++){
		printf(" valor : %d", arreglo[i]);
	}
	
	gpioConf_t vectorGPIO[4] = {{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};
	//for ()
	GPIOInit(vectorGPIO[0].pin, vectorGPIO[0].dir);
	GPIOInit(vectorGPIO[1].pin, vectorGPIO[1].dir);
	GPIOInit(vectorGPIO[2].pin, vectorGPIO[2].dir);
	GPIOInit(vectorGPIO[3].pin, vectorGPIO[3].dir);

	//gpioConf_t vectorGPIO_map[3] = {{GPIO_19,GPIO_OUTPUT}, {GPIO_18,GPIO_OUTPUT}, {GPIO_9,GPIO_OUTPUT}};

	for (int i = 0; i < digitos; i++){
		//vectorGPIO[i].GPIOInit();
		modificarStatus(arreglo[i], vectorGPIO);
	}
	
	//displayLeds(numero, digitos, vectorGPIO, vectorGPIO_map)
	
}
/*==================[end of file]============================================*/
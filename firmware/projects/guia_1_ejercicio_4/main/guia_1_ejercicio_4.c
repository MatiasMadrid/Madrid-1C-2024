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
#define N_DIGITOS = 3

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
void displayLeds(uint32_t data, uint digitos, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_map){
	
	uint8_t arreglo[digitos];
	convertToBcdArray (data, digitos, arreglo);

	for (int i=0; i<3; i++){ //define a digitos = 3
		//on/off (pulso?)

		modificarStatus(arreglo[i], vectorGPIO);

		GPIOOn(vectorGPIO_map[i].pin);
		GPIOOff(vectorGPIO_map[i].pin);
		
		
		
	}
}



/*==================[external functions definition]==========================*/
void app_main(void){


	uint32_t numero = 735;
	uint digitos = 3;

	//uint8_t arreglo[digitos];
	//convertToBcdArray (numero, digitos, arreglo); //se convierte de decimal a BCD
	//for (int i = 0; i < digitos; i++){
	//	printf(" valor : %d", arreglo[i]);
	//}
	
	gpioConf_t vectorGPIOs[4] = {{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}};
	//for ()
	GPIOInit(vectorGPIOs[0].pin, vectorGPIOs[0].dir);
	GPIOInit(vectorGPIOs[1].pin, vectorGPIOs[1].dir);
	GPIOInit(vectorGPIOs[2].pin, vectorGPIOs[2].dir);
	GPIOInit(vectorGPIOs[3].pin, vectorGPIOs[3].dir);

	gpioConf_t vectorGPIO_mapeo[3] = {{GPIO_19,GPIO_OUTPUT},{GPIO_18,GPIO_OUTPUT},{GPIO_9,GPIO_OUTPUT}};
	GPIOInit(vectorGPIO_mapeo[0].pin, vectorGPIO_mapeo[0].dir);
	GPIOInit(vectorGPIO_mapeo[1].pin, vectorGPIO_mapeo[1].dir);
	GPIOInit(vectorGPIO_mapeo[2].pin, vectorGPIO_mapeo[2].dir);

	//for (int i = 0; i < digitos; i++){
	//
	//	modificarStatus(arreglo[i], vectorGPIOs);
	//}
	
	displayLeds(numero, digitos, vectorGPIOs, vectorGPIO_mapeo);
	
}
/*==================[end of file]============================================*/
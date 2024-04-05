/*! @mainpage Ejercicio_4
 *
 * @section genDesc General Description
 *
 * Este código permite mostrar en la placa LCD un número de hasta tres dígitos.
 * El número decimal, se transforma en BCD para que pueda ser interpretado por el display
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 *
 * @author Madrid Matias (pablo.madrid@ingenieria.uner.edu.ar)
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

/** @def N_DIGITOS
 *  @brief Constante con el numero máximo de digitos
*/
#define N_DIGITOS 3

/*==================[internal functions declaration]=========================*/
/** @struct gpioConf_t
 *  @brief Estructura de GPIOs, que contiene numero de pin y dir que representa el estado (IN, OUT)
*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/** @fn convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief Funcion que recibe un número decimal y le otorga a un arreglo un digito en BCD de dicho numero en cada una de sus posiciones
 * @param data valor del número decimal
 * @param digitis cantidad de dígitos del numero
 * @param bcd_number puntero a un arreglo
*/
void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{ 
	for (uint i = digits; i>0; i--){
		bcd_number[i-1] = data % 10;
		data = data / 10;
	}
}

/** @fn modificarStatus(uint32_t data, gpioConf_t *vectorGPIO)
 * @brief Esta función recibe un número en BCD (dígito) y un vector de GPIO, enciende o apaga el GPIO correspondiente para que codifiquen dicho número
 * @param data número en BCD
 * @param vectorGPIO puntero a vector de GPIO
*/
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

/** @fn displayLeds(uint32_t data, uint digitos, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_map)
 * @brief Función que recibe un número decimal, la cantidad de dígitos, un vector de GPIO para decodificar un nnumero y un vector GPIO para mapear los digitos a cada led. Muestra el numero en el LCD.
 * @param data número decimal
 * @param digitos cantidad de dígitos
 * @param vectorGPIO vector de GPIO que codifica el numero (20, 21, 22 y 23)
 * @param vectorGPIO_map vector de GPIO que mapea a cada BCD de 7 segmentos (19, 18 y 9)
*/
void displayLeds(uint32_t data, uint digitos, gpioConf_t *vectorGPIO, gpioConf_t *vectorGPIO_map){
	
	uint8_t arreglo[digitos];
	convertToBcdArray (data, digitos, arreglo);

	for (int i=0; i<N_DIGITOS; i++){ //define a digitos = 3
		//on/off (pulso?)

		modificarStatus(arreglo[i], vectorGPIO);

		GPIOOn(vectorGPIO_map[i].pin);
		GPIOOff(vectorGPIO_map[i].pin);
		
	}
}



/*==================[external functions definition]==========================*/
void app_main(void){


	uint32_t numero = 738;
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
//#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

struct RLstat {
	int moving;
	int speed;
	int direction;
	int position;
};

static struct RLstat RL = {
	.moving = 0,
	.speed = 5,
	.direction = 0,
	.position = 0,
};

void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void button_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));

void timer_ISR(void)
{
	//REVISAR: tomar el c�digo de avance de posici�n del led rotante de la pr�ctica anterior
	if (RL.moving) {
		if (RL.direction) {
			RL.position++;
			if (RL.position > 5)
				RL.position = 0;
		} else {
			RL.position--;
			if (RL.position < 0)
				RL.position = 5;
		}
		D8Led_segment(RL.position);

	}
	D8Led_segment(RL.position);
}

void button_ISR(void)
{
	unsigned int whicheint = rEXTINTPND;
	unsigned int buttons = (whicheint >> 2) & 0x3;

	if (buttons & BUT1) {
		led1_off();
		led2_off();
		if(RL.direction)
			RL.direction = 0;
		else
			RL.direction = 1;

		// REVISAR: utilizando la interfaz para los leds definida en leds.h
		// hay que apagar ambos leds
		// Tambi�n hay que comutar la direcci�n del movimiento del led rotante
		// representado por el campo direction de la variable RL
	}

	if (buttons & BUT2) {
		if(RL.moving){
			RL.moving = 0;
		}
		else{
			RL.moving = 1;
		}
	}

	//REVISAR: usar el codigo de la primera parte parte de atencion a los
	//pulsadores

	// eliminamos rebotes
	Delay(2000);
	// borramos el flag en extintpnd
	rEXTINTPND = rEXTINTPND & ~(buttons << 2);
	//REVISAR: debemos borrar las peticiones de interrupci�n en
	//EXTINTPND escribiendo un 1 en los flags que queremos borrar (los
	//correspondientes a los pulsadores pulsados)
}


void keyboard_ISR(void)
{
	int key;

	/* Eliminar rebotes de presi�n */
	Delay(200);
	
	/* Escaneo de tecla */
	key = kb_scan();

	if (key != -1) {
		/* Visualizacion en el display */
		D8Led_digit(key);
		//REVISAR: mostrar la tecla en el display utilizando el interfaz
		//definido en D8Led.h

		switch (key) {
			case 0:
				tmr_set_divider(TIMER0, D1_8);
				tmr_set_count(TIMER0, 0, 62500);
				//REVISAR: poner en timer0 divisor 1/8 y contador 62500
				break;
			case 1:
				tmr_set_divider(TIMER0, D1_8);
				tmr_set_count(TIMER0, 0, 31250);
				//REVISAR: poner en timer0 timer divisor 1/8 y contador 31250
				break;
			case 2:
				tmr_set_divider(TIMER0, D1_8);
				tmr_set_count(TIMER0, 0, 15625);
				//REVISAR: poner en timer0 timer divisor 1/8 y contador 15625
				break;
			case 3:
				tmr_set_divider(TIMER0, D1_4);
				tmr_set_count(TIMER0, 0, 62500);
				//REVISAR: poner en timer0 timer divisor 1/4 y contador 15625
				break;
			default:
				break;
		}
		
		/* Esperar a que la tecla se suelte, consultando el registro de datos */		
		/*REVISAR: true si est� pulsada la tecla (leer del registro rPDATG)*/
		while (~rPDATG & 0x1);//eint1
	}

    /* Eliminar rebotes de depresi�n */
    Delay(200);
     
    /* Borrar interrupciones pendientes */
	//REVISAR
	//borrar la interrupci�n por la l�nea EINT1 en el registro rI_ISPC
    rI_ISPC = rI_ISPC & INT_BIT(24);
}

int setup(void)
{
	leds_init();
	D8Led_init();
	D8Led_segment(RL.position);

	/* Port G: configuraci�n para generaci�n de interrupciones externas,
	 *         botones y teclado
	 **/

	portG_conf(1, EINT);
	portG_conf(6, EINT);
	portG_conf(7, EINT);

	portG_eint_trig(1, FALLING);//Necesario?
	portG_eint_trig(6, FALLING);
	portG_eint_trig(7, FALLING);

	//Necesario resistencia en pin 1?
	portG_conf_pup(6, ENABLE);
	portG_conf_pup(7, ENABLE);
	//REVISAR: utilizando el interfaz para el puerto G definido en gpio.h
	//configurar los pines 1, 6 y 7 del puerto G para poder generar interrupciones
	//externas por flanco de bajada por ellos y activar las correspondientes
	//resistencias de pull-up.


	/********************************************************************/

	/* Configuraci�n del timer */


	//COMPLETAR: tomar el c�digo de la segunda parte

	tmr_set_prescaler(TIMER0, 255);
	tmr_set_divider(TIMER0, 8);
	tmr_set_count(TIMER0, 62500, 1);


	if (RL.moving)
		tmr_start(TIMER0);
	/***************************/

	// Registramos las ISRs
	pISR_TIMER0   = timer_ISR;//REVISAR: registrar la RTI del timer
	pISR_EINT4567 = button_ISR;//REVISAR: registrar la RTI de los botones
	pISR_EINT1    = keyboard_ISR;//REVISAR: registrar la RTI del teclado

	/* Configuraci�n del controlador de interrupciones
	 * Habilitamos la l�nea IRQ, en modo vectorizado y registramos una ISR para
	 * la l�nea IRQ
	 * Configuramos el timer 0 en modo IRQ y habilitamos esta l�nea
	 * Configuramos la l�nea EINT4567 en modo IRQ y la habilitamos
	 * Configuramos la l�nea EINT1 en modo IRQ y la habilitamos
	 */

	ic_init();

	//REVISAR: utilizando el interfaz definido en intcontroller.h
	//		habilitar la l�nea IRQ en modo vectorizado
	//		deshabilitar la l�nea FIQ
	//		configurar la l�nea INT_TIMER0 en modo IRQ
	//		configurar la l�nea INT_EINT4567 en modo IRQ
	//		configurar la l�nea INT_EINT1 en modo IRQ
	//		habilitar la l�nea INT_TIMER0
	//		habilitar la l�nea INT_EINT4567
	//		habilitar la l�nea INT_EINT1

	/***************************************************/

	ic_conf_irq(ENABLE, VEC);
	ic_conf_fiq(DISABLE);

	ic_conf_line(INT_TIMER0, IRQ);
	ic_conf_line(INT_EINT4567, IRQ);
	ic_conf_line(INT_EINT1, IRQ);

	ic_enable(INT_TIMER0);
	ic_enable(INT_EINT4567);
	ic_enable(INT_EINT1);


	Delay(0);
	return 0;
}

int loop(void)
{
	return 0;
}


int main(void)
{
	setup();

	while (1) {
		loop();
	}
}

#include <stdio.h>
#include "44b.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

#define N 4

/* Variables para la gestión de la ISR del teclado
 *
 * Keybuffer: puntero que apuntará al buffer en el que la ISR del teclado debe
 *            almacenar las teclas pulsadas
 * keyCount: variable en el que la ISR del teclado almacenará el número de teclas pulsadas
 */
volatile static char *keyBuffer = NULL;
volatile static int keyCount = 0;

/* Variables para la gestion de la ISR del timer
 *
 * tmrbuffer: puntero que apuntará al buffer que contendrá los dígitos que la ISR del
 *            timer debe mostrar en el display de 8 segmentos
 * tmrBuffSize: usado por printD8Led para indicar el tamaño del buffer a mostrar
 */
volatile static char *tmrBuffer = NULL;
volatile static int tmrBuffSize = 0;

//Variables globales para la gestión del juego
static char passwd[N];  //Buffer para guardar la clave inicial
static char guess[N];   //Buffer para guardar la segunda clave

enum state {
	INIT = 0,     //Init:       Inicio del juego
	SPWD = 1,     //Show Pwd:   Mostrar password
	DOGUESS = 2,  //Do guess:   Adivinar contraseña
	SGUESS = 3,   //Show guess: Mostrar el intento
	GOVER = 4     //Game Over:  Mostrar el resultado
};
enum state gstate; //estado/fase del juego

//REVISAR: Declaración adelantada de las ISRs de timer y teclado (las marca como ISRs)
void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));


// Función que va guardando las teclas pulsadas
static void push_buffer(char *buffer, int key)
{
	int i;
	for (i=0; i < N-1; i++)
		buffer[i] = buffer[i+1];
	buffer[N-1] = (char) key;
}

void timer_ISR(void)
{
	static int pos = 0; //contador para llevar la cuenta del dígito del buffer que toca mostrar

    //REVISAR: Visualizar el dígito en la posición pos del buffer tmrBuffer en el display

	if(pos >= tmrBuffSize) {
		pos = 0;
		tmr_stop(TIMER0);
		tmrBuffer = NULL;
	} else {
		D8Led_segment(tmrBuffer[pos]);
		pos++;
	}

	// Si es el último dígito:
	//      Poner pos a cero,
	//      Parar timer
	//      Dar tmrBuffer valor NULL

	// Si no, se apunta al siguiente dígito a visualizar (pos)

	// REVISAR: Finalizar correctamente la ISR
	ic_cleanflag(INT_TIMER0);
}

void printD8Led(char *buffer, int size)
{
	//Esta rutina prepara el buffer que debe usar timer_ISR (tmrBuffer)
	tmrBuffer = buffer;
	tmrBuffSize = size;

	//REVISAR: Arrancar el TIMER0
	tmr_start(TIMER0);

	while(tmrBuffer != NULL);
	//REVISAR: Esperar a que timer_ISR termine (tmrBuffer)
}

void keyboard_ISR(void)
{
	int key;

	/* Eliminar rebotes de presión */
	Delay(200);

	/* Escaneo de tecla */
	// REVISAR

	key = kb_scan();

	if (key != -1) {
		//REVISAR:
		//Si la tecla pulsada es F deshabilitar interrupciones por teclado y
		//poner keyBuffer a NULL

		// Si la tecla no es F guardamos la tecla pulsada en el buffer apuntado
		// por keybuffer mediante la llamada a la rutina push_buffer

		if(key == 0xf) {
			ic_disable(INT_EINT1);
			keyBuffer = NULL;
		} else {
			push_buffer(keyBuffer, key);
		}


		// Actualizamos la cuenta del número de teclas pulsadas
		keyCount++;

		/* Esperar a que la tecla se suelte, consultando el registro de datos rPDATG */
		while (!(rPDATG & 0x02));
	}

	/* Eliminar rebotes de depresiÃ³n */
	Delay(200);

	//REVISAR: Finalizar correctamente la ISR
	ic_cleanflag(INT_EINT1);
}

int read_kbd(char *buffer)
{
	//Esta rutina prepara el buffer en el que keyboard_ISR almacenará las teclas
	//pulsadas (keyBuffer) y pone a 0 el contador de teclas pulsadas
	keyBuffer = buffer;
	keyCount = 0;

	//REVISAR: Habilitar interrupciones por teclado
	ic_enable(INT_EINT1);


	//REVISAR: Esperar a que keyboard_ISR indique que se ha terminado de
	//introducir la clave (keyBuffer)
	while(keyBuffer != NULL);


	//REVISAR: Devolver número de teclas pulsadas
	return keyCount;
}

static int show_result()
{
	int error = 0;
	int i = 0;
	char buffer[2] = {0};

	// REVISAR: poner error a 1 si las contraseñas son distintas

	while(error == 0 && i < N){
		if(passwd[i]!=guess[i]) error = 1;
		i++;
	}

	if(error){
		buffer[0] = 0xE;
		buffer[1] = 0xE;
	} else {
		buffer[0] = 0xA;
		buffer[1] = 0xA;
	}
	printD8Led(&buffer);

	// REVISAR
	// Hay que visualizar el resultado durante 2s.
	// Si se ha acertado tenemos que mostrar una A y si no una E
	// Como en printD8Led haremos que la ISR del timer muestre un buffer con dos
	// caracteres A o dos caracteres E (eso durará 2s)

	// REVISAR: esperar a que la ISR del timer indique que se ha terminado
	while(tmrBuffer!=NULL);

	// REVISAR: Devolver el valor de error para indicar si se ha acertado o no
	return error;
}

int setup(void)
{

	D8Led_init();

	/* REVISAR: Configuración del timer0 para interrumpir cada segundo */
	tmr_set_prescaler(TIMER0, 255);
	tmr_set_divider(TIMER0, 8);
	tmr_set_count(TIMER0,62500, 1);


	/***************************/
	/* REVISAR: Port G-configuración para generación de interrupciones externas
	 *         por teclado
	 **/
	porG_conf(1, EINT);
	portG_eint_trig(1, FALLING);
	portG_conf_pup(1, ENABLE);//REVISAR


	/********************************************************************/

	// REVISAR: Registramos las ISRs
	pISR_TIMER0 = timer_ISR;
	pISR_EINT1 = keyboard_ISR;


	/* Configuración del controlador de interrupciones*/


	ic_init();

	ic_conf_irq(ENABLE,  VEC);
	ic_conf_line(INT_TIMER0, IRQ);
	ic_conf_line(INT_EINT1, IRQ);
	ic_enable(TIMER0);
	ic_enable(INT_EINT1);


	 /* Habilitamos la línea IRQ, en modo vectorizado y registramos una ISR para
		 * la línea IRQ
		 * Configuramos el timer 0 en modo IRQ y habilitamos esta línea
		 * Configuramos la línea EINT1 en modo IRQ y la habilitamos
		 */

	/***************************************************/

	Delay(0);

	/* Inicio del juego */
	gstate = INIT;
	D8Led_digit(12);

	return 0;
}


int loop(void)
{
	int count; //número de teclas pulsadas
	int error;

	//Máquina de estados

	switch (gstate) {
		case INIT:
			int numteclas;
			do {
				D8Led_init();
				D8Led_digit(0xC);
				numteclas = read_kbd(passwd);
				if(numteclas < 4)
					D8Led_digit(0xE);
				//REVISAR:
    			//Visualizar una C en el display
     			//Llamar a la rutina read_kbd para guardar los 4 dígitos en el buffer passwd
     			//Esta rutina devuelve el número de teclas pulsadas.
				//Dibujar una E en el display si el número de teclas pulsadas es menor que 4
			}/*permanecer en el while mientras se hayan pulsado menos de 4 teclas*/
			while (numteclas < 4);

			//REVISAR: Pasar al estado siguiente
			gstate = SPWD;
			break;

		case SPWD:
			printD8Led(passwd, 4);
			// REVISAR:
			// Visualizar en el display los 4 dígitos del buffer passwd, para
			// ello llamar a la rutina printD8Led
			Delay(10000);
			//REVISAR: Pasar al estado siguiente
			gstate = DOGUESS;
			break;

		case DOGUESS:
			Delay(10000);
			int nteclas;
			do {
				D8Led_digit(0xF);
				nteclas = read_kbd(guess);
				if(numteclas < 4)
					D8Led_digit(0xE);
				//REVISAR:
				//Visualizar en el display una F
				//Llamar a la rutina read_kbd para guardar los 4 dígitos en el buffer guess
				//Esta rutina devuelve el número de teclas pulsadas.
				//Dibujar una E en el display si el número de teclas pulsadas es menor que 4
			} /*permanecer en el while mientras se hayan pulsado menos de 4 teclas*/
			while (nteclas < 4);

				gstate = SGUESS;
			//REVISAR: Pasar al estado siguiente

			break;

		case SGUESS:
			printD8Led(guess, 4);

			//REVISAR:
			//Visualizar en el display los 4 dígitos del buffer guess,
			//para ello llamar a la rutina printD8Led
			Delay(10000);
			//REVISAR: Pasar al estado siguiente
			gstate = GOVER;
			break;

		case GOVER:
			int results = show_result();
			//REVISAR:
			//Mostrar el mensaje de acierto o error con show_result()
			Delay(10000);
			//Si he acertado el estado siguiente es INIT sino DOGUESS
			if(results == 1)
				gstate = DOGUESS;
			else
				gstate = INIT;
			break;
	}
	return 0;
}

int main(void)
{
	setup();

	while (1) {
		loop();
	}
}

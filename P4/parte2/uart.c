#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "44b.h"
#include "uart.h"
#include "intcontroller.h"

#define BUFLEN 100

// Estructura utilizada para mantener el estado de cada puerto
struct port_stat {
	enum URxTxMode rxmode;       //Modo de recepci�n (DIS, POLL, INT, DMA)
	enum URxTxMode txmode;       //Modo de env�o     (DIS, POLL, INT, DMA)
	unsigned char ibuf[BUFLEN];  //Buffer de recepci�n (usado en modo INT)
	int rP;                      //Puntero de lectura en ibuf (modo INT)
	int wP;                      //Puntero de escritura en ibuf (modo INT)
	char *sendP;                 //Puntero a la cadena de env�o (modo INT)
	enum ONOFF echo;             //Marca si el puerto debe hacer eco de los caracteres recibidos
};

static struct port_stat uport[2]; //Array con el estado de los puertos

// REVISAR: Declaraci�n adelantada de las rutinas de tratamiento de
// interrupci�n de la uart por linea IRQ (las marca como ISRs)
// Las rutinas son: Uart0_RxInt, Uart0_TxInt, Uart1_RxInt, Uart1_TxInt
void Uart0_TxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart1_TxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart0_RxInt(void) __attribute__ ((interrupt ("IRQ")));
void Uart1_RxInt(void) __attribute__ ((interrupt ("IRQ")));

void uart_init(void)
{
	int i;

	// Inicializaci�n de las estructuras de estado de los puertos
	for (i=0; i < 2; i++) {
		uport[i].rxmode = DIS;
		uport[i].txmode = DIS;
		uport[i].rP = 0;
		uport[i].wP = 0;
		uport[i].sendP = NULL;
		uport[i].echo = OFF;
	}

	//REVISAR: Registrar adecuadamente las rutinas de tratamiento de
	//interrupci�n de la uart

	pISR_URXD0=Uart0_RxInt;
	pISR_URXD1=Uart1_RxInt;
	pISR_UTXD0=Uart0_TxInt;
	pISR_UTXD1=Uart1_TxInt;

	//REVISAR: Configurar las lineas de interrupci�n de la uart en modo IRQ
	ic_conf_line(INT_URXD0, IRQ);
	ic_conf_line(INT_URXD1, IRQ);
	ic_conf_line(INT_UTXD0, IRQ);
	ic_conf_line(INT_UTXD1, IRQ);
}
	
/* uart_lconf: Esta funci�n configura el modo linea de la uart,
 *       N�mero de bits por trama
 *       N�mero de bits de parada
 *       Paridad
 *       Modo infrarrojos
 *       Baudios
 * y configura los pines adecuados para que las lineas Rx y Tx de los puertos
 * salgan fuera del chip, hacia los conectores DB9 de la placa
 */
int uart_lconf(enum UART port, struct ulconf *lconf)
{
	unsigned int confvalue = 0; // valor de configuraci�n del registro ULCON
	int baud;
		
	// REVISAR: darle a confvalue el valor adecuado en funci�n de la
	// configuraci�n deseada (par�metro lconf)

	confvalue = (lconf->ired << 6) || ((lconf->par < 4) ? 0 : lconf->par + 3 << 3) || (lconf->stopb << 2) || lconf->wordlen;

	baud = (int)( MCLK /(16.0 * lconf->baud) + 0.5) - 1;

	switch (port) {
		case UART0:
			rULCON0 = confvalue;
			rUBRDIV0 = baud;
			// habilitamos la salida fuera del chip de las se�ales RxD0 y TxD0
			rPCONE = (rPCONE & ~(0xF << 2)) | (0x2 << 2) | (0x2 << 4);
			break;

		case UART1:
			rULCON1 = confvalue;
			rUBRDIV1 = baud;
			// habilitamos la salida fuera del chip de las se�ales RxD1 y TxD1
			rPCONC = rPCONC  | (0xF << 24);
			break;

		default:
			return -1;
	}

	uport[port].echo = lconf->echo;

	return 0;
}

/* uart_conf_txmode: funci�n que configura el modo de transmici�n del puerto
 */
int uart_conf_txmode(enum UART port, enum URxTxMode mode)
{
	int conf = 0; //variable para modo POLL/INT o DMA

	if (mode < 0 || mode > 3)
		return -1;

	if (port < 0 || port > 1)
		return -1;

	switch (mode) {
		case POLL:
		case INT:
			conf = 1;
			break;
		case DMA:
			conf = (port == UART0) ? 2 : 3;
			break;
		default:
			conf = 0;
	}
		

	switch (port) {
		case UART0:
			rUCON0 = (1 << 8) || conf << 2;//REVISAR: modo indicado por conf, Tx interrupt por nivel
			break;

		case UART1:
			rUCON1 = (1 << 8) || conf << 2;//REVISAR: modo indicado por conf, Tx interrupt por nivel
			break;
	}

	uport[port].txmode = mode;

	return 0;
}

/* uart_conf_rxmode: funci�n que configura el modo de recepci�n del puerto
 */
int uart_conf_rxmode(enum UART port, enum URxTxMode mode)
{
	int conf = 0; //variable para modo POLL/INT o DMA

	if (mode < 0 || mode > 3)
		return -1;

	if (port < 0 || port > 1)
		return -1;

	switch (mode) {
		case POLL:
		case INT:
			conf = 1;
			break;
		case DMA:
			conf = (port == UART0) ? 2 : 3;
			break;
		default:
			conf = 0;
	}

	switch (port) {
		case UART0:
			rUCON0 = conf;//REVISAR: modo indicado por conf, Rx interrupt por pulso
			//REVISAR: si se el modo es por interrupciones habilitar la linea
			//de interrupci�n por recepci�n en el puerto 0
			ic_enable(INT_URXD0);
			break;

		case UART1:
			rUCON1 = conf;//REVISAR: modo indicado por conf, Rx interrupt por pulso
			//REVISAR: si se el modo es por interrupciones habilitar la linea
			//de interrupci�n por recepci�n en el puerto 1
			ic_enable(INT_URXD1);
			break;
	}

	uport[port].rxmode = mode;

	return 0;
}

/* uart_rx_ready: funci�n que realiza un espera activa hasta que el puerto haya
 * recibido un byte
 */
static void uart_rx_ready(enum UART port)
{
	switch (port) {
		case UART0:
			//REVISAR: esperar a que la uart0 haya recibido un dato (UTRSTAT0,
			//Receive Buffer Data Ready)
			//while(...)
			while(!(rUTRSTAT0 & 0x01)) ;
			break;

		case UART1:
			//REVISAR: esperar a que la uart1 haya recibido un dato (UTRSTAT1,
			//Receive Buffer Data Ready)
			while(!(rUTRSTAT1 & 0x01)) ;
			break;
	}
}

/* uart_tx_ready: funci�n que realiza un espera activa hasta que se vac�e el
 * buffer de transmici�n del puerto
 */
static void uart_tx_ready(enum UART port)
{
	switch (port) {
		case UART0:
			//REVISAR: esperar a que se vacie el buffer de transmici�n de la
			//uart0 (UTRSTAT0, Transmit Buffer Empty)
			while(!(rUTRSTAT0 & 0x02)) ;
			break;

		case UART1:
			//REVISAR: esperar a que se vacie el buffer de transmici�n de la
			//uart1 (UTRSTAT1, Transmit Buffer Empty)
			while(!(rUTRSTAT1 & 0x02)) ;
			break;
	}
}

/* uart_write: funcion que escribe un byte en el buffer de transmici�n del
 * puerto
 */
static void uart_write(enum UART port, char c)
{
	if (port == UART0)
		WrUTXH0(c);//REVISAR: Escribir el car�cter c en el puerto 0, usar la macro WrUTXH0
	else
		WrUTXH1(c);//REVISAR: Escribir el car�cter c en el puerto 1, usar la macro WrUTXH1
}
 
/* uart_read: funci�n que lee un byte del buffer (registro) de recepci�n del
 * puerto, y hace el eco del car�cter si el puerto tiene el eco activado
 */
static char uart_read(enum UART port)
{
	char c;

	if (port == UART0)
		c = RdURXH0();//REVISAR: Leer un byte del puerto 0, usar la macro RdUTXH0
	else
		c = RdUTXH1();//REVISAR: Leer un byte del puerto 1, usar la macro RdUTXH1

	if (uport[port].echo == ON) {
		uart_tx_ready();//REVISAR: Esperar a que el puerto est� listo para transmitir
		uart_write(c);//REVISAR: Escribir el car�cter le�do (c) en el puerto port
	}
	return c;
}

/* uart_readtobuf: funci�n invocada por la ISR de recepci�n. Su misi�n es
 * escribir el car�cter recibido en el buffer de recepci�n del puerto (campo
 * ibuf de la estructura port_stat correspondiente)
 */
static void uart_readtobuf(enum UART port)//write
{
	char c;
	struct port_stat *pst = &uport[port];

	c = uart_read(port);
	pst->ibuf[pst->wP] = c;
	pst->wP = pst->wP++ % BUFLEN;

	/* REVISAR:
	 * 1. Leer un byte del puerto y copiarlo en el buffer de recepci�n del
	 *    puerto en la posici�n indicada por el puntero de escritura.
	 *
	 * 2. Incrementar el puntero de escritura y si es necesario corregir su
	 *    valor para que est� siempre en el rango 0 - BUFLEN-1 (gestionado de
	 *    forma circular)
	 */

}

/* uart_readfrombuf: funci�n invocada por uart_getch en el caso de que el puerto
 * est� configurado por interrupciones para la recepci�n. Su misi�n es esperar a
 * que al menos haya un byte en el buffer de recepci�n, y entonces sacarlo del
 * buffer y devolverlo como byte le�do.
 */
static char uart_readfrombuf(enum UART port)
{
	struct port_stat *pst = &uport[port];

	pst->rP = pst->rP % BUFLEN;

	uart_rx_ready();

	char c = pst->ibuf[pst->rP];

	pst->rP++;

	return c;
	/* REVISAR:
	 * 1. Corregir (de forma circular) el valor del puntero de lectura si está
	 *    fuera del rango 0 - BUFLEN-1.
	 * 2. Esperar a que el buffer de recepci�n contenga alg�n byte.
	 * 3. Extraer el primer byte y devolverlo (el byte se devuelve y el puntero
	 *    de lectura se deja incrementado, con lo que el byte queda fuera del
	 *    buffer)
	 */
}

/* ISR de recepci�n por el puerto 0 */
void Uart0_RxInt(void)
{
	uart_readtobuf(UART0);
	
	//REVISAR: borrar el flag de interrupci�n por recepci�n en el puerto 0
	ic_cleanflag(INT_URXD0);
}

/* ISR de recepci�n por el puerto 1 */
void Uart1_RxInt(void)
{
	uart_readtobuf(UART1);
	
	//REVISAR: borrar el flag de interrupci�n por recepci�n en el puerto 1
	ic_cleanflag(INT_URXD1);
}

/* uart_dotxint: rutina invocada por la ISR de transmici�n. Su misi�n es enviar
 * el siguiente byte del buffer de transmici�n, apuntado por el campo sendP de
 * la estructura port_stat asociada al puerto, y si la transmici�n ha finalizado
 * desactivar las interrupciones de env�o y se�alizar el final del env�o
 * poniendo el puntero sendP a NULL.
 */
static void uart_dotxint(enum UART port)
{
	struct port_stat *pst = &uport[port];

	if (*pst->sendP != '\0' ) {
		if (*pst->sendP == '\n') {
			/* Para que funcione bien con los terminales windows, vamos a hacer
			 * la conversi�n de \n por \r\n, por tanto enviamos un car�cter \r
			 * extra en este caso
			 */
			uart_write(port, '\r');
			uart_tx_ready(port);
			//REVISAR: enviar \r y esperar a que el puerto quede libre para
			//enviar
		}

		uart_write(port, *pst->sendP);
		pst->sendP++;
		//REVISAR: enviar el car�cter apuntado por sendP e incrementar dicho
		//puntero
	}

	if (*pst->sendP == '\0') {
		//REVISAR: si hemos llegado al final de la cadena de caracteres
		// deshabilitamos la linea de interrupci�n por transmici�n del puerto
		// y ponemos el puntero sendP a NULL
		if(port == UART0)
			ic_disable(INT_UTXD0);
		else
			ic_disable(INT_UTXD1);
		pst->sendP = NULL;
	}
}

/* ISR de transmici�n por el puerto 0 */
void Uart0_TxInt(void)
{
	uart_dotxint(UART0);
	
	//REVISAR: borrar el flag de interrupci�n por transmici�n en el puerto 0
	ic_cleanflag(INT_UTXD0);
}

/* ISR de transmici�n por el puerto 1 */
void Uart1_TxInt(void)
{
	uart_dotxint(UART1);
	
	//REVISAR: borrar el flag de interrupci�n por transmici�n en el puerto 1
	ic_cleanflag(INT_UTXD1);
}


/* uart_getch: funci�n bloqueante (sincrona) para la recepci�n de un byte por el
 * puerto serie
 */
int uart_getch(enum UART port, char *c)
{
	if (port < 0 || port > 1)
		return -1;

	switch (uport[port].rxmode) {
		case POLL:
			uart_rx_ready(port);
			*c = uart_read(port);
			// REVISAR: Esperar a que el puerto port haya recibido un byte
			// Leer dicho byte y escribirlo en la direcci�n apuntada por c
			// CLASE: *c=uart_read();
			break;

		case INT:
			*c = uart_readfrombuf(port);
			// REVISAR: Leer el primer byte del buffer de recepci�n del puerto
			// y copiarlo en la direcci�n apuntada por c
			break;

		case DMA:
			//OPCIONAL
			return -1;
			break;

		default:
			return -1;
	}

	return 0;
}

/* uart_sendch: funci�n bloqueante (sincrona) para la transmici�n de un byte por el
 * puerto serie
 */
int uart_sendch(enum UART port, char c)
{
	char localB[2] = {0};

	if (port < 0 || port > 1)
		return -1;

	switch (uport[port].txmode) {
		case POLL:
			uart_tx_ready(port);
			if(c == '\n') {
				uart_write(port, '\r');
				uart_tx_ready(port);
				uart_write(port, '\n');
			} else {
				uart_write(port, c);
			}
			/* REVISAR:
			 * 1. Esperar a que el puerto est� listo para transmitir un byte
			 * 2. Si el byte es \n enviamos primero \r y volvemos a esperar a
			 *    que est� listo para transmitir
			 * 3. Enviamos el car�cter c por el puerto
			 */
			break;

		case INT:
			localB[0] = c;
			uart_send_str(port, localB);
			break;

		case DMA:
			//OPCIONAL
			return -1;
			break;

		default:
			return -1;
	}

	return 0;
}

/* uart_send_str: funci�n bloqueante (sincrona) para la transmici�n de una
 * cadena de caracteres por el puerto serie
 */
int uart_send_str(enum UART port, char *str)
{
	int line;

	if (port < 0 || port > 1)
		return -1;

	switch (uport[port].txmode) {
		case POLL:
			while(*str != '\n') {
				uart_sendch(port, *str);
				str++;
			}
			//REVISAR: usar uart_sendch para enviar todos los bytes de la
			//cadena apuntada por str
			break;

		case INT:
			struct port_stat *pst = uport[port];
			pst->sendP = str;
			if(port == UART0)
				ic_enable(INT_UTXD0);
			else
				ic_enable(INT_UTXD1);

			while(pst->sendP != NULL) ;
			/* REVISAR:
			 * 1. Hacer que el puntero del buffer de env�o (campo sendP en la
			 *    estructura port_stat del puerto) apunte al comienzo de la
			 *    cadena str.
			 * 2. Habilitar las interrupciones por transmici�n en el puerto
			 * 3. Esperar a que se complete el env�o (la ISR pondr� a NULL el
			 *    puntero de env�o sendP)
			 */
			break;

		case DMA:
			//TODO
			return -1;
			break;

		default:
			return -1;
	}

	return 0;

}

/* uart_printf: funci�n bloqueante (sincrona) para la transmici�n de una
 * cadena de caracteres con formato por el puerto serie
 */
void uart_printf(enum UART port, char *fmt, ...)
{
    va_list ap;
    char str[256];

    va_start(ap, fmt);
    vsnprintf(str, 256, fmt, ap);
    uart_send_str(port, str);
    va_end(ap);
}


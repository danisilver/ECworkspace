#include "44b.h"
#include "gpio.h"

/* Port B interface implementation */

int portB_conf(int pin, enum port_mode mode)
{
	int ret = 0;
	if (pin < 0 || pin > 10)
		return -1; // indica error

	if (mode == SIGOUT){
		rPCONB = rPCONB | (0x1 << pin);
			// REVISAR: poner en rPCONB el bit indicado por pin a 1 para que por
		// dicho pin en el puerto B salga la señal correspondiente del
		// controlador de memoria

	} else if (mode == OUTPUT) {
		rPCONB = rPCONB & (~(0x1 << pin));
		// REVISAR: poner en rPCONB el bit indicado por pin a 0 para que dicho
		// pin sea un pin de salida
	} else {
		ret = -1; // indica error
	}

	return ret;
}

int portB_write(int pin, enum digital val)
{
	if (pin < 0 || pin > 10)
		return -1; // indica error

	if (val < 0 || val > 1)
		return -1; // indica error

	if (val){
		rPDATB = rPDATB | (0x1 << pin);
		// REVISAR: poner en rPDATB el bit indicado por pin a 1
	}else{
		rPDATB = rPDATB & (~(0x1 << pin));
		// REVISAR: poner en rPDATB el bit indicado por pin a 0
	}

	return 0;
}

/* Port G interface implementation */

int portG_conf(int pin, enum port_mode mode)
{
	int pos  = pin*2;

	if (pin < 0 || pin > 7)
		return -1; // indica error

	switch (mode) {
		case INPUT:
			rPCONG = rPCONG & ~(0x3<<pos);
			// REVISAR: poner en rPCONG 00 a partir de la posiciÃ³n pos para
			// configurar como pin de entrada el pin indicado por el parÃ¡metro pin
			break;
		case OUTPUT:
			rPCONG = rPCONG & ~(0x2<<pos);
			rPCONG = rPCONG | (0x1<<pos);

			// REVISAR: poner en rPCONG 01 a partir de la posiciÃ³n pos para
			// configurar como pin de salida el pin indicado por el parÃ¡metro pin
			break;
		case SIGOUT:
			rPCONG = rPCONG & ~(0x3<<pos);
			rPCONG = rPCONG | (0x2<<pos);
			// REVISAR: poner en rPCONG 10 a partir de la posiciÃ³n pos para
			// que salga la seÃ±al interna correspondiente por el pin indicado
			// por el parÃ¡metro pin
			break;
		case EINT:
			rPCONG = rPCONG | (0x3<<pos);
			// REVISAR: poner en rPCONG 11 a partir de la posiciÃ³n pos para
			// habilitar la generaciÃ³n de interrupciones externas por el pin
			// indicado por el parÃ¡metro pin
			break;
		default:
			return -1;
	}

	return 0;
}

int portG_eint_trig(int pin, enum trigger trig)
{
	int pos = pin*4;

	if (pin < 0 || pin > 7)
		return -1;

switch (trig) {
		case LLOW:
			rEXTINT = rEXTINT & (~(0x7<<pos));
			// REVISAR: poner en rEXTINT a partir de la posición pos tres bits
			// a 000, para configurar interrupciones externas por nivel bajo
			break;
		case LHIGH:
			rEXTINT = rEXTINT & (~(0x7<<pos));
			rEXTINT = rEXTINT| (0x1<<pos);
			// REVISAR: poner en rEXTINT a partir de la posición pos tres bits
			// a 001, para configurar interrupciones externas por nivel alto
			break;
		case FALLING:
			rEXTINT = rEXTINT & (~(0x7<<pos));
			rEXTINT = rEXTINT| (0x2<<pos);
			// REVISAR: poner en rEXTINT a partir de la posición pos tres bits
			// a 010, para configurar interrupciones externas por flanco de
			// bajada
			break;
		case RISING:
			rEXTINT = rEXTINT & (~(0x7<<pos));
			rEXTINT = rEXTINT| (0x4<<pos);
			// REVISAR: poner en rEXTINT a partir de la posición pos tres bits
			// a 100, para configurar interrupciones externas por flanco de
			// subida
			break;
		case EDGE:
			rEXTINT = rEXTINT & (~(0x7<<pos));
			rEXTINT = rEXTINT| (0x6<<pos);
			// REVISAR: poner en rEXTINT a partir de la posición pos tres bits
			// a 110, para configurar interrupciones externas por cualquier
			// flanco
			break;
		default:
			return -1;
	}
	return 0;
}

int portG_write(int pin, enum digital val)
{
	int pos = pin*2;

	if (pin < 0 || pin > 7)
		return -1; // indica error

	if (val < 0 || val > 1)
		return -1; // indica error

	if ((rPCONG & (0x3 << pos)) != (0x1 << pos))
		return -1; // indica error

	if (val){
		rPDATG = rPDATG | (0x1 << pin);
		// REVISAR: poner en rPDATG el bit indicado por pin a 1
	}else{
		rPDATG = rPDATG & (~(0x1 << pin));
		// REVISAR: poner en rPDATG el bit indicado por pin a 0
	}
	return 0;
}

int portG_read(int pin, enum digital* val)
{
	int pos = pin*2;

	if (pin < 0 || pin > 7)
		return -1; // indica error

	if (rPCONG & (0x3 << pos))
		return -1; // indica error

	/*REVISAR: true si esta a 1 en rPDATG el pin indicado por el parÃ¡metro pin*/
	if (rPDATG & (0x1 << pin))
		*val = HIGH;
	else
		*val = LOW;

	return 0;
}

int portG_conf_pup(int pin, enum enable st)
{
	if (pin < 0 || pin > 7)
		return -1; // indica error

	if (st != ENABLE && st != DISABLE)
		return -1; // indica error

	if (st == ENABLE){
		// REVISAR: poner el pin de rPUPG indicado por el parametro pin al valor adecuado,
		// para activar la resistencia de pull-up
		rPUPG = rPUPG & (~(0x1 << pin));
	}else{
		rPUPG = rPUPG | (0x1 << pin);
		// REVISAR: poner el pin de rPUPG indicado por el parametro pin al valor adecuado,
		// para desactivar la resistencia de pull-up
	}
	return 0;
}


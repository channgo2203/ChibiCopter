/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS receiver over serial
*/

#include "ch.h"
#include "hal.h"

#include "GPS.h"
#include "TinyGPS.h"

#include "Comms.h"

EventSource gps_event;

// Setup some timers and callbacks
static VirtualTimer vt2;
static void led2off(void *p) {

  (void)p;
  palClearPad(GPIOD, GPIOD_LED5); // red
}

/*
 * GPS setup
 */
static const SerialConfig sd1cfg = {
	57600,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

static WORKING_AREA(GPSWA, 128);
static msg_t GPS(void *arg){

	(void)arg;
	chRegSetThreadName("GPS");
	while (TRUE){
		// Read a byte off the GPS
		uint8_t c = chnGetTimeout((BaseChannel *)&SD1, TIME_INFINITE);
		if (TinyGPS_encode(c)){
      		palSetPad(GPIOD, GPIOD_LED5); // red

			chEvtBroadcast(&gps_event);

			// Set a timer to turn off the red led
			/*chSysLock();
			if (chVTIsArmedI(&vt2))
				chVTResetI(&vt2);
			chVTSetI(&vt2, MS2ST(500), led2off, NULL);
			chSysUnlock();
			*/
		}
	}
	return 0;
}

/*
 * Activates the serial driver 1 using the driver default configuration, but at 57600
 * PA9(TX) and PA10(RX) are routed to USART1.
 */

void GPSInit(void){

	sdStart(&SD1, &sd1cfg);
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // not currently connected
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // incoming data from the GPS

	chEvtInit(&gps_event);

	chThdCreateStatic(GPSWA, sizeof(GPSWA), NORMALPRIO, GPS, NULL);
}
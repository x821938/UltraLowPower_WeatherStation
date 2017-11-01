#include "Power.h"
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>    
#include <avr/wdt.h>



/* Watchdog interrupt vector */
ISR( WDT_vect ) { 
	wdt_disable();  // disable watchdog
}  



/* Makes a deep sleep for X second using as little power as possible */
void gotoDeepSleep( uint16_t seconds ) {
	pinMode( 0, INPUT );
	pinMode( 2, INPUT );

	while ( seconds > 0 ) {
		set_sleep_mode( SLEEP_MODE_PWR_DOWN );
		ADCSRA = 0;            // turn off ADC
		power_all_disable();  // power off ADC, Timer 0 and 1, serial interface
		noInterrupts();       // timed sequence coming up
		resetWatchdog();      // get watchdog ready
		sleep_enable();       // ready to sleep
		interrupts();         // interrupts are required now
		sleep_cpu();          // sleep                
		sleep_disable();      // precaution
		seconds--;
	}
	power_all_enable();   // power everything back on

}



/* Prepare watchdog */
void resetWatchdog() {
	MCUSR = 0; // clear various "reset" flags
	WDTCR = bit( WDCE ) | bit( WDE ) | bit( WDIF ); // allow changes, disable reset, clear existing interrupt
	// set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
	WDTCR = bit( WDIE ) | bit( WDP2 ) | bit( WDP1 );    // set WDIE, and 1 seconds delay
	//WDTCR = bit( WDIE ) | bit( WDP3 ) | bit( WDP0 );    // set WDIE, and 8 seconds delay
														
	wdt_reset(); // pat the dog
} 



/* Remove Wakes up the ESP by pulling it's reset pin low for a short time  */
void wakeESP() {
	pinMode( ESP_RESET_PIN, OUTPUT );
	digitalWrite( ESP_RESET_PIN, LOW );
	delay( 10 );
	digitalWrite( ESP_RESET_PIN, HIGH );
	pinMode( ESP_RESET_PIN, INPUT_PULLUP );
}



/* Can turn on/off power to sensors. Might save battery if sensor doesn't have good power down mode  */
void sensorPower(bool powerOn) {
	if ( powerOn ) {
		pinMode( SENSOR_POWER_PIN, OUTPUT );
		digitalWrite( SENSOR_POWER_PIN, HIGH );
		delay( 5 ); // Make sure sensors are powered up before asking them
	} else {
		digitalWrite( SENSOR_POWER_PIN, LOW );
		pinMode( SENSOR_POWER_PIN, INPUT );
	}
}
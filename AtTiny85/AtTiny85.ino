#include "Setup.h"
#include <USIWire.h>
#include "Sensor.h"
#include "Power.h"
#include "SlaveI2C.h"
#include "Storage.h"


// Format for a measurement
struct SensorData {
	uint16_t lux;		  // We don't need float precision. Saving storage
	uint16_t humidity;	  // raw data. real humidity = ( 125.0*RH_Code / 65536 ) - 6;
	uint16_t temperature; // raw data. real temperature = ( 175.72*raw / 65536 ) - 46.85;
};



// Global objects
Storage storage( sizeof( SensorData ) );
SlaveI2C slaveI2C;
SensorTsl sensorTsl;
SensorSi sensorSi;



void setup() {
	pinMode( ESP_RESET_PIN, INPUT_PULLUP );
	resetWatchdog(); // Needed for deep sleep to succeed
}



void loop() {

	static enum State { // Our state machine
		SLEEP,
		MEASURING,
		MASTER_WAKE,
		SENDING
	} state = SLEEP;

	static uint16_t secondsSleeping = 0;
	static unsigned long masterWokenUpAt;

	switch ( state ) {
		case SLEEP:
			sensorPower( false );	// Save power by turning off power to sensors
			slaveI2C.end();			// We don't want to be i2c slave anymore. 
			gotoDeepSleep( MEASUREMENT_EVERY );		// Deep sleep for X seconds
			secondsSleeping += MEASUREMENT_EVERY;	// Keep a track of how many seconds we have been sleeping
			state = MEASURING;
			break;
		case MEASURING:
			SensorData sensorData;
			
			sensorPower( true );	// Power up i2c devices and read data from them
			sensorData.lux = sensorTsl.readLux();
			sensorData.temperature = sensorSi.readTemp();
			sensorData.humidity = sensorSi.readHumidity();
			sensorPower( false );	// Turn off i2c device again
			storage.addElement( &sensorData );

			state = SLEEP;			// Sucessfully stored the data, starting sleeping again
			if ( secondsSleeping >= WAKE_MASTER_EVERY ) state = MASTER_WAKE; // Unless it's wakeup time
			break;
		case MASTER_WAKE:
			sensorPower( true );	// Sensors needs to have power otherwise they drag down the I2C
			slaveI2C.begin();		// Now we are slave for the ESP
			wakeESP();
			masterWokenUpAt = millis();
			state = SENDING;
			break;
		case SENDING:
			if ( slaveI2C.masterGoingToSleep() ) {
				if ( slaveI2C.masterGotOurData() ) storage.clear();		// If Master has confirmed our data. We can start with new measurements
				secondsSleeping = 0;
				state = SLEEP;
			}
			if ( millis() - masterWokenUpAt > GIVEUP_ON_MASTER_AFTER * 1000UL ) {
				secondsSleeping = 0;
				state = SLEEP;
			}
			break;
	}
}
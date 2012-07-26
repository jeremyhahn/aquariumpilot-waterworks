/**
 * Responsible for managing application configuration in EEPROM
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Configuration.h"

Configuration::Configuration() {

	this->_autoWaterChanges = EEPROM.read(12);
	if(this->_autoWaterChanges == 0 || this->_autoWaterChanges == 255) {
		this->_autoWaterChanges = true;
		EEPROM.write(12, this->_autoWaterChanges);
	}

	this->_autoFillReservoir = EEPROM.read(13);
	if(this->_autoFillReservoir == 0 || this->_autoFillReservoir == 255) {
		this->_autoFillReservoir = false;
		EEPROM.write(13, this->_autoFillReservoir);
	}

	this->_autoCirculation = EEPROM.read(14);
	if(this->_autoCirculation == 0 || this->_autoCirculation == 255) {
		this->_autoCirculation = true;
		EEPROM.write(14, this->_autoCirculation);
	}

	this->_reservoirPowerheadOutlet = EEPROM.read(15);
	if(this->_reservoirPowerheadOutlet == 0 || this->_reservoirPowerheadOutlet == 255) {
		this->_reservoirPowerheadOutlet = 28;
		EEPROM.write(15, this->_reservoirPowerheadOutlet);
	}

	this->_aquariumFillPumpOutlet = EEPROM.read(16);
	if(this->_aquariumFillPumpOutlet == 0 || this->_aquariumFillPumpOutlet == 255) {
		this->_aquariumFillPumpOutlet = 29;
		EEPROM.write(16, this->_aquariumFillPumpOutlet);
	}

	//this->_drainMillisPerGallon = EEPROM.read(17);
	if(this->_drainMillisPerGallon == 0 || this->_drainMillisPerGallon == 255) {
		this->_drainMillisPerGallon = 8100000; // 8100000
		//EEPROM.write(17, this->_drainMillisPerGallon);
	}

	//this->_fillMillisPerGallon = EEPROM.read(18);
	if(this->_fillMillisPerGallon == 0 || this->_fillMillisPerGallon == 255) {
		this->_fillMillisPerGallon = 1500000;  // 1500000
		//EEPROM.write(18, this->_fillMillisPerGallon);
	}

	//this->_waterChangeMillis = EEPROM.read(19);
	if(this->_waterChangeMillis == 0 || this->_waterChangeMillis == 255) {
		this->_waterChangeMillis = 86400000;   // 86400000
		//EEPROM.write(19, this->_waterChangeMillis);
	}
}

bool Configuration::isAutoWaterChangesEnabled() {
	return this->_autoWaterChanges;
}

bool Configuration::isAutoFillReservoirEnabled() {
	return this->_autoFillReservoir;
}

bool Configuration::isAutoCirculationEnabled() {
	return this->_autoCirculation;
}

int Configuration::getReservoirPowerheadOutlet() {
	return this->_reservoirPowerheadOutlet;
}

int Configuration::getAquariumFillPumpOutlet() {
	return this->_aquariumFillPumpOutlet;
}

long Configuration::getDrainMillisPerGallon() {
	return this->_drainMillisPerGallon;
}

long Configuration::getFillMillisPerGallon() {
	return this->_fillMillisPerGallon;
}

long Configuration::getWaterChangeMillis() {
	return this->_waterChangeMillis;
}

void Configuration::erase() {

	EEPROM.write(4, 255);
	EEPROM.write(5, 255);
	EEPROM.write(6, 255);
	EEPROM.write(7, 255);
	EEPROM.write(8, 255);
	EEPROM.write(9, 255);
	EEPROM.write(10, 255);
	EEPROM.write(11, 255);
}

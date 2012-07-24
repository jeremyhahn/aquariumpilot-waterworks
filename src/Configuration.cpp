/**
 * Responsible for managing application configuration in EEPROM
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Configuration.h"

Configuration::Configuration() {

	this->_dailyWaterChanges = EEPROM.read(4);
	if(this->_dailyWaterChanges == 0 || this->_dailyWaterChanges == 255) {
		this->_dailyWaterChanges = false;
		EEPROM.write(4, this->_dailyWaterChanges);
	}

	this->_autoFillReservoir = EEPROM.read(5);
	if(this->_autoFillReservoir == 0 || this->_autoFillReservoir == 255) {
		this->_autoFillReservoir = false;
		EEPROM.write(5, this->_autoFillReservoir);
	}

	this->_autoCirculation = EEPROM.read(6);
	if(this->_autoCirculation == 0 || this->_autoCirculation == 255) {
		this->_autoCirculation = false;
		EEPROM.write(6, this->_autoCirculation);
	}

	this->_reservoirPowerheadOutlet = EEPROM.read(7);
	if(this->_reservoirPowerheadOutlet == 0 || this->_reservoirPowerheadOutlet == 255) {
		this->_reservoirPowerheadOutlet = 28;
		EEPROM.write(7, this->_reservoirPowerheadOutlet);
	}

	this->_aquariumFillPumpOutlet = EEPROM.read(8);
	if(this->_aquariumFillPumpOutlet == 0 || this->_aquariumFillPumpOutlet == 255) {
		this->_aquariumFillPumpOutlet = 29;
		EEPROM.write(8, this->_aquariumFillPumpOutlet);
	}

	//this->_drainMillisPerGallon = EEPROM.read(9);
	if(this->_drainMillisPerGallon == 0 || this->_drainMillisPerGallon == 255) {
		this->_drainMillisPerGallon = 8100000;
		//EEPROM.write(9, this->_drainMillisPerGallon);
	}

	//this->_fillMillisPerGallon = EEPROM.read(10);
	if(this->_fillMillisPerGallon == 0 || this->_fillMillisPerGallon == 255) {
		this->_fillMillisPerGallon = 1500000;
		//EEPROM.write(10, this->_fillMillisPerGallon);
	}
}

bool Configuration::isDailyWaterChangesEnabled() {
	return this->_dailyWaterChanges;
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

void Configuration::erase() {

	EEPROM.write(4, 255);
	EEPROM.write(5, 255);
	EEPROM.write(6, 255);
	EEPROM.write(7, 255);
	EEPROM.write(8, 255);
	EEPROM.write(9, 255);
	EEPROM.write(10, 255);
}

/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Configuration.h"

Configuration::Configuration() {

	this->load();
}

void Configuration::load() {

	this->_autoWaterChanges = this->read(EEPROM_AUTOWATERCHANGES, DEFAULT_VALUE_AUTOWATERCHANGES);
	this->_autoFillReservoir = this->read(EEPROM_AUTOFILLRESERVOIR, DEFAULT_VALUE_AUTOFILLRESERVOIR);
	this->_autoFillReservoirHour = this->read(EEPROM_AUTOFILLRESERVOIRHOUR, DEFAULT_VALUE_AUTOFILLRESERVOIRHOUR);
	this->_autoFillReservoirMinutes = this->read(EEPROM_AUTOFILLRESERVOIRMINUTES, DEFAULT_VALUE_AUTOFILLRESERVOIRMINUTES);
	this->_autoCirculation = this->read(EEPROM_AUTOCIRCULATION, DEFAULT_VALUE_AUTOCIRCULATION);
	this->_reservoirPowerheadOutlet = this->read(EEPROM_RESERVOIRPOWERHEADOUTLET, DEFAULT_VALUE_RESERVOIRPOWERHEADOUTLET);
	this->_aquariumFillPumpOutlet = this->read(EEPROM_AQUARIUMFILLPUMPOUTLET, DEFAULT_VALUE_AQUARIUMFILLPUMPOUTLET);
	this->_drainMinutesPerGallon = this->read(EEPROM_DRAINMINUTESPERGALLON, DEFAULT_VALUE_DRAINMINUTESPERGALLON);
	this->_drainSecondsPerGallon = this->read(EEPROM_DRAINSECONDSPERGALLON, DEFAULT_VALUE_DRAINSECONDSPERGALLON);
	this->_fillMinutesPerGallon = this->read(EEPROM_FILLMINUTESPERGALLON, DEFAULT_VALUE_FILLMINUTESPERGALLON);
	this->_fillSecondsPerGallon = this->read(EEPROM_FILLSECONDSPERGALLON, DEFAULT_VALUE_FILLSECONDSPERGALLON);
	this->_autoWaterChangeHour = this->read(EEPROM_AUTOWATERCHANGEHOUR, DEFAULT_VALUE_AUTOWATERCHANGEHOUR);
	this->_autoWaterChangeMinutes = this->read(EEPROM_AUTOWATERCHANGEMINUTES, DEFAULT_VALUE_AUTOWATERCHANGEMINUTES);
	this->_autoWaterChangeGallons = this->read(EEPROM_AUTOWATERCHANGEGALLONS, DEFAULT_VALUE_AUTOWATERCHANGEGALLONS);
	this->_autoTopOff = this->read(EEPROM_AUTOTOPOFF, DEFAULT_VALUE_AUTOTOPOFF);
	this->_autoTopOffMinutes = this->read(EEPROM_AUTOTOPOFFMINUTES, DEFAULT_VALUE_AUTOTOPOFFMINUTES);

	// Pinouts
	this->_pin_outlet1 = this->read(EEPROM_OUTLET1, DEFAULT_PINOUT_OUTLET1);
	this->_pin_outlet2 = this->read(EEPROM_OUTLET2, DEFAULT_PINOUT_OUTLET2);
	this->_pin_outlet3 = this->read(EEPROM_OUTLET3, DEFAULT_PINOUT_OUTLET3);
	this->_pin_outlet4 = this->read(EEPROM_OUTLET4, DEFAULT_PINOUT_OUTLET4);
	this->_pin_outlet5 = this->read(EEPROM_OUTLET5, DEFAULT_PINOUT_OUTLET5);
	this->_pin_outlet6 = this->read(EEPROM_OUTLET6, DEFAULT_PINOUT_OUTLET6);
	this->_pin_outlet7 = this->read(EEPROM_OUTLET7, DEFAULT_PINOUT_OUTLET7);
	this->_pin_outlet8 = this->read(EEPROM_OUTLET8, DEFAULT_PINOUT_OUTLET8);
	this->_pin_rodiAquariumSolenoid = this->read(EEPROM_RODIAQUARIUMSOLENOID, DEFAULT_PINOUT_RODIAQUARIUMSOLENOID);
	this->_pin_rodiReservoirSolenoid = this->read(EEPROM_RODIRESERVOIRSOLENOID, DEFAULT_PINOUT_RODIRESERVOIRSOLENOID);
	this->_pin_aquariumDrainSolenoid = this->read(EEPROM_AQUARIUMDRAINSOLENOID, DEFAULT_PINOUT_AQUARIUMDRAINSOLENOID);
	this->_pin_upperFloatValve = this->read(EEPROM_RESERVOIRUPPERFLOATVALVE, DEFAULT_PINOUT_RESERVOIRUPPERFLOATVALVE);
	this->_pin_lowerFloatValve = this->read(EEPROM_RESERVOIRLOWERFLOATVALVE, DEFAULT_PINOUT_RESERVOIRLOWERFLOATVALVE);
	this->_onewire = this->read(EEPROM_ONEWIRE, DEFAULT_VALUE_ONEWIRE);
}

int Configuration::read(int address, uint8_t defaultValue) {

	int val = EEPROM.read(address);
	if(val == 255) {
	   EEPROM.write(address, defaultValue);
	   return defaultValue;
	}
	return val;
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

int Configuration::getDrainMinutesPerGallon() {
	return this->_drainMinutesPerGallon;
}

int Configuration::getDrainSecondsPerGallon() {
	return this->_drainSecondsPerGallon;
}

int Configuration::getFillMinutesPerGallon() {
	return this->_fillMinutesPerGallon;
}

int Configuration::getFillSecondsPerGallon() {
	return this->_fillSecondsPerGallon;
}

int Configuration::getAutoWaterChangeHour() {
	return this->_autoWaterChangeHour;
}

int Configuration::getAutoWaterChangeMinutes() {
	return this->_autoWaterChangeMinutes;
}

int Configuration::getAutoWaterChangeGallons() {
	return this->_autoWaterChangeGallons;
}

int Configuration::getAutoFillReservoirHour() {
	return this->_autoFillReservoirHour;
}

int Configuration::getAutoFillReservoirMinutes() {
	return this->_autoFillReservoirHour;
}

bool Configuration::isAutoTopOffEnabled() {
	return this->_autoTopOff;
}

int Configuration::getAutoTopOffMinutes() {
	return this->_autoTopOffMinutes;
}

void Configuration::setPinOutlet1(int pin) {
	this->_pin_outlet1 = pin;
	EEPROM.write(EEPROM_OUTLET1, pin);
}

int Configuration::getPinOutlet1() {
	return this->_pin_outlet1;
}

void Configuration::setPinOutlet2(int pin) {
	this->_pin_outlet2 = pin;
	EEPROM.write(EEPROM_OUTLET2, pin);
}

int Configuration::getPinOutlet2() {
	return this->_pin_outlet2;
}

void Configuration::setPinOutlet3(int pin) {
	this->_pin_outlet3 = pin;
	EEPROM.write(EEPROM_OUTLET3, pin);
}

int Configuration::getPinOutlet3() {
	return this->_pin_outlet3;
}

void Configuration::setPinOutlet4(int pin) {
	this->_pin_outlet4 = pin;
	EEPROM.write(EEPROM_OUTLET4, pin);
}

int Configuration::getPinOutlet4() {
	return this->_pin_outlet4;
}

void Configuration::setPinOutlet5(int pin) {
	this->_pin_outlet5 = pin;
	EEPROM.write(EEPROM_OUTLET5, pin);
}

int Configuration::getPinOutlet5() {
	return this->_pin_outlet5;
}

void Configuration::setPinOutlet6(int pin) {
	this->_pin_outlet6 = pin;
	EEPROM.write(EEPROM_OUTLET6, pin);
}

int Configuration::getPinOutlet6() {
	return this->_pin_outlet6;
}

void Configuration::setPinOutlet7(int pin) {
	this->_pin_outlet7 = pin;
	EEPROM.write(EEPROM_OUTLET7, pin);
}

int Configuration::getPinOutlet7() {
	return this->_pin_outlet7;
}

void Configuration::setPinOutlet8(int pin) {
	this->_pin_outlet8 = pin;
	EEPROM.write(EEPROM_OUTLET8, pin);
}

int Configuration::getPinOutlet8() {
	return this->_pin_outlet8;
}

void Configuration::setPinRodiAquariumSolenoid(int pin) {
	this->_pin_rodiAquariumSolenoid = pin;
	EEPROM.write(EEPROM_RODIAQUARIUMSOLENOID, pin);
}

int Configuration::getPinRodiAquariumSolenoid() {
	return this->_pin_rodiAquariumSolenoid;
}

void Configuration::setPinRodiReservoirSolenoid(int pin) {
	this->_pin_rodiReservoirSolenoid = pin;
	EEPROM.write(EEPROM_RODIRESERVOIRSOLENOID, pin);
}

int Configuration::getPinRodiReservoirSolenoid() {
	return this->_pin_rodiReservoirSolenoid;
}

void Configuration::setPinAquariumDrainSolenoid(int pin) {
	this->_pin_aquariumDrainSolenoid = pin;
	EEPROM.write(EEPROM_AQUARIUMDRAINSOLENOID, pin);
}

int Configuration::getPinAquariumDrainSolenoid() {
	return this->_pin_aquariumDrainSolenoid;
}

void Configuration::setPinReservoirUpperFloatValve(int pin) {
	this->_pin_upperFloatValve = pin;
	EEPROM.write(EEPROM_RESERVOIRUPPERFLOATVALVE, pin);
}

int Configuration::getPinReservoirUpperFloatValve() {
	return this->_pin_upperFloatValve;
}

void Configuration::setPinReservoirLowerFloatValve(int pin) {
	this->_pin_lowerFloatValve = pin;
	EEPROM.write(EEPROM_RESERVOIRLOWERFLOATVALVE, pin);
}

int Configuration::getPinReservoirLowerFloatValve() {
	return this->_pin_lowerFloatValve;
}

void Configuration::setOneWire(int pin) {
	this->_onewire = pin;
}

int Configuration::getOneWire() {
	return this->_onewire;
}

void Configuration::erase() {
	for(int i=EEPROM_START_ADDRESS; i<(EEPROM_END_ADDRESS+1); i++) {
		EEPROM.write(i, 255);
	}
}

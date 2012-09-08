/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Arduino.h"
#include "Netmask.h"

/**
 * Constructor - Initialize the netmask from EEPROM
 * if the first byte is not 0 or 255. Otherwise,
 * default the netmask to 255.255.255.0.
 *
 * @return void
 */
Netmask::Netmask() {

	this->_netmask_octet1 = EEPROM.read(4);

	if(this->_netmask_octet1 == 255 || this->_netmask_octet1 == 0 ) {

		this->_netmask_octet1 = 255;
		this->_netmask_octet2 = 255;
		this->_netmask_octet3 = 255;
		this->_netmask_octet4 = 0;
	}
	else {

		this->_netmask_octet2 = EEPROM.read(5);
		this->_netmask_octet3 = EEPROM.read(6);
		this->_netmask_octet4 = EEPROM.read(7);
	}

	this->_netmask[0] = _netmask_octet1;
	this->_netmask[1] = _netmask_octet2;
	this->_netmask[2] = _netmask_octet3;
	this->_netmask[3] = _netmask_octet4;

	this->_netmask_string = String(this->_netmask_octet1);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet2);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet3);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet4);
}

/**
 * Destructor - empty
 */
Netmask::~Netmask() {
}

/**
 * Sets the specified netmask and stores it in EEPROM
 *
 * @param byte octet1 The first octet of the netmask
 * @param byte octet2 The second octet of the netmask
 * @param byte octet3 The third octet of the netmask
 * @param byte octet4 The fourth octet of the netmask
 * @return void
 */
void Netmask::set(byte octet1, byte octet2, byte octet3, byte octet4) {

	this->_netmask_octet1 = octet1;
	this->_netmask_octet2 = octet2;
	this->_netmask_octet3 = octet3;
	this->_netmask_octet4 = octet4;

	this->_netmask[0] = this->_netmask_octet1;
	this->_netmask[1] = this->_netmask_octet2;
	this->_netmask[2] = this->_netmask_octet3;
	this->_netmask[3] = this->_netmask_octet4;

	this->_netmask_string = String(this->_netmask_octet1);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet2);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet3);
	this->_netmask_string.concat(".");
	this->_netmask_string.concat(this->_netmask_octet4);

	EEPROM.write(4, octet1);
	EEPROM.write(5, octet2);
	EEPROM.write(6, octet3);
	EEPROM.write(7, octet4);
}

/**
 * Returns a pointer to the byte array containing the IP address
 *
 * @return The pointer to the byte array that stores the IP address
 */
byte* Netmask::getBytes() {

	this->_netmask[0] = this->_netmask_octet1;
	this->_netmask[1] = this->_netmask_octet2;
	this->_netmask[2] = this->_netmask_octet3;
	this->_netmask[3] = this->_netmask_octet4;
	return (&this->_netmask[0]);
}

/**
 * Erases the netmask from EEPROM
 *
 * @return void
 */
void Netmask::erase() {

	EEPROM.write(4, 255);
	EEPROM.write(5, 255);
	EEPROM.write(6, 255);
	EEPROM.write(7, 255);
}

/**
 * Returns a string that contains the formatted netmask address.
 *
 * @return The netmask
 */
String Netmask::toString() {

	return this->_netmask_string;
}

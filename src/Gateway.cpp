/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Arduino.h"
#include "Gateway.h"

/**
 * Constructor - Initialize the gateway from EEPROM
 * if the first byte is not 0 or 255. Otherwise,
 * default the gateway to 192.168.11.1.
 *
 * @return void
 */
Gateway::Gateway() {

	this->_gateway_octet1 = EEPROM.read(8);

	if(this->_gateway_octet1 == 255 || this->_gateway_octet1 == 0 ) {

		this->_gateway_octet1 = 192;
		this->_gateway_octet2 = 168;
		this->_gateway_octet3 = 11;
		this->_gateway_octet4 = 1;
	}
	else {

		this->_gateway_octet2 = EEPROM.read(9);
		this->_gateway_octet3 = EEPROM.read(10);
		this->_gateway_octet4 = EEPROM.read(11);
	}

	this->_gateway[0] = _gateway_octet1;
	this->_gateway[1] = _gateway_octet2;
	this->_gateway[2] = _gateway_octet3;
	this->_gateway[3] = _gateway_octet4;

	this->_gateway_string = String(this->_gateway_octet1);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet2);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet3);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet4);
}

/**
 * Destructor - empty
 */
Gateway::~Gateway() {
}

/**
 * Sets the specified gateway and stores it in EEPROM
 *
 * @param byte octet1 The first octet of the gateway
 * @param byte octet2 The second octet of the gateway
 * @param byte octet3 The third octet of the gateway
 * @param byte octet4 The fourth octet of the gateway
 * @return void
 */
void Gateway::set(byte octet1, byte octet2, byte octet3, byte octet4) {

	this->_gateway_octet1 = octet1;
	this->_gateway_octet2 = octet2;
	this->_gateway_octet3 = octet3;
	this->_gateway_octet4 = octet4;

	this->_gateway[0] = this->_gateway_octet1;
	this->_gateway[1] = this->_gateway_octet2;
	this->_gateway[2] = this->_gateway_octet3;
	this->_gateway[3] = this->_gateway_octet4;

	this->_gateway_string = String(this->_gateway_octet1);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet2);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet3);
	this->_gateway_string.concat(".");
	this->_gateway_string.concat(this->_gateway_octet4);

	EEPROM.write(8, octet1);
	EEPROM.write(9, octet2);
	EEPROM.write(10, octet3);
	EEPROM.write(11, octet4);
}

/**
 * Returns a pointer to the byte array containing the IP address
 *
 * @return The pointer to the byte array that stores the IP address
 */
byte* Gateway::getBytes() {

	this->_gateway[0] = this->_gateway_octet1;
	this->_gateway[1] = this->_gateway_octet2;
	this->_gateway[2] = this->_gateway_octet3;
	this->_gateway[3] = this->_gateway_octet4;
	return (&this->_gateway[0]);
}

/**
 * Erases the gateway from EEPROM
 *
 * @return void
 */
void Gateway::erase() {

	EEPROM.write(8, 255);
	EEPROM.write(9, 255);
	EEPROM.write(10, 255);
	EEPROM.write(11, 255);
}

/**
 * Returns a string that contains the formatted gateway address.
 *
 * @return The gateway
 */
String Gateway::toString() {

	return this->_gateway_string;
}

/**
 * Responsible for managing the IP address of the Arduino
 * in EEPROM.
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Arduino.h"
#include "IpAddress.h"

/**
 * Constructor - Initialize the IP address from EEPROM
 * if the first byte is not 0 or 255. Otherwise,
 * default the IP address to 192.168.11.241.
 *
 * @return void
 * @see http://arduino.cc/it/Tutorial/EEPROMRead
 */
IpAddress::IpAddress() {

	this->_octet1 = EEPROM.read(0);

	if(this->_octet1 == 255 || this->_octet1 == 0 ) {

		this->_octet1 = 192;
		this->_octet2 = 168;
		this->_octet3 = 11;
		this->_octet4 = 241;
	}
	else {

		this->_octet2 = EEPROM.read(1);
		this->_octet3 = EEPROM.read(2);
		this->_octet4 = EEPROM.read(3);
	}

	this->_ip[0] = _octet1;
	this->_ip[1] = _octet2;
	this->_ip[2] = _octet3;
	this->_ip[3] = _octet4;
}

/**
 * Destructor - empty
 */
IpAddress::~IpAddress() {
}

/**
 * Sets the specified IP address and stores it in EEPROM
 *
 * @param byte octet1 The first octet of the IP address
 * @param byte octet2 The second octet of the IP address
 * @param byte octet3 The third octet of the IP address
 * @param byte octet4 The fourth octet of the IP address
 * @return void
 * @see http://arduino.cc/it/Tutorial/EEPROMWrite
 */
void IpAddress::set(byte octet1, byte octet2, byte octet3, byte octet4) {

	this->_octet1 = octet1;
	this->_octet2 = octet2;
	this->_octet3 = octet3;
	this->_octet4 = octet4;

	this->_ip[0] = this->_octet1;
	this->_ip[1] = this->_octet2;
	this->_ip[2] = this->_octet3;
	this->_ip[3] = this->_octet4;

	EEPROM.write(0, octet1);
	EEPROM.write(1, octet2);
	EEPROM.write(2, octet3);
	EEPROM.write(3, octet4);
}

/**
 * Returns a pointer to the byte array containing the IP address
 *
 * @return The pointer to the byte array that stores the IP address
 */
byte* IpAddress::getBytes() {

	this->_ip[0] = this->_octet1;
	this->_ip[1] = this->_octet2;
	this->_ip[2] = this->_octet3;
	this->_ip[3] = this->_octet4;
	return (&this->_ip[0]);
}

/**
 * Erases the address from EEPROM
 *
 * @return void
 */
void IpAddress::erase() {

	EEPROM.write(0, 255);
	EEPROM.write(1, 255);
	EEPROM.write(2, 255);
	EEPROM.write(3, 255);
}

/**
 * Returns a string that contains the formatted IP address.
 *
 * @return The IP address
 */
String IpAddress::toString() {

	String s = String(this->_octet1);
	s.concat(".");
	s.concat(this->_octet2);
	s.concat(".");
	s.concat(this->_octet3);
	s.concat(".");
	s.concat(this->_octet4);

	return s;
}

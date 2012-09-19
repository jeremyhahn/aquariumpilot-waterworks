/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "Arduino.h"
#include "EEPROM.h"

const int DEFAULT_PINOUT_OUTLET1 = 28;
const int DEFAULT_PINOUT_OUTLET2 = 29;
const int DEFAULT_PINOUT_OUTLET3 = 30;
const int DEFAULT_PINOUT_OUTLET4 = 31;
const int DEFAULT_PINOUT_OUTLET5 = 32;
const int DEFAULT_PINOUT_OUTLET6 = 33;
const int DEFAULT_PINOUT_OUTLET7 = 34;
const int DEFAULT_PINOUT_OUTLET8 = 35;
const int DEFAULT_PINOUT_RODIAQUARIUMSOLENOID = 36;
const int DEFAULT_PINOUT_RODIRESERVOIRSOLENOID = 37;
const int DEFAULT_PINOUT_AQUARIUMDRAINSOLENOID = 38;
const int DEFAULT_PINOUT_SPARESOLENOID = 39;
const int DEFAULT_PINOUT_RESERVOIRUPPERFLOATVALVE = 40;
const int DEFAULT_PINOUT_RESERVOIRLOWERFLOATVALVE = 41;
const int DEFAULT_PINOUT_ONEWIRE = 42;

const bool DEFAULT_VALUE_AUTOWATERCHANGES = true;
const bool DEFAULT_VALUE_AUTOFILLRESERVOIR = true;
const int DEFAULT_VALUE_AUTOFILLRESERVOIRHOUR = 6;
const int DEFAULT_VALUE_AUTOFILLRESERVOIRMINUTES = 0;
const bool DEFAULT_VALUE_AUTOCIRCULATION = true;
const int DEFAULT_VALUE_RESERVOIRPOWERHEADOUTLET = 28;
const int DEFAULT_VALUE_AQUARIUMFILLPUMPOUTLET = 29;
const int DEFAULT_VALUE_DRAINMINUTESPERGALLON = 2;
const int DEFAULT_VALUE_DRAINSECONDSPERGALLON = 30;
const int DEFAULT_VALUE_FILLMINUTESPERGALLON = 2;
const int DEFAULT_VALUE_FILLSECONDSPERGALLON = 0;
const int DEFAULT_VALUE_AUTOWATERCHANGEHOUR = 12;
const int DEFAULT_VALUE_AUTOWATERCHANGEMINUTES = 0;
const int DEFAULT_VALUE_AUTOWATERCHANGEGALLONS = 1;
const bool DEFAULT_VALUE_AUTOTOPOFF = true;
const int DEFAULT_VALUE_AUTOTOPOFFMINUTES = 1;
const int DEFAULT_VALUE_ONEWIRE = 42;

const int EEPROM_AUTOWATERCHANGES = 12;
const int EEPROM_AUTOFILLRESERVOIR = 13;
const int EEPROM_AUTOFILLRESERVOIRHOUR = 14;
const int EEPROM_AUTOFILLRESERVOIRMINUTES = 15;
const int EEPROM_AUTOCIRCULATION = 16;
const int EEPROM_RESERVOIRPOWERHEADOUTLET = 17;
const int EEPROM_AQUARIUMFILLPUMPOUTLET = 18;
const int EEPROM_DRAINMINUTESPERGALLON = 19;
const int EEPROM_DRAINSECONDSPERGALLON = 20;
const int EEPROM_FILLMINUTESPERGALLON = 21;
const int EEPROM_FILLSECONDSPERGALLON = 22;
const int EEPROM_AUTOWATERCHANGEHOUR = 23;
const int EEPROM_AUTOWATERCHANGEMINUTES = 24;
const int EEPROM_AUTOWATERCHANGEGALLONS = 25;
const int EEPROM_AUTOTOPOFF = 26;
const int EEPROM_AUTOTOPOFFMINUTES = 27;
const int EEPROM_OUTLET1 = 28;
const int EEPROM_OUTLET2 = 29;
const int EEPROM_OUTLET3 = 30;
const int EEPROM_OUTLET4 = 31;
const int EEPROM_OUTLET5 = 32;
const int EEPROM_OUTLET6 = 33;
const int EEPROM_OUTLET7 = 34;
const int EEPROM_OUTLET8 = 35;
const int EEPROM_RODIAQUARIUMSOLENOID = 36;
const int EEPROM_RODIRESERVOIRSOLENOID = 37;
const int EEPROM_AQUARIUMDRAINSOLENOID = 38;
const int EEPROM_RESERVOIRUPPERFLOATVALVE = 39;
const int EEPROM_RESERVOIRLOWERFLOATVALVE = 40;
const int EEPROM_ONEWIRE = 41;

const int EEPROM_START_ADDRESS = 12;
const int EEPROM_END_ADDRESS = 41;

class Configuration {

	private:
		bool _autoWaterChanges;          	 // If enabled, water changes are performed on a routine basis
		bool _autoFillReservoir;         	 // If enabled, start filling reservoir with RO/DI water when lowerFloatValve stops floating
		int _autoFillReservoirHour;		 	 // The hour part of time it takes to fill the reservoir
		int _autoFillReservoirMinutes;   	 // The minute part of time it takes to fill the reservoir
		bool _autoCirculation;           	 // If enabled, start the powerhead when uppserFloatValve starts floating
		int _reservoirPowerheadOutlet;  	 // The outlet the power head that stirs the reservoir is plugged int
		int _aquariumFillPumpOutlet;    	 // The outlet the booster pump that fills the aquarium is plugged in
		int _drainMinutesPerGallon;      	 // The minute part of time it takes to drain 1 gallon of water
		int _drainSecondsPerGallon;      	 // The second part of time it takes to to drain 1 gallon of water
		int _fillMinutesPerGallon;       	 // The minute part of time it takes to pump 1 gallon of water
		int _fillSecondsPerGallon;		 	 // The second part of time it takes to pump 1 gallon of water
		int _autoWaterChangeHour;   	 	 // The hour part of time to perform an automated water change (HH):MM:SS
		int _autoWaterChangeMinutes;	 	 // The minute part of time to perform an automated water change HH:(MM):SS
		int _autoWaterChangeGallons;	 	 // Total number of gallons to change during an automated water change
		bool _autoTopOff;					 // If enabled, tops up evaporated water each day
		int _autoTopOffMinutes;				 // Total number of minutes to run automatic top off each day
		int _pin_outlet1;			 	 	 // The pin that controls outlet / socket 1
		int _pin_outlet2;			 	 	 // The pin that controls outlet / socket 2
		int _pin_outlet3;           	 	 // The pin that controls outlet / socket 3
		int _pin_outlet4;           	 	 // The pin that controls outlet / socket 4
		int _pin_outlet5;           	 	 // The pin that controls outlet / socket 5
		int _pin_outlet6;               	 // The pin that controls outlet / socket 6
		int _pin_outlet7;               	 // The pin that controls outlet / socket 7
		int _pin_outlet8;               	 // The pin that controls outlet / socket 8
		int _pin_rodiAquariumSolenoid;  	 // The pin that controls RO/DI -> Aquarium solenoid
		int _pin_rodiReservoirSolenoid; 	 // The pin that controls RO/DI -> Reservoir solenoid
		int _pin_aquariumDrainSolenoid; 	 // The pin that controls aquarium drain solenoid
		int _pin_upperFloatValve;       	 // The pin that controls reservoir upper float valve
		int _pin_lowerFloatValve;	       	 // The pin that controls reservoir lower float valve
		int _onewire;						 // The pin that controls Dallas Temperature / OneWire

	public:
		Configuration();
		void load();
		void erase();
		int read(int address, uint8_t defaultValue);

		bool isAutoWaterChangesEnabled();
		bool isAutoFillReservoirEnabled();
		bool isAutoCirculationEnabled();
		bool isAutoTopOffEnabled();
		int getReservoirPowerheadOutlet();
		int getAquariumFillPumpOutlet();
		int getDrainMinutesPerGallon();
		int getDrainSecondsPerGallon();
		int getFillMinutesPerGallon();
		int getFillSecondsPerGallon();
		int getAutoWaterChangeHour();
		int getAutoWaterChangeMinutes();
		int getAutoWaterChangeGallons();
		int getAutoFillReservoirHour();
		int getAutoFillReservoirMinutes();
		int getAutoTopOffMinutes();

		// Pinouts
		void setPinOutlet1(int pin);
		int getPinOutlet1();

		void setPinOutlet2(int pin);
		int getPinOutlet2();

		void setPinOutlet3(int pin);
		int getPinOutlet3();

		void setPinOutlet4(int pin);
		int getPinOutlet4();

		void setPinOutlet5(int pin);
		int getPinOutlet5();

		void setPinOutlet6(int pin);
		int getPinOutlet6();

		void setPinOutlet7(int pin);
		int getPinOutlet7();

		void setPinOutlet8(int pin);
		int getPinOutlet8();

		void setPinRodiAquariumSolenoid(int pin);
		int getPinRodiAquariumSolenoid();

		void setPinRodiReservoirSolenoid(int pin);
		int getPinRodiReservoirSolenoid();

		void setPinAquariumDrainSolenoid(int pin);
		int getPinAquariumDrainSolenoid();

		void setPinReservoirUpperFloatValve(int pin);
		int getPinReservoirUpperFloatValve();

		void setPinReservoirLowerFloatValve(int pin);
		int getPinReservoirLowerFloatValve();

		void setOneWire(int pin);
		int getOneWire();
};

#endif

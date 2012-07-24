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

class Configuration {

	private:
		byte _dailyWaterChanges;         // If enabled, daily water changes are performed on an automatic basis
		byte _autoFillReservoir;         // If enabled, start filling reservoir with RO/DI water when lowerFloatValve stops floating
		byte _autoCirculation;           // If enabled, start the powerhead when uppserFloatValve starts floating
		byte _reservoirPowerheadOutlet;   // The outlet the power head that stirs the reservoir is plugged int
		byte _aquariumFillPumpOutlet;     // The outlet the booster pump that fills the aquarium is plugged in
		// prod = 8100000 (13.5 minutes), test = 100000 (10 seconds)
		long _drainMillisPerGallon;      // Total minutes (in milliseconds) it takes to drain 1 gallon of water from the aquarium (default: 13.5 minutes)
		// prod = 1500000 (2.5 minutes), test = 150000 (15 seconds)
		long _fillMillisPerGallon;       // Total minutes (in milliseconds) it takes to pump 1 gallon of water to the aquarium (2.5 minutes)

	public:
		Configuration();
		void erase();
		bool isDailyWaterChangesEnabled();
		bool isAutoFillReservoirEnabled();
		bool isAutoCirculationEnabled();
		int getReservoirPowerheadOutlet();
		int getAquariumFillPumpOutlet();
		long getDrainMillisPerGallon();
		long getFillMillisPerGallon();
};

#endif

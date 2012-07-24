#include "Arduino.h"
#include "SPI.h"
#include "EEPROM.h"
#include "Ethernet.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "IpAddress.h"
#include "SimpleTimer.h"
#include "Configuration.h"

extern int  __bss_end;
extern int  *__brkval;
extern void startWaterChange(int gallons, int cycle, boolean override);

// REST server - url buffer size
#define BUFSIZE 255

// REST server - API key required to communicate with the web service
String apiKey = "WW123";

// Switch debouncing
long debounceDelay = 50;                  // the debounce time; increase if the output flickers
int upperFloatValveState;                 // the current reading from the upper float valve
int upperFloatValveLastState = LOW;       // the previous reading from the upper float valve
long upperFloatValveLastDebounceTime = 0; // the last time the upper float valve pin was toggled
int lowerFloatValveState;                 // the current reading from the lower float valve
int lowerFloatValveLastState = LOW;       // the previous reading from the lower float valve
long lowerFloatValveLastDebounceTime = 0; // the last time the upper float lower pin was toggled

// Pin allocations
int sdcardPin = 4;
int ethernetPin = 53;
int outlet1 = 24;
int outlet2 = 25;
int outlet3 = 26;
int outlet4 = 27;
int outlet5 = 28;
int outlet6 = 29;
int outlet7 = 30;
int outlet8 = 31;
int rodiAquariumSolenoid = 32;
int rodiReservoirSolenoid = 33;
int aquariumDrainSolenoid = 36;
int upperFloatValve = 40;
int lowerFloatValve = 41;

// Water change variables
SimpleTimer timer;
int wcDrainTimerId, wcFillTimerId, wcDailyTimerId;
int wcTotalGallons = 0;            	   // Total number of gallons the water change session is responsible for replenishing
int wcCycle = 0;                   	   // Which iteration in the session; 1 gallon of water is changed per cycle. ie., If 5 gallons is requested, 5 cycles will take place
boolean waterChangeInProgress = false; // Water change state tracking

// Dallas temperature sensor
#define ONE_WIRE_BUS 38
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress reservoirTemp = { 0x28, 0xC5, 0x05, 0x07, 0x04, 0x00, 0x00, 0xA0 };

byte mac[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAF }; // AC
IpAddress ipAddress;
EthernetServer server(80);
EthernetClient client;

// Loads the application configs from EEPROM
Configuration config;

int main(void) {
	init();

	setup();

	for (;;)
		loop();

	return 0;
}

void setup() {

	Ethernet.begin(mac, ipAddress.getBytes());
	server.begin();

	sensors.begin();

	pinMode(upperFloatValve, INPUT);
	pinMode(lowerFloatValve, INPUT);
}

void sendHtmlHeader() {

	client.println("<h5>AquariumPilot v1.0</h5>");
}

void send404() {

	client.println("HTTP/1.1 404 Not Found");
	client.println("Content-Type: text/html");
	client.println("X-Powered-By: AquariumPilot v1.0");
	client.println();

	sendHtmlHeader();
	client.println("<h1>Not Found</h1>");
}

void reply(int statusCode, String statusMessage, String body) {

	client.print("HTTP/1.1 ");
	client.print(statusCode);
	client.print(" ");
	client.print(statusMessage);
	client.println("Content-Type: text/html");
	client.println("X-Powered-By: AquariumPilot v1.0");
	client.println();

	sendHtmlHeader();
	client.println(body);
}

void checkUpperReservoirLevel() {

	int reading = digitalRead(upperFloatValve);

	// If the float changes position due to noise or floating
	if (reading != upperFloatValveLastState) {

		// reset the debouncing timer
		upperFloatValveLastDebounceTime = millis();
	}

	if ((millis() - upperFloatValveLastDebounceTime) > debounceDelay) {

		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state
		upperFloatValveState = reading;
	}

	// The float valve goes LOW when the float is floating (in the up position)
	if (upperFloatValveState == LOW) {

		// Turn off the RO/DI solenoid valve
		digitalWrite(rodiReservoirSolenoid, LOW);

		// Turn on the powerhead if auto circulate is enabled
		if (config.isAutoCirculationEnabled())
			digitalWrite(config.getReservoirPowerheadOutlet(), HIGH);
	}

	// save the reading.  Next time through the loop, it'll be the upperFloatValveLastState
	upperFloatValveLastState = reading;
}

void checkLowerReservoirLevel() {

	int reading = digitalRead(lowerFloatValve);

	// If the float changes position due to noise or floating
	if (reading != lowerFloatValveLastState) {

		// reset the debouncing timer
		lowerFloatValveLastDebounceTime = millis();
	}

	if ((millis() - lowerFloatValveLastDebounceTime) > debounceDelay) {

		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state
		lowerFloatValveState = reading;
	}

	// The float valve goes HIGH when the float is not floating (in the down position)
	if (lowerFloatValveState == HIGH) {

		// Turn on the RO/DI solenoid valve if auto fill is enabled
		if (config.isAutoFillReservoirEnabled())
			digitalWrite(rodiReservoirSolenoid, HIGH);

		// Turn off the powerhead
		digitalWrite(config.getReservoirPowerheadOutlet(), LOW);
	}

	// save the reading.  Next time through the loop, it'll be the lowerFloatValveLastState
	lowerFloatValveLastState = reading;
}

void updateWaterChangeCounters() {

	//if(!wcDrainTimerId && !wcFillTimerId) {
	if(!timer.isEnabled(wcDrainTimerId) && !timer.isEnabled(wcFillTimerId)) {

		// If more water to change, update counters and start the next cycle
		if(wcTotalGallons > wcCycle) {
			wcCycle++;
			startWaterChange(wcTotalGallons, wcCycle, true);
			return;
		}

		// Water change complete
		waterChangeInProgress = false;
		wcCycle = 0;
		wcTotalGallons = 0;
	}
}

void turnOffAquariumDrain() {

	digitalWrite(aquariumDrainSolenoid, LOW);
	timer.disable(wcDrainTimerId);
	updateWaterChangeCounters();
}

void turnOffAquariumFillPump() {

	digitalWrite(config.getAquariumFillPumpOutlet(), LOW);
	timer.disable(wcFillTimerId);
	updateWaterChangeCounters();
}

void startWaterChange(int gallons, int cycle, boolean override) {

	if(waterChangeInProgress && !override) {
		return;
	}

	waterChangeInProgress = true;
	wcTotalGallons = gallons;
	wcCycle = cycle;

	wcDrainTimerId = timer.setTimeout(config.getDrainMillisPerGallon(), turnOffAquariumDrain);
	wcFillTimerId = timer.setTimeout(config.getFillMillisPerGallon(), turnOffAquariumFillPump);

	digitalWrite(aquariumDrainSolenoid, HIGH);
	digitalWrite(config.getAquariumFillPumpOutlet(), HIGH);
}

void stopWaterChange() {

	digitalWrite(aquariumDrainSolenoid, LOW);
	digitalWrite(config.getAquariumFillPumpOutlet(), LOW);

	if(wcDrainTimerId) timer.disable(wcDrainTimerId);
	if(wcFillTimerId) timer.disable(wcFillTimerId);
}

int availableMemory() {

	int free_memory;

	if ((int) __brkval == 0)
		free_memory = ((int) &free_memory) - ((int) &__bss_end);
	else
		free_memory = ((int) &free_memory) - ((int) __brkval);
	return free_memory;
}

void factoryReset() {

	ipAddress.erase();
	config.erase();
}

void loop() {

	timer.run();

	// Check the position of the float valve located at the top of the reservoir
	checkUpperReservoirLevel();

	client = server.available();

	char clientline[BUFSIZE];
	int index = 0;

	if (client) {

		//reset input buffer
		index = 0;

		while (client.connected()) {

			if (client.available()) {

				char c = client.read();

				if (c != '\n' && c != '\r' && index < BUFSIZE) {
					clientline[index++] = c;
					continue;
				}

				client.flush();

				String urlString = String(clientline);
				String op = urlString.substring(0, urlString.indexOf(' '));
				urlString = urlString.substring(urlString.indexOf('/'), urlString.indexOf(' ', urlString.indexOf('/')));
				urlString.toCharArray(clientline, BUFSIZE);

				char *key = strtok(clientline, "/");
				char *resource = strtok(NULL, "/");
				char *param1 = strtok(NULL, "/");
				char *param2 = strtok(NULL, "/");

				if (!apiKey.equals(key)) {
					reply(403, "Forbidden", "<h1>Forbidden</h1>");
					break;
				}

				char outValue[20] = "";
				String jsonOut = String();
				int address = atoi(param1);

				if (param1 != NULL) {  // Pin / address / id

					//
					// REST resources that use the "pin" parameter but no value....
					//

					if (strncmp(resource, "waterchange", 11) == 0) {
						startWaterChange(atoi(param1), 1, false);  // says pin, but its really the value. 1 = number of gallons to change
						jsonOut = "tRu";
					}

					//
					// REST resources that use both "pin" and "value" parameters
					//

					if (param2 != NULL) { // This is a write operation

						pinMode(address, OUTPUT);

						if (strncmp(resource, "digital", 7) == 0) {

							if (strncmp(param2, "HIGH", 4) == 0) {
								digitalWrite(address, HIGH);
							}

							if (strncmp(param2, "LOW", 3) == 0) {
								digitalWrite(address, LOW);
							}
						}
						else if (strncmp(resource, "analog", 6) == 0) {
							analogWrite(address, atoi(param2));
						}
						else if (strncmp(resource, "eeprom", 6) == 0) {
							EEPROM.write(address, atoi(param2));
						}

						//  return status
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: text/html");
						client.println("X-Powered-By: AquariumPilot v1.0");
						client.println();
						break;
					}
					else { // No value specified -- read operation

						if (strncmp(resource, "eeprom", 6) == 0) {
							sprintf(outValue, "%s", EEPROM.read(address));
						}
						else if (strncmp(resource, "temp", 4) == 0) {

							sensors.requestTemperatures();

							if (address == 1) {
								float fTemp = sensors.getTempF(reservoirTemp);
								dtostrf(fTemp, 2, 2, outValue);
							}
						}
						else if (strncmp(resource, "analog", 6) == 0) {
							sprintf(outValue, "%d", analogRead(address));
						}
						else if (strncmp(resource, "digital", 7) == 0) {

							int selectedPin = atoi(param1);
							int inValue = digitalRead(selectedPin);

							if (inValue <= 0) {
								sprintf(outValue, "%s", "LOW");
								//sprintf(outValue,"%d",digitalRead(selectedPin));
							}

							if (inValue >= 1) {
								sprintf(outValue, "%s", "HIGH");
							}
						}

						//  assemble the json output
						jsonOut += "{\"";
						jsonOut += param1;
						jsonOut += "\":\"";
						jsonOut += outValue;
						jsonOut += "\"}";

						//  return value with wildcarded Cross-origin policy
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: text/html");
						client.println("Access-Control-Allow-Origin: *");
						client.println("X-Powered-By: AquariumPilot v1.0");
						client.println();
						client.println(jsonOut);
						break;
					}
				}
				//
				// REST resources that use neither "pin" or "value" parameters...
				//
				// These resources are not related to the arduino, but rather the
				// purpose / implementation itself.
				//
				if (strncmp(resource, "system", 6) == 0) {

					jsonOut += "{";
						jsonOut += "\"ipAddress\":\"" + ipAddress.toString() + "\", ";
						jsonOut += "\"availableMemory\":\"" + String(availableMemory()) + "\", ";
						jsonOut += "\"uptime\":\"" + String(millis()) + "\" ";
					jsonOut += "}";
				}
				else if (strncmp(resource, "status", 6) == 0) {

					sensors.requestTemperatures();

					char reservoirTempF[6];
					float fTemp = sensors.getTempF(reservoirTemp);
					dtostrf(fTemp, 2, 2, reservoirTempF);

					jsonOut += "{";
						jsonOut += "\"outlet1\":\"" + String(digitalRead(outlet1)) + "\", ";
						jsonOut += "\"outlet2\":\"" + String(digitalRead(outlet2)) + "\", ";
						jsonOut += "\"outlet3\":\"" + String(digitalRead(outlet3)) + "\", ";
						jsonOut += "\"outlet4\":\"" + String(digitalRead(outlet4)) + "\", ";
						jsonOut += "\"outlet5\":\"" + String(digitalRead(outlet5)) + "\", ";
						jsonOut += "\"outlet6\":\"" + String(digitalRead(outlet6)) + "\", ";
						jsonOut += "\"outlet7\":\"" + String(digitalRead(outlet7)) + "\", ";
						jsonOut += "\"outlet8\":\"" + String(digitalRead(outlet8)) + "\", ";
						jsonOut += "\"rodiAquariumSolenoid\":\"" + String(digitalRead(rodiAquariumSolenoid)) + "\", ";
						jsonOut += "\"rodiReservoirSolenoid\":\"" + String(digitalRead(rodiReservoirSolenoid)) + "\", ";
						jsonOut += "\"aquariumDrainSolenoid\":\"" + String(digitalRead(aquariumDrainSolenoid)) + "\", ";
						jsonOut += "\"upperFloatValve\":\"" + String(digitalRead(upperFloatValve)) + "\", ";
						jsonOut += "\"lowerFloatValve\":\"" + String(digitalRead(lowerFloatValve)) + "\", ";
						jsonOut += "\"reservoirTemp\":\"" + String(reservoirTempF) + "\", ";
						jsonOut += "\"waterChangeInProgress\":\"" + String(waterChangeInProgress) + "\", ";
						jsonOut += "\"wcTotalGallons\":\"" + String(wcTotalGallons) + "\", ";
						jsonOut += "\"wcCycle\":\"" + String(wcCycle) + "\", ";
						jsonOut += "\"wcDrainTimerId\":\"" +  String(wcDrainTimerId) + "\", ";
						jsonOut += "\"wcDrainTimerEnabled\":\"" + String(timer.isEnabled(wcDrainTimerId)) + "\", ";
						jsonOut += "\"wcFillTimerId\":\"" +  String(wcFillTimerId) + "\", ";
						jsonOut += "\"wcFillTimerEnabled\":\"" + String(timer.isEnabled(wcFillTimerId)) + "\", ";
						jsonOut += "\"numTimers\":\"" + String(timer.getNumTimers()) + "\" ";
					jsonOut += "}";
				}
				else if (strncmp(resource, "pinout", 6) == 0) {

					jsonOut += "{";
						jsonOut += "\"outlet1\":\"" + String(outlet1) + "\", ";
						jsonOut += "\"outlet2\":\"" + String(outlet2) + "\", ";
						jsonOut += "\"outlet3\":\"" + String(outlet3) + "\", ";
						jsonOut += "\"outlet4\":\"" + String(outlet4) + "\", ";
						jsonOut += "\"outlet5\":\"" + String(outlet5) + "\", ";
						jsonOut += "\"outlet6\":\"" + String(outlet6) + "\", ";
						jsonOut += "\"outlet7\":\"" + String(outlet7) + "\", ";
						jsonOut += "\"outlet8\":\"" + String(outlet8) + "\", ";
						jsonOut += "\"rodiAquariumSolenoid\":\"" + String(rodiAquariumSolenoid) + "\", ";
						jsonOut += "\"rodiReservoirSolenoid\":\"" + String(rodiReservoirSolenoid) + "\", ";
						jsonOut += "\"aquariumDrainSolenoid\":\"" + String(aquariumDrainSolenoid) + "\", ";
						jsonOut += "\"upperFloatValve\":\"" + String(upperFloatValve) + "\", ";
						jsonOut += "\"lowerFloatValve\":\"" + String(lowerFloatValve) + "\" ";
					jsonOut += "}";
				}
				else if (strncmp(resource, "config", 6) == 0) {

					jsonOut += "{";
						jsonOut += "\"dailyWaterChangesEnabled\":\"" + String(config.isAutoFillReservoirEnabled()) + "\", ";
						jsonOut += "\"autoFillReservoirEnabled\":\"" + String(config.isAutoFillReservoirEnabled()) + "\", ";
						jsonOut += "\"autoCirculationEnabled\":\"" + String(config.isAutoCirculationEnabled()) + "\", ";
						jsonOut += "\"reservoirPowerheadOutlet\":\"" + String(config.getReservoirPowerheadOutlet()) + "\", ";
						jsonOut += "\"aquariumFillPumpOutlet\":\"" + String(config.getAquariumFillPumpOutlet()) + "\", ";
						jsonOut += "\"drainMillisPerGallon\":\"" + String(config.getDrainMillisPerGallon()) + "\", ";
						jsonOut += "\"_fillMillisPerGallon\":\"" + String(config.getFillMillisPerGallon()) + "\" ";
					jsonOut += "}";
				}
				else if (strncmp(resource, "reset", 12) == 0) {
					factoryReset();
				}
				else {
					send404();
				}

				client.println("HTTP/1.1 200 OK");
				client.println("Content-Type: text/html");
				client.println("Access-Control-Allow-Origin: *");
				client.println("X-Powered-By: AquariumPilot v1.0");
				client.println();
				client.println(jsonOut);

				break;
			}
		}

		// give the web browser time to receive the data
		delay(100);

		// close the connection:
		client.stop();
	}

	// Check the position of the float valve located at the bottom of the reservoir
	checkLowerReservoirLevel();
}

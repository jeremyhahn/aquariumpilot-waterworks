#include "Arduino.h"
#include "SPI.h"
#include "EEPROM.h"
#include "Ethernet.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "SimpleTimer.h"
#include "IpAddress.h"
#include "Netmask.h"
#include "Gateway.h"
#include "Configuration.h"

//
// TODO: Implement SMTP
//
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235534880/15
//

extern int  __bss_end;
extern int  *__brkval;
extern void startWaterChange(int gallons, int cycle, boolean override);
extern void startAutoWaterChange();
extern void sendNotification(String body);

// REST server - url buffer size
#define BUFSIZE 255

// REST server - API key required to communicate with the web service
String apiKey = "ABC123";

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
int maintenanceInProgress = false;

// Dallas temperature sensor
#define ONE_WIRE_BUS 38
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress reservoirTemp = { 0x28, 0xC5, 0x05, 0x07, 0x04, 0x00, 0x00, 0xA0 };

byte mac[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAF }; // AC
byte mailServer[] = { 172, 16, 201, 2 };
IpAddress ipAddress;
Netmask netmask;
Gateway gateway;
EthernetServer httpServer(80);
EthernetClient httpClient;
EthernetClient smtpClient;

String lastError;

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

	//for(int i = 0; i < 512; i++)
	//	EEPROM.write(i, 0);

	//ipAddress.set(192, 168, 11, 51);
	//netmask.set(255, 255, 255, 0);
	//gateway.set(192, 168, 11, 1);

	Ethernet.begin(mac, ipAddress.getBytes());
	httpServer.begin();

	sensors.begin();

	pinMode(outlet1, OUTPUT);
	pinMode(outlet2, OUTPUT);
	pinMode(outlet3, OUTPUT);
	pinMode(outlet4, OUTPUT);
	pinMode(outlet5, OUTPUT);
	pinMode(outlet6, OUTPUT);
	pinMode(outlet7, OUTPUT);
	pinMode(outlet8, OUTPUT);
	pinMode(rodiAquariumSolenoid, OUTPUT);
	pinMode(rodiReservoirSolenoid, OUTPUT);
	pinMode(aquariumDrainSolenoid, OUTPUT);

	pinMode(upperFloatValve, INPUT);
	pinMode(lowerFloatValve, INPUT);

	if(config.isAutoWaterChangesEnabled()) {
		wcDailyTimerId = timer.setInterval(config.getWaterChangeMillis(), startAutoWaterChange);
	}
}

void sendHtmlHeader() {

	httpClient.println("<h5>AquariumPilot v1.0</h5>");
}

void send404() {

	httpClient.println("HTTP/1.1 404 Not Found");
	httpClient.println("Content-Type: text/html");
	httpClient.println("X-Powered-By: AquariumPilot v1.0");
	httpClient.println();

	sendHtmlHeader();
	httpClient.println("<h1>Not Found</h1>");
}

void reply(int statusCode, String statusMessage, String body) {

	httpClient.print("HTTP/1.1 ");
	httpClient.print(statusCode);
	httpClient.print(" ");
	httpClient.print(statusMessage);
	httpClient.println("Content-Type: text/html");
	httpClient.println("X-Powered-By: AquariumPilot v1.0");
	httpClient.println();

	sendHtmlHeader();
	httpClient.println(body);
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
		sendNotification("Upper float valve triggered. RO/DI valve is now off.");

		// Turn on the powerhead if auto circulate is enabled
		if (config.isAutoCirculationEnabled()) {
			digitalWrite(config.getReservoirPowerheadOutlet(), HIGH);
			sendNotification("Upper float valve triggered. Auto-circulation is enabled. The powerhead is now on.");
		}
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
		if (config.isAutoFillReservoirEnabled()) {
			digitalWrite(rodiReservoirSolenoid, HIGH);
			sendNotification("Lower float valve triggered. Autofill is enabled. RO/DI valve is now on.");
		}

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
		int gallons = wcTotalGallons;

		waterChangeInProgress = false;
		wcCycle = 0;
		wcTotalGallons = 0;

		String body = String("Your water change of ");
		body.concat(gallons);
		body.concat(" ");
		body.concat(gallons > 1 ? "gallons" : "gallon");
		body.concat(" is now complete.");
		sendNotification(body);
	}
}

void turnOffAquariumDrain() {

	digitalWrite(aquariumDrainSolenoid, LOW);
	timer.disable(wcDrainTimerId);
	updateWaterChangeCounters();
	sendNotification("Aquarium drain is now off.");
}

void turnOffAquariumFillPump() {

	digitalWrite(config.getAquariumFillPumpOutlet(), LOW);
	timer.disable(wcFillTimerId);
	updateWaterChangeCounters();
	sendNotification("Aquarium fill pump is now off.");
}

void startWaterChange(int gallons, int cycle, boolean override) {

	if(maintenanceInProgress) return;

	if(waterChangeInProgress && !override) {
		return;
	}

	String body = String("Starting water change. Gallons: ");
	body.concat(gallons);
	body.concat(", Cycle: ");
	body.concat(cycle);
	//body.concat(", Override: ");
	//body.concat(override);
	sendNotification(body);

	waterChangeInProgress = true;
	wcTotalGallons = gallons;
	wcCycle = cycle;

	wcDrainTimerId = timer.setTimeout(config.getDrainMillisPerGallon(), turnOffAquariumDrain);
	wcFillTimerId = timer.setTimeout(config.getFillMillisPerGallon(), turnOffAquariumFillPump);

	digitalWrite(aquariumDrainSolenoid, HIGH);
	digitalWrite(config.getAquariumFillPumpOutlet(), HIGH);
}

void startAutoWaterChange() {

	if(maintenanceInProgress) return;
	startWaterChange(1, 1, false);
}

void stopWaterChange() {

	digitalWrite(aquariumDrainSolenoid, LOW);
	digitalWrite(config.getAquariumFillPumpOutlet(), LOW);

	if(wcDrainTimerId) timer.disable(wcDrainTimerId);
	if(wcFillTimerId) timer.disable(wcFillTimerId);
}

void sendNotification(String body) {

	int wait = 1000;

	if (smtpClient.connect(mailServer, 25)) {

		smtpClient.println("HELO arduino.makeabyte.com");
		delay(wait);

		smtpClient.println("MAIL FROM:<jeremy.hahn@makeabyte.com>");
		delay(wait);

		smtpClient.println("RCPT TO:<jeremy.hahn@makeabyte.com>");
		delay(wait);

		smtpClient.println("DATA");
		delay(wait);

		smtpClient.println("Subject:[AquariumPilot] Water Change Notification");
		smtpClient.println("");

		smtpClient.println(body);
		smtpClient.println("\r\n\r\n- AquariumPilot");
		smtpClient.println(".");

		smtpClient.println("QUIT");
		delay(wait);

		smtpClient.println();
		smtpClient.stop();
	}
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

	// Erase EERPOM
	//for(int i = 0; i < 512; i++)
	//    EEPROM.write(i, 0);
}

void loop() {

	timer.run();

	// Check the position of the float valve located at the top of the reservoir
	//checkUpperReservoirLevel();

	httpClient = httpServer.available();

	char clientline[BUFSIZE];
	int index = 0;

	if (httpClient) {

		//reset input buffer
		index = 0;

		while (httpClient.connected()) {

			if (httpClient.available()) {

				char c = httpClient.read();

				if (c != '\n' && c != '\r' && index < BUFSIZE) {
					clientline[index++] = c;
					continue;
				}

				httpClient.flush();

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

				if (param1 != NULL) {  // Pin / id

					//
					// REST resources that use the "pin" parameter but no value....
					//

					if (strncmp(resource, "waterchange", 11) == 0) {
						startWaterChange(atoi(param1), 1, false);  // says address, but its really the value. 1 = number of gallons to change
					}
					else if (strncmp(resource, "maintenance", 11) == 0) {
						maintenanceInProgress = atoi(param1);
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
						httpClient.println("HTTP/1.1 200 OK");
						httpClient.println("Content-Type: text/html");
						httpClient.println("X-Powered-By: AquariumPilot v1.0");
						httpClient.println();
						break;
					}
					else { // No value specified -- read operation

						if (strncmp(resource, "eeprom", 6) == 0) {
							sprintf(outValue, "%d", EEPROM.read(address));
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
						httpClient.println("HTTP/1.1 200 OK");
						httpClient.println("Content-Type: text/html");
						httpClient.println("Access-Control-Allow-Origin: *");
						httpClient.println("X-Powered-By: AquariumPilot v1.0");
						httpClient.println();
						httpClient.println(jsonOut);
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
						jsonOut += "\"netmask\":\"" + netmask.toString() + "\", ";
						jsonOut += "\"gateway\":\"" + gateway.toString() + "\", ";
						jsonOut += "\"availableMemory\":\"" + String(availableMemory()) + "\", ";
						jsonOut += "\"uptime\":\"" + String(millis()) + "\", ";
						jsonOut += "\"lastError\":\"" + lastError + "\" ";
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
						jsonOut += "\"wcDailyTimerId\":\"" +  String(wcDailyTimerId) + "\", ";
						jsonOut += "\"wcDailyTimerEnabled\":\"" + String(timer.isEnabled(wcDailyTimerId)) + "\", ";
						jsonOut += "\"activeTimers\":\"" + String(timer.getNumTimers()) + "\", ";
						jsonOut += "\"maintenanceInProgress\":\"" + String(maintenanceInProgress) + "\" ";
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
						jsonOut += "\"autoWaterChangesEnabled\":\"" + String(config.isAutoWaterChangesEnabled()) + "\", ";
						jsonOut += "\"autoFillReservoirEnabled\":\"" + String(config.isAutoFillReservoirEnabled()) + "\", ";
						jsonOut += "\"autoCirculationEnabled\":\"" + String(config.isAutoCirculationEnabled()) + "\", ";
						jsonOut += "\"reservoirPowerheadOutlet\":\"" + String(config.getReservoirPowerheadOutlet()) + "\", ";
						jsonOut += "\"aquariumFillPumpOutlet\":\"" + String(config.getAquariumFillPumpOutlet()) + "\", ";
						jsonOut += "\"drainMillisPerGallon\":\"" + String(config.getDrainMillisPerGallon()) + "\", ";
						jsonOut += "\"fillMillisPerGallon\":\"" + String(config.getFillMillisPerGallon()) + "\", ";
						jsonOut += "\"waterChangeMillis\":\"" + String(config.getWaterChangeMillis()) + "\" ";
					jsonOut += "}";
				}
				else if (strncmp(resource, "reset", 12) == 0) {
					factoryReset();
				}
				else {
					send404();
				}

				httpClient.println("HTTP/1.1 200 OK");
				httpClient.println("Content-Type: text/html");
				httpClient.println("Access-Control-Allow-Origin: *");
				httpClient.println("X-Powered-By: AquariumPilot v1.0");
				httpClient.println();
				httpClient.println(jsonOut);

				break;
			}
		}

		// give the web browser time to receive the data
		delay(100);

		// close the connection:
		httpClient.stop();
	}

	// Check the position of the float valve located at the bottom of the reservoir
	//checkLowerReservoirLevel();
}

/**
 * AquariumPilot - Aquarium automation
 *
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#include "Arduino.h"
#include "SPI.h"
#include "EEPROM.h"
#include "Ethernet.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "Timer.h"
#include "IpAddress.h"
#include "Netmask.h"
#include "Gateway.h"
#include "Configuration.h"
#include "Wire.h"
#include "Chronodot.h"

extern int  __bss_end;
extern int  *__brkval;
extern void startWaterChange(int gallons, int cycle);
extern void startAutoWaterChange();
extern void updateWaterChangeCounters();
extern void sendNotification(String message, String body);

#define DEBUG true

// Web service request buffer size
#define BUFSIZE 255

// API key required to communicate with the web service
String apiKey = "ABC123";

// Switch debouncing
long debounceDelay = 50;                  // the debounce time; increase if the output flickers
int upperFloatValveState;                 // the current reading from the upper float valve
int upperFloatValveLastState = LOW;       // the previous reading from the upper float valve
long upperFloatValveLastDebounceTime = 0; // the last time the upper float valve pin was toggled
int lowerFloatValveState;                 // the current reading from the lower float valve
int lowerFloatValveLastState = LOW;       // the previous reading from the lower float valve
long lowerFloatValveLastDebounceTime = 0; // the last time the upper float lower pin was toggled

// Automated water changes
Timer timer;
int wcDrainTimerId, wcFillTimerId = -1; // Water change timer ids
int wcTotalGallons = 0;            	    // Total number of gallons the water change session is responsible for replenishing
int wcCycle = 0;                   	    // Which iteration in the session; 1 gallon of water is changed per cycle. ie., If 5 gallons is requested, 5 cycles will take place
int wcLastChangedMonth;				    // The month when the automated water change last took place
int wcLastChangedDay;				    // The day when the automated water change last took place
bool maintenanceInProgress = false;    // Maintenance mode state tracking
bool waterChangeInProgress = false;    // Water change state tracking
bool reservoirFillInProgress = false;  // Reservoir fill state tracking
bool aquariumDrainInProgress = false;  // Aquarium drain state tracking
bool aquariumFillInProgress = false;   // Aquarium fill state tracking

// Automated top off
int topOffTimerId;                      // Auto top off timer id
int topOffLastMonth;				    // The month when the automated top off last took place
int topOffLastDay;				   	    // The day when the automated top off last took place
bool topOffInProgress = false;		    // Aquarium top off state tracking
// TODO: REPLACE HARD CODED TOP OFF CONST W/ ENTRIES IN CONFIGURATION CLASS
const int AUTOTOPOFFHOUR = 14;
const int AUTOTOPOFFMINUTE = 0;

// Dallas temperature sensor
#define ONE_WIRE_BUS 38
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress reservoirTemp = { 0x28, 0xC5, 0x05, 0x07, 0x04, 0x00, 0x00, 0xA0 };
DeviceAddress roomTemp = { 0x28, 0x2A, 0x74, 0x28, 0x04, 0x00, 0x00, 0xE7 };

byte mac[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x1F }; // AC
byte mailServer[] = { 172, 16, 201, 2 };
IpAddress ipAddress;
Netmask netmask;
Gateway gateway;
EthernetServer httpServer(80);
EthernetClient httpClient;
EthernetClient smtpClient;

// Loads the application configs from EEPROM
Configuration config;

Chronodot RTC;
/*
int seconds; //00-59;
int minutes; //00-59;
int hours;//1-12 - 00-23;
int day;//1-7
int date;//01-31
int month;//01-12
int year;//0-99;
*/
DateTime now;
int yesterday;
String dateString;
String timeString;
String dateTimeString;

int main(void) {

	init();

	setup();

	for (;;)
		loop();

	return 0;
}

void setup() {

	Serial.begin(115200);
	Serial.println("Initializing...");

	//int sdcardPin = 4;
	//int ethernetPin = 53;

	//for(int i = 0; i < 512; i++)
		//EEPROM.write(i, 0);

	//ipAddress.set(192, 168, 11, 51);
	//netmask.set(255, 255, 255, 0);
	//gateway.set(192, 168, 11, 1);
    //config.erase();

	Serial.print("Address: ");
	Serial.println(ipAddress.toString());

	Serial.print("Netmask: ");
	Serial.println(netmask.toString());

	Serial.print("Gateway: ");
	Serial.println(gateway.toString());

	// mac, ip, dns, gateway, subnet
	Ethernet.begin(mac, ipAddress.getBytes(), gateway.getBytes(), gateway.getBytes(), netmask.getBytes());
	httpServer.begin();

	sensors.begin();

	pinMode(config.getPinOutlet1(), OUTPUT);
	pinMode(config.getPinOutlet2(), OUTPUT);
	pinMode(config.getPinOutlet3(), OUTPUT);
	pinMode(config.getPinOutlet4(), OUTPUT);
	pinMode(config.getPinOutlet5(), OUTPUT);
	pinMode(config.getPinOutlet6(), OUTPUT);
	pinMode(config.getPinOutlet7(), OUTPUT);
	pinMode(config.getPinOutlet8(), OUTPUT);
	pinMode(config.getPinRodiAquariumSolenoid(), OUTPUT);
	pinMode(config.getPinRodiReservoirSolenoid(), OUTPUT);
	pinMode(config.getPinAquariumDrainSolenoid(), OUTPUT);

	pinMode(config.getPinReservoirUpperFloatValve(), INPUT);
	pinMode(config.getPinReservoirLowerFloatValve(), INPUT);

	Wire.begin();
	RTC.begin();
	//setSyncProvider(RTC.get); // set sync to use the ChronoDot
	//setSyncInterval(10); // sync every 10 seconds if possible

	// Set the real-time clock
	// uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, int tempF, float tempC
	//RTC.adjust(DateTime((uint16_t) 2012, (uint8_t) 8, (uint8_t) 30, (uint8_t) 15, (uint8_t) 38, (uint8_t) 0, 0, (float) 0));
}

void debug(String message) {

	Serial.print("[");
	Serial.print(dateTimeString);
	Serial.print("] ");
	Serial.println(message);
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

void sendNotification(String subject, String body) {

	#ifdef DEBUG
	debug("sendNotification()");
	#endif

	int wait = 1500;

	if (smtpClient.connect(mailServer, 25)) {

		smtpClient.println("EHLO aquariumpilot.bostonsaltwater.com");
		delay(wait);

		smtpClient.println("MAIL FROM:<jeremy.hahn@makeabyte.com>");
		delay(wait);

		smtpClient.println("RCPT TO:<jeremy.hahn@makeabyte.com>");
		delay(wait);

		smtpClient.println("DATA");
		delay(wait);

		smtpClient.print("Subject:[TESTING! AquariumPilot] ");
		smtpClient.println(subject);
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

void stopAquariumDrain() {

	#ifdef DEBUG
	debug("stopAquariumDrain()");
	#endif

	digitalWrite(config.getPinAquariumDrainSolenoid(), LOW);
	wcDrainTimerId = -1;
	aquariumDrainInProgress = false;
	updateWaterChangeCounters();

	String message = "Aquarium drain is now off.";
	sendNotification("Aquarium Drain Event", message);

	#ifdef DEBUG
	debug(message);
	#endif
}

void stopAquariumFillPump() {

	#ifdef DEBUG
	debug("stopAquariumFillPump()");
	#endif

	digitalWrite(config.getAquariumFillPumpOutlet(), LOW);
	wcFillTimerId = -1;
	aquariumFillInProgress = false;
	updateWaterChangeCounters();

	String message = "Aquarium fill pump is now off.";
	sendNotification("Aquarium Fill Event", message);

	#ifdef DEBUG
	debug(message);
	#endif
}

void stopAquariumRoDi() {

	digitalWrite(config.getPinRodiAquariumSolenoid(), LOW);
}

void startAquariumRoDi() {

	digitalWrite(config.getPinRodiAquariumSolenoid(), HIGH);
}

void startReservoirPowerhead() {

	digitalWrite(config.getReservoirPowerheadOutlet(), HIGH);
}

void stopReservoirPowerhead() {

	digitalWrite(config.getReservoirPowerheadOutlet(), LOW);
}

void startReservoirRoDi() {

	digitalWrite(config.getPinRodiReservoirSolenoid(), HIGH);
}

void stopReservoirRoDi() {

	digitalWrite(config.getPinRodiReservoirSolenoid(), LOW);
}

void stopAutoTopOff() {

	#ifdef DEBUG
	debug("stopAutoTopOff()");
	#endif

	stopAquariumRoDi();
	topOffInProgress = false;
	topOffTimerId = -1;

	String message = "Automatic top off stopped.";
	sendNotification("Automated Top Off", message);

	#ifdef DEBUG
	debug(message);
	#endif
}

void startAutoTopOff() {

	#ifdef DEBUG
	debug("startAutoTopOff()");
	#endif

	if(maintenanceInProgress) return;

	topOffInProgress = true;
	startAquariumRoDi();

	long topOffMillis = config.getAutoTopOffMinutes() * 60000;
	topOffTimerId = timer.after(topOffMillis, stopAutoTopOff);

	String message = "Automatic top off started.";
	sendNotification("Automated Top Off Event", message);

	#ifdef DEBUG
	debug(message);
	#endif

	topOffLastMonth = now.month();
	topOffLastDay = now.day();
}

void startWaterChange(int gallons, int cycle) {

	#ifdef DEBUG
	debug("startWaterChange()");
	#endif

	if(maintenanceInProgress || waterChangeInProgress) return;

	waterChangeInProgress = true;
	aquariumFillInProgress = true;
	aquariumDrainInProgress = true;
	wcTotalGallons = gallons;
	wcCycle = cycle;

	long drainMillis = config.getDrainMinutesPerGallon() * 60000;
	drainMillis += config.getDrainSecondsPerGallon() * 1000;

	long fillMillis = config.getFillMinutesPerGallon() * 60000;
	fillMillis += config.getFillSecondsPerGallon() * 1000;

	wcDrainTimerId = timer.after(drainMillis, stopAquariumDrain);
	wcFillTimerId = timer.after(fillMillis, stopAquariumFillPump);

	digitalWrite(config.getPinAquariumDrainSolenoid(), HIGH);
	digitalWrite(config.getAquariumFillPumpOutlet(), HIGH);

	String body = String("Starting water change of ");
	body.concat(gallons);
	body.concat(" gallons. Fill scheduled for ");
	body.concat(config.getFillMinutesPerGallon());
	body.concat(":");
	body.concat(config.getFillSecondsPerGallon());
	body.concat(". Drain scheduled for ");
	body.concat(config.getDrainMinutesPerGallon());
	body.concat(":");
	body.concat(config.getDrainSecondsPerGallon());
	body.concat(". Drain Timer Id: ");
	body.concat(wcDrainTimerId);
	body.concat(". Fill Timer Id: ");
	body.concat(wcFillTimerId);
	body.concat(". Cycle: ");
	body.concat(cycle);
	body.concat(", DateTime: ");
	body.concat(dateTimeString);
	sendNotification("Water Change Event", body);

	#ifdef DEBUG
	debug(body);
	#endif
}

void startAutoWaterChange() {

	#ifdef DEBUG
	debug("startAutoWaterChange()");
	#endif

	if(maintenanceInProgress) return;
	startWaterChange(config.getAutoWaterChangeGallons(), 1);
}

/*
 * Unused
 *
void stopWaterChange() {

	Serial.println("stopWaterChange()");

	stopAquariumDrain();
	stopAquariumFillPump();

	wcDrainTimerId = 0;
	wcFillTimerId = 0;

	waterChangeInProgress = false;
	aquariumFillInProgress = false;
	aquariumDrainInProgress = false;
}*/

void updateWaterChangeCounters() {

	#ifdef DEBUG
	debug("updateWaterChangeCounters()");
	#endif

	if(!aquariumDrainInProgress && !aquariumFillInProgress) {

		// If more water to change, update counters and start the next cycle
		if(wcTotalGallons > wcCycle) {
			wcCycle++;
			startWaterChange(wcTotalGallons, wcCycle);
			return;
		}

		// Water change complete
		int gallons = wcTotalGallons;

		waterChangeInProgress = false;
		wcLastChangedMonth = now.month();
		wcLastChangedDay = now.day();

		wcCycle = 0;
		wcTotalGallons = 0;

		String body = String("Your water change of ");
		body.concat(gallons);
		body.concat(" ");
		body.concat(gallons > 1 ? "gallons" : "gallon");
		body.concat(" is now complete. DateTime: ");
		body.concat(dateTimeString);
		sendNotification("Water Change Event", body);

		#ifdef DEBUG
		debug(body);
		#endif
	}
}

int availableMemory() {

	#ifdef DEBUG
	debug("availableMemory()");
	#endif

	int free_memory;

	if ((int) __brkval == 0)
		free_memory = ((int) &free_memory) - ((int) &__bss_end);
	else
		free_memory = ((int) &free_memory) - ((int) __brkval);

	return free_memory;
}

void factoryReset() {

	#ifdef DEBUG
	debug("factoryReset()");
	#endif

	ipAddress.erase();
	netmask.erase();
	gateway.erase();
	config.erase();
}

void checkUpperReservoirLevel() {

	//Serial.println("checkUpperReservoirLevel()");

	int reading = digitalRead(config.getPinReservoirUpperFloatValve());

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

	// The upper float valve goes LOW when the float is floating (in the up position)
	if(upperFloatValveState == LOW && reservoirFillInProgress) {

		stopAquariumRoDi();
		reservoirFillInProgress = false;
		String message = "Reservoir RO/DI is now off.";

		if(config.isAutoCirculationEnabled()) {

		   startReservoirPowerhead();
		   message += " Powerhead is now on.";
		}

		sendNotification("Reservoir Upper Float Event", message);

		#ifdef DEBUG
		debug(message);
		#endif
	}

	// save the reading.  Next time through the loop, it'll be the upperFloatValveLastState
	upperFloatValveLastState = reading;
}

void checkLowerReservoirLevel() {

	//Serial.println("checkLowerReservoirLevel()");

	int reading = digitalRead(config.getPinReservoirLowerFloatValve());

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
	if(lowerFloatValveState == HIGH && !maintenanceInProgress) {

		String message = "";

		if (config.isAutoFillReservoirEnabled()) {

			startReservoirRoDi();
			reservoirFillInProgress = true;

			message += "Reservoir RO/DI is now on.";
		}

		stopReservoirPowerhead();
        maintenanceInProgress = true;

        message += " Powerhead is now off.";
        sendNotification("Reservoir Lower Float Event", message);

		#ifdef DEBUG
        debug(message);
		#endif
	}

	// save the reading.  Next time through the loop, it'll be the lowerFloatValveLastState
	lowerFloatValveLastState = reading;
}

void loop() {

	//Serial.println("loop()");

	// "tick" the timer
	timer.update();

	// Date/Time formatting
	now = RTC.now();
	dateString = String(now.month()) + "/" + String(now.day()) + "/" + String(now.year());
	timeString = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
	dateTimeString = dateString + " " + timeString;

	// Automated water changes
	if(config.isAutoWaterChangesEnabled()) {

		bool hoursMatter = now.day() == wcLastChangedDay;
		bool minutesMatter = now.hour() == config.getAutoWaterChangeHour();

		if(now.month() >= wcLastChangedMonth
				&& now.day() != wcLastChangedDay
				&& (hoursMatter ? (now.hour() >= config.getAutoWaterChangeHour()) : true)
				&& (minutesMatter ? (now.minute() >= config.getAutoWaterChangeMinutes()) : true)
				&& !waterChangeInProgress) {

		   startAutoWaterChange();
		}
	}

	// Automated top off
	if(config.isAutoTopOffEnabled()) {

		bool hoursMatter = now.day() == topOffLastDay;
		bool minutesMatter = now.hour() == AUTOTOPOFFHOUR;

		if(now.month() >= topOffLastMonth
				&& now.day() != topOffLastDay
				&& (hoursMatter ? (now.hour() >= AUTOTOPOFFHOUR) : true)
				&& (minutesMatter ? (now.minute() >= AUTOTOPOFFMINUTE) : true)
				&& !topOffInProgress) {

			startAutoTopOff();
		}
	}

	// Check the position of the float valve located at the top of the reservoir
	checkUpperReservoirLevel();

	// Listen for web service requests
	httpClient = httpServer.available();

	char clientline[BUFSIZE];
	int index = 0;

	if (httpClient) {

		// reset input buffer
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

					// /waterchange
					if (strncmp(resource, "waterchange", 11) == 0) {
						startWaterChange(atoi(param1), 1);  // says address, but its really the value. 1 = number of gallons to change
					}
					// /maintenance
					else if (strncmp(resource, "maintenance", 11) == 0) {
						maintenanceInProgress = atoi(param1);
					}

					//
					// REST resources that use both "pin" and "value" parameters
					//

					if (param2 != NULL) { // This is a write operation

						pinMode(address, OUTPUT);

						// /digital/?
						if (strncmp(resource, "digital", 7) == 0) {

							// /digital/high/?
							if (strncmp(param2, "HIGH", 4) == 0) {
								digitalWrite(address, HIGH);
							}

							// /digital/low/?
							if (strncmp(param2, "LOW", 3) == 0) {
								digitalWrite(address, LOW);
							}
						}
						// /analog/?
						else if (strncmp(resource, "analog", 6) == 0) {
							analogWrite(address, atoi(param2));
						}
						// /eeprom/?
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

						// /eeprom
						if(strncmp(resource, "eeprom", 6) == 0) {
						   sprintf(outValue, "%d", EEPROM.read(address));
						}
						// /temp
						else if (strncmp(resource, "temp", 4) == 0) {

							sensors.requestTemperatures();

							// /temp/1
							if (address == 1) {
								float fTemp = sensors.getTempF(reservoirTemp);
								dtostrf(fTemp, 2, 2, outValue);
							}
						}
						// /analog
						else if (strncmp(resource, "analog", 6) == 0) {
							sprintf(outValue, "%d", analogRead(address));
						}
						// /digital
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

						// Create response
						jsonOut += "{\"";
						jsonOut += param1;
						jsonOut += "\":\"";
						jsonOut += outValue;
						jsonOut += "\"}";

						// Return with wildcarded Cross-origin policy
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

				// /system
				if (strncmp(resource, "system", 6) == 0) {

					jsonOut += "{";
						jsonOut += "\"ipAddress\":\"" + ipAddress.toString() + "\", ";
						jsonOut += "\"netmask\":\"" + netmask.toString() + "\", ";
						jsonOut += "\"gateway\":\"" + gateway.toString() + "\", ";
						jsonOut += "\"availableMemory\":\"" + String(availableMemory()) + "\", ";
						jsonOut += "\"uptime\":\"" + String(millis()) + "\", ";
						jsonOut += "\"hours\":\"" + String(now.hour()) + "\", ";
						jsonOut += "\"minutes\":\"" + String(now.minute()) + "\", ";
						jsonOut += "\"seconds\":\"" + String(now.second()) + "\", ";
						jsonOut += "\"dayOfWeek\":\"" + String(now.dayOfWeek()) + "\", ";
						jsonOut += "\"day\":\"" + String(now.day()) + "\", ";
						jsonOut += "\"month\":\"" + String(now.month()) + "\", ";
						jsonOut += "\"year\":\"" + String(now.year()) + "\", ";
						jsonOut += "\"tempF\":\"" + String(now.tempF()) + "\", ";
						jsonOut += "\"yesterday\":\"" + String(yesterday) + "\", ";
						jsonOut += "\"SDA\":\"" + String(SDA) + "\", ";
						jsonOut += "\"SCL\":\"" + String(SCL) + "\" ";
					jsonOut += "}";
				}
				// /status
				else if (strncmp(resource, "status", 6) == 0) {

					sensors.requestTemperatures();

					char reservoirTempF[6];
					float fTemp = sensors.getTempF(reservoirTemp);
					dtostrf(fTemp, 2, 2, reservoirTempF);

					char roomTempF[6];
					float fTemp2 = sensors.getTempF(roomTemp);
					dtostrf(fTemp2, 2, 2, roomTempF);

					jsonOut += "{";
						jsonOut += "\"outlet1\":\"" + String(digitalRead(config.getPinOutlet1())) + "\", ";
						jsonOut += "\"outlet2\":\"" + String(digitalRead(config.getPinOutlet2())) + "\", ";
						jsonOut += "\"outlet3\":\"" + String(digitalRead(config.getPinOutlet3())) + "\", ";
						jsonOut += "\"outlet4\":\"" + String(digitalRead(config.getPinOutlet4())) + "\", ";
						jsonOut += "\"outlet5\":\"" + String(digitalRead(config.getPinOutlet5())) + "\", ";
						jsonOut += "\"outlet6\":\"" + String(digitalRead(config.getPinOutlet6())) + "\", ";
						jsonOut += "\"outlet7\":\"" + String(digitalRead(config.getPinOutlet7())) + "\", ";
						jsonOut += "\"outlet8\":\"" + String(digitalRead(config.getPinOutlet8())) + "\", ";
						jsonOut += "\"rodiAquariumSolenoid\":\"" + String(digitalRead(config.getPinRodiAquariumSolenoid())) + "\", ";
						jsonOut += "\"rodiReservoirSolenoid\":\"" + String(digitalRead(config.getPinRodiReservoirSolenoid())) + "\", ";
						jsonOut += "\"aquariumDrainSolenoid\":\"" + String(digitalRead(config.getPinAquariumDrainSolenoid())) + "\", ";
						jsonOut += "\"upperFloatValve\":\"" + String(digitalRead(config.getPinReservoirUpperFloatValve())) + "\", ";
						jsonOut += "\"lowerFloatValve\":\"" + String(digitalRead(config.getPinReservoirLowerFloatValve())) + "\", ";
						jsonOut += "\"reservoirTemp\":\"" + String(reservoirTempF) + "\", ";
						jsonOut += "\"roomTemp\":\"" + String(roomTempF) + "\", ";
						jsonOut += "\"wcTotalGallons\":\"" + String(wcTotalGallons) + "\", ";
						jsonOut += "\"wcCycle\":\"" + String(wcCycle) + "\", ";
						jsonOut += "\"wcDrainTimerId\":\"" +  String(wcDrainTimerId) + "\", ";
						jsonOut += "\"wcFillTimerId\":\"" +  String(wcFillTimerId) + "\", ";
						jsonOut += "\"wcLastChangedMonth\":\"" +  String(wcLastChangedMonth) + "\", ";
						jsonOut += "\"wcLastChangedDay\":\"" +  String(wcLastChangedDay) + "\", ";
						jsonOut += "\"topOffTimerId\":\"" +  String(topOffTimerId) + "\", ";
						jsonOut += "\"topOffLastMonth\":\"" +  String(topOffLastMonth) + "\", ";
						jsonOut += "\"topOffLastDay\":\"" +  String(topOffLastDay) + "\", ";
						jsonOut += "\"yesterday\":\"" + String(yesterday) + "\", ";
						jsonOut += "\"waterChangeInProgress\":\"" + String(waterChangeInProgress) + "\", ";
						jsonOut += "\"topOffInProgress\":\"" + String(topOffInProgress) + "\", ";
						jsonOut += "\"reservoirFillInProgress\":\"" + String(reservoirFillInProgress) + "\", ";
						jsonOut += "\"aquariumFillInProgress\":\"" + String(aquariumFillInProgress) + "\", ";
						jsonOut += "\"aquariumDrainInProgress\":\"" + String(aquariumDrainInProgress) + "\", ";
						jsonOut += "\"maintenanceInProgress\":\"" + String(maintenanceInProgress) + "\" ";
					jsonOut += "}";
				}
				// /config
				else if (strncmp(resource, "config", 6) == 0) {

					// Reload EEPROM configs
					config.load();

					jsonOut += "{";
						jsonOut += "\"autoWaterChangesEnabled\":\"" + String(config.isAutoWaterChangesEnabled()) + "\", ";
						jsonOut += "\"autoFillReservoirEnabled\":\"" + String(config.isAutoFillReservoirEnabled()) + "\", ";
						jsonOut += "\"autoCirculationEnabled\":\"" + String(config.isAutoCirculationEnabled()) + "\", ";
						jsonOut += "\"reservoirPowerheadOutlet\":\"" + String(config.getReservoirPowerheadOutlet()) + "\", ";
						jsonOut += "\"aquariumFillPumpOutlet\":\"" + String(config.getAquariumFillPumpOutlet()) + "\", ";
						jsonOut += "\"drainMinutesPerGallon\":\"" + String(config.getDrainMinutesPerGallon()) + "\", ";
						jsonOut += "\"drainSecondsPerGallon\":\"" + String(config.getDrainSecondsPerGallon()) + "\", ";
						jsonOut += "\"fillMinutesPerGallon\":\"" + String(config.getFillMinutesPerGallon()) + "\", ";
						jsonOut += "\"fillSecondsPerGallon\":\"" + String(config.getFillSecondsPerGallon()) + "\", ";
						jsonOut += "\"autoWaterChangeHour\":\"" + String(config.getAutoWaterChangeHour()) + "\", ";
						jsonOut += "\"autoWaterChangeMinutes\":\"" + String(config.getAutoWaterChangeMinutes()) + "\", ";
						jsonOut += "\"autoWaterChangeGallons\":\"" + String(config.getAutoWaterChangeGallons()) + "\", ";
						jsonOut += "\"autoTopOff\":\"" + String(config.isAutoTopOffEnabled()) + "\", ";
						jsonOut += "\"autoTopMinutes\":\"" + String(config.getAutoTopOffMinutes()) + "\", ";
						// Pinouts
						jsonOut += "\"pinOutlet1\":\"" + String(config.getPinOutlet1()) + "\", ";
						jsonOut += "\"pinOutlet2\":\"" + String(config.getPinOutlet2()) + "\", ";
						jsonOut += "\"pinOutlet3\":\"" + String(config.getPinOutlet3()) + "\", ";
						jsonOut += "\"pinOutlet4\":\"" + String(config.getPinOutlet4()) + "\", ";
						jsonOut += "\"pinOutlet5\":\"" + String(config.getPinOutlet5()) + "\", ";
						jsonOut += "\"pinOutlet6\":\"" + String(config.getPinOutlet6()) + "\", ";
						jsonOut += "\"pinOutlet7\":\"" + String(config.getPinOutlet7()) + "\", ";
						jsonOut += "\"pinOutlet8\":\"" + String(config.getPinOutlet8()) + "\", ";
						jsonOut += "\"pinRodiAquariumSolenoid\":\"" + String(config.getPinRodiAquariumSolenoid()) + "\", ";
						jsonOut += "\"pinRodiReservoirSolenoid\":\"" + String(config.getPinRodiReservoirSolenoid()) + "\", ";
						jsonOut += "\"pinAquariumDrainSolenoid\":\"" + String(config.getPinAquariumDrainSolenoid()) + "\", ";
						jsonOut += "\"pinUpperFloatValve\":\"" + String(config.getPinReservoirUpperFloatValve()) + "\", ";
						jsonOut += "\"pinLowerFloatValve\":\"" + String(config.getPinReservoirLowerFloatValve()) + "\" ";
					jsonOut += "}";
				}
				// /reset
				else if (strncmp(resource, "reset", 5) == 0) {
					factoryReset();
				}
				// /reload
				else if (strncmp(resource, "reload", 6) == 0) {
					config.load();
				}
				else {
					send404();
					break;
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
	checkLowerReservoirLevel();
}

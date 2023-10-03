#include <Arduino.h>
#include <ArtnetWifi.h>
#include <Esp.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <HardwareSerial.h>
#include <include/wl_definitions.h>
#include <IPAddress.h>
#include <pins_arduino.h>
#include <cstdint>
#include <ArduinoOTA.h>
#include <vector>
#include "defines.h"
#ifdef IR_ENABLE
#include <IRremoteESP8266.h>
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>

#include "Log.h"
#include "LEDMatrix.h"
#include "App.h"
#include "VisualizerApp.h"
#include "FadeAndScrollVisualizer.h"
#include "FireVisualizer.h"
#include "DemoReelVisualizer.h"
#include "VUMeterVisualizer.h"
#include "NoiseWithPaletteVisualizer.h"
#include "GlowVisualizer.h"
#include "Runnable.h"
#include "TurnOnRunnable.h"
#include "TurnOffRunnable.h"

ESP8266WebServer server(80);

WebSocketsServer webSocket = WebSocketsServer(81);
uint8_t socketNumber = 0;
unsigned long messageNumber;

LEDMatrix matrix(LEDS_WIDTH, LEDS_HEIGHT);

TurnOnRunnable turnOnRunnable(&matrix);
TurnOffRunnable turnOffRunnable(&matrix);

ArtnetWifi artnet;

#ifdef IR_ENABLE
IRrecv irrecv(PIN_RECV_IR);

long irCodes[] = { 0xff3ac5, 0xffba45, 0xff827d, 0xff02fd, 0xff1ae5, 0xff9a65,
		0xffa25d, 0xff22dd, 0xff2ad5, 0xffaa55, 0xff926d, 0xff12ed, 0xff0af5,
		0xff8a75, 0xffb24d, 0xff32cd, 0xff38c7, 0xffb847, 0xff7887, 0xfff807,
		0xff18e7, 0xff9867, 0xff58a7, 0xffd827, 0xff28d7, 0xffa857, 0xff6897,
		0xffe817, 0xff08f7, 0xff8877, 0xff48b7, 0xffc837, 0xff30cf, 0xffb04f,
		0xff708f, 0xfff00f, 0xff10ef, 0xff906f, 0xff50af, 0xffd02f, 0xff20df,
		0xffa05f, 0xff609f, 0xffe01f };
#endif

boolean isOn = false;
boolean isPlaying = true;

VisualizerApp* pCurrentApp = NULL;
VisualizerApp* pVisApp1;
VisualizerApp* pVisApp2;
VisualizerApp* pVisApp3;
VisualizerApp* pVisApp4;
VisualizerApp* pVisApp5;
VisualizerApp* pVisApp6;


Runnable* pCurrentRunnable = NULL;

int sensorPin = A0;
long sensorValue = 0;

long lmillis = 0;
uint8_t lastButton = 0;
float brightness = 1.0f;

String lampHostname;
float calibRed = 1.0f;
float calibGreen = 1.0f;
float calibBlue = 1.0f;

float val = 0.0f;
float maxVal = 0.0f;
float minVal = 2.0f;
float curVal = 0.0f;

long lastSensorBurstRead = 0;
long lastMillis = 0;

CRGB solidColor = CRGB::Wheat;

void onButton(uint8_t btn);
void onColor(int color1Rgb, int color2Rgb, int color3Rgb);
void update();
void switchApp(VisualizerApp* pApp);
void writeConfiguration();

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence,
		uint8_t* data) {

	int pixel = 0;
	uint16_t x = 0, y = 0;
	for (int i = 0; i < length; i += 3) {
		matrix.setPixel(x, y, data[i], data[i + 1], data[i + 2]);
		if (++x >= LEDS_WIDTH) {
			++y;
			x = 0;
		}
	}

	matrix.update();
}

void connectToWiFi() {
	WiFi.hostname(lampHostname);
	WiFi.mode(WIFI_STA);
	WiFi.begin();
	ESP.wdtDisable();
	WiFiManager wifiManager;
	wifiManager.setDebugOutput(false); // without this WDT restarts ESP
	wifiManager.autoConnect("SmartLEDLampAP");
	ESP.wdtEnable(0);

	if (WiFi.isConnected()) {
		Logger.info("IP: %s", WiFi.localIP().toString().c_str());
	}
}

void handleAction() {
	String act = server.arg("act");
	if (act.length() != 0) {
		if (act == "off" && isOn) {
			turnOffRunnable.init();
			pCurrentRunnable = &turnOffRunnable;
			update();
		} else if (act == "on" && !isOn) {
			turnOnRunnable.init();
			pCurrentRunnable = &turnOnRunnable;
			update();
		}
	}

	String btn = server.arg("btn");
	if (btn.length() != 0) {
		long btnNo = btn.toInt();

		if (btnNo >= 0) {
			onButton((uint8_t) btnNo);
		}
	}

	String colorParameter = server.arg("color");
	if (colorParameter.length() != 0) {
		int color1Offset = colorParameter.indexOf(",");
		int color2Offset = colorParameter.indexOf(",", color1Offset + 1);

		int color1 = colorParameter.substring(0, color1Offset).toInt();
		int color2 = colorParameter.substring(color1Offset + 1, color2Offset).toInt();
		int color3 = colorParameter.substring(color2Offset + 1, colorParameter.length()).toInt();

		Logger.debug("Received Color 1 '%i'", color1);
		Logger.debug("Received Color 2 '%i'", color2);
		Logger.debug("Received Color 3 '%i'", color3);
		onColor(color1, color2, color3);
	}

	String brightness = server.arg("brightness");
	if (brightness.length() != 0) {
		long brightnessValue = brightness.toInt();

		if (brightnessValue >= 1 && brightnessValue <= 100) {
			matrix.setBrightness((float) (brightnessValue / 100.0));
			update();
		}
	}


	String buttonJson = "";
	String mode = "Solid";
	if (pCurrentApp) {
		std::vector<ButtonMapping> buttonMappings = pCurrentApp->getButtonMappings();

		if (!buttonMappings.empty()) {
			for (int i = 0; i < buttonMappings.size(); i++) {
				ButtonMapping mapping = buttonMappings.at(i);
				buttonJson.concat("{\"function\":\""+ mapping.getButtonIdentifier() + "\", \"label\":\"" + mapping.functionName + "\"}");

				if (i + 1 < buttonMappings.size()) {
					buttonJson.concat(",");
				}
			}
		}

		mode = pCurrentApp->getName();
	}

	String payload = String(
			"{ \"mode\": \"" + mode + "\", \"brightness\":\"" + String(matrix.getBrightness()) + "\", \"buttons\":[" + buttonJson + "]}");

	server.send(200, "text/plain", payload);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload,
		size_t lenght) {
	socketNumber = num;

	switch (type) {
	case WStype_DISCONNECTED:
		break;
	case WStype_CONNECTED:
		webSocket.sendTXT(num, "Connected");
		break;
	case WStype_TEXT:
		if (payload[0] == '!') {
			Logger.debug("Received RPC '%s'", &payload[1]);

			char* token = strtok((char*) &payload[1], " ");

			if (strcmp("setCalibration", token) == 0) {
				token = strtok(NULL, " ");
				double red = strtod(token, NULL);
				token = strtok(NULL, " ");
				double green = strtod(token, NULL);
				token = strtok(NULL, " ");
				double blue = strtod(token, NULL);

				calibRed = red;
				calibGreen = green;
				calibBlue = blue;

				Logger.debug("Calibrating to %f, %f, %f", red, green, blue);
				matrix.setCalibration(calibRed, calibGreen, calibBlue);
			} else if (strcmp("setHostname", token) == 0) {
				token = strtok(NULL, " ");
				lampHostname = token;
				if (lampHostname.length() > 31)
					lampHostname = lampHostname.substring(0, 31);
				lampHostname.trim();

				Logger.debug("Setting hostname to %s", lampHostname.c_str());
			} else if (strcmp("saveConfiguration", token) == 0) {
				writeConfiguration();
				Logger.debug("Configuration saved");
			} else if (strcmp("getConfiguration", token) == 0) {
				String ret = "<getConfiguration " + lampHostname + " "
						+ calibRed + " " + calibGreen + " " + calibBlue;
				webSocket.sendTXT(num, ret.c_str());
			}

			update();
		}
		break;
	}
}

void startWebServer() {
	server.on("/action/", handleAction);
	server.begin();

	server.serveStatic("/resources", SPIFFS, "/resources", "max-age=86400");
	server.serveStatic("/", SPIFFS, "/control.html");
	server.serveStatic("/config.html", SPIFFS, "/config.html");

	Logger.info("HTTP server started");

	webSocket.onEvent(webSocketEvent);
	webSocket.begin();

	Logger.info("WebSocket server started");
}

void setupOTA() {
	ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname(lampHostname.c_str());

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR)
		Serial.println("Auth Failed");
		else
		if (error == OTA_BEGIN_ERROR)
		Serial.println("Begin Failed");
		else
		if (error == OTA_CONNECT_ERROR)
		Serial.println("Connect Failed");
		else
		if (error == OTA_RECEIVE_ERROR)
		Serial.println("Receive Failed");
		else
		if (error == OTA_END_ERROR)
		Serial.println("End Failed");
	});
	ArduinoOTA.begin();
}

void switchApp(VisualizerApp* pApp) {
	if (pCurrentApp) {
		pCurrentApp->stop();
	}
	if (pApp) {
		pApp->start();
	}

	pCurrentApp = pApp;

	update();
}

void readConfiguration() {
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) {
		Logger.info("No config file found, using defaults");
		return;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Logger.error("Config file size is too large");
		return;
	}

	std::unique_ptr<char[]> buf(new char[size]);
	configFile.readBytes(buf.get(), size);

	StaticJsonDocument<200> jsonBuffer;
	auto success = deserializeJson(jsonBuffer, buf.get());

	if (!success) {
		Logger.error("Failed to parse config file");
		return;
	}

	lampHostname = jsonBuffer["hostname"].as<String>();

	if (!lampHostname || !lampHostname.length()) {
		char host[15];
		sprintf(host, "LEDLamp-%06x", ESP.getChipId());
		lampHostname = host;
		Logger.info("No hostname set, defaulting to %s", lampHostname.c_str());
	} else {
		Logger.info("Hostname is %s", lampHostname.c_str());
	}

	calibRed = jsonBuffer["calibration"]["red"];
	calibGreen = jsonBuffer["calibration"]["green"];
	calibBlue = jsonBuffer["calibration"]["blue"];

	configFile.close();
}

void writeConfiguration() {
	File configFile = SPIFFS.open("/config.json", "w");

	StaticJsonDocument<200> doc;
	
	if (lampHostname)
		doc["hostname"] = lampHostname;

	doc.createNestedObject("calibration");
	doc["calibration"]["red"] = calibRed;
	doc["calibration"]["green"] = calibGreen;
	doc["calibration"]["blue"] = calibBlue;

	serializeJsonPretty(doc, configFile);

	configFile.close();
}

void setup() {
	// Turn off blue status LED
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);

	delay(300);

	Logger.begin();
	Serial.println("\n\n\n\n");
	Logger.info("Starting Smart LED Lamp");
	Logger.info("Free Sketch Space: %i", ESP.getFreeSketchSpace());

	SPIFFS.begin();

	matrix.init();
	matrix.clear();

	readConfiguration();

	matrix.setCalibration(calibRed, calibGreen, calibBlue);

	connectToWiFi();
	setupOTA();

	matrix.clear();

	startWebServer();

	artnet.setArtDmxCallback(onDmxFrame);
	artnet.begin();

	matrix.clear();

	pVisApp1 = new VisualizerApp(&matrix, "Fade");
	pVisApp1->setVisualizer(0, new FadeAndScrollVisualizer(1, 20));

	pVisApp2 = new VisualizerApp(&matrix, "Fire");
	pVisApp2->setVisualizer(0, new FireVisualizer());

	pVisApp3 = new VisualizerApp(&matrix, "Demo");
	pVisApp3->setVisualizer(0, new DemoReelVisualizer());

	pVisApp4 = new VisualizerApp(&matrix, "VUMeter");
	pVisApp4->setVisualizer(0, new VUMeterVisualizer(&curVal));

	pVisApp5 = new VisualizerApp(&matrix, "Noise");
	pVisApp5->setVisualizer(0, new NoiseWithPaletteVisualizer());

	pVisApp6 = new VisualizerApp(&matrix, "Glow");
	pVisApp6->setVisualizer(0, new GlowVisualizer());


	switchApp(NULL);

#ifdef IR_ENABLE
	irrecv.enableIRIn(); // Start the receiver
#endif

	update();

	pinMode(sensorPin, INPUT);
}

void update() {
	if (isOn) {
		if (pCurrentApp) {
			pCurrentApp->update();
		} else {
			matrix.fill(solidColor);
		}
	} else {
		matrix.clear();
	}
	matrix.update();
}

void onButton(uint8_t btn) {
	Logger.debug("Button '%i' pressed", btn);

	if (btn == BTN_POWER) {
		if (!isOn) {
			turnOnRunnable.init();
			pCurrentRunnable = &turnOnRunnable;
		} else {
			turnOffRunnable.init();
			pCurrentRunnable = &turnOffRunnable;
		}
		update();
	}

	if (!isOn) {
		return;
	}

	if (pCurrentApp) {
		if (pCurrentApp->onButtonPressed(btn))
			return;
	}

	switch (btn) {
	case BTN_BRIGHTER:
		brightness += 0.05f;
		if (brightness > 1.0f)
			brightness = 1.0f;
		matrix.setBrightness(brightness);
		matrix.update();
		break;
	case BTN_DARKER:
		brightness -= 0.05f;
		if (brightness < 0.05f)
			brightness = 0.05f;
		matrix.setBrightness(brightness);
		matrix.update();
		break;
	case BTN_PAUSE:
		isPlaying = !isPlaying;
		delay(200);
		break;
	case BTN_FLASH:
		switchApp(pVisApp5);
		break;
	case BTN_JUMP3:
		switchApp(pVisApp1);
		break;
	case BTN_JUMP7:
		switchApp(pVisApp2);
		break;
	case BTN_FADE3:
		switchApp(pVisApp3);
		break;
	case BTN_FADE7:
		//switchApp(pVisApp4);
		switchApp(pVisApp6);
		break;
	case BTN_SOLID:
		switchApp(NULL);
		break;
	}

}

void onColor(int color1Hsv, int color2Hsv, int color3Hsv) {
	if (pCurrentApp) {
		pCurrentApp->setColor(color1Hsv * 0.698, color2Hsv * 0.698, color3Hsv * 0.698);
	}
	else {
		CHSV hsv = CHSV(color1Hsv * 0.698, 255, 255);
		CRGB rgb;
		hsv2rgb_rainbow(hsv, rgb);
		solidColor = rgb;
		matrix.fill(solidColor);
	}
}

void readAnalogPeek() {
	maxVal = -10.0f;
	minVal = 10.0f;
//	String message = "# " + String(messageNumber) + " ";
	for (int i = 0; i < 300; ++i) {
		sensorValue = analogRead(sensorPin);
//		if (i & 1) {
//			message = message + String(sensorValue) + ";";
//		}
		float newVal = sensorValue / 1024.0f;
		if (newVal > maxVal)
			maxVal = newVal;

		if (newVal < minVal)
			minVal = newVal;

		if (maxVal - minVal > curVal) {
			curVal = maxVal - minVal;
		}
	}
//	message[message.length() - 1] = '\0';
//	webSocket.sendTXT(socketNumber, message);
	lastSensorBurstRead = millis();
}

void loop() {
	long currentMillis = millis();

//	if (currentMillis > lastSensorBurstRead + 6 /*5*/) {
//		readAnalogPeek();
//	}

	ArduinoOTA.handle();
	server.handleClient();
	webSocket.loop();
	Logger.loop();

	if (pCurrentRunnable) {
		pCurrentRunnable->run();
		update();
	}

	if (isOn && isPlaying) {
		if (pCurrentApp) {
			pCurrentApp->run();
		} else {
			artnet.read();
		}
	}

#ifdef IR_ENABLE
	decode_results results;

	if (currentMillis > lmillis + 200) {
		lastButton = 0;
	}

	if (irrecv.decode(&results)) {
		if (results.value == 0xffffffff && lastButton != BTN_POWER) {
			lmillis = currentMillis;
		} else if ((results.value & 0xff000000) == 0) {
			for (int i = 0; i < sizeof(irCodes) / sizeof(irCodes[0]); ++i) {
				if (irCodes[i] == results.value) {
					lastButton = i + 1;
					lmillis = currentMillis;
					break;
				}
			}
		}
		if (lastButton)
			onButton(lastButton);
		irrecv.resume(); // Receive the next value
	}
#endif
}

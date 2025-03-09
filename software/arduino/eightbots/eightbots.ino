#include <Wire.h>
#include <SPI.h>
#include <GRGB.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#include "pin_config.h"

GRGB led(COMMON_ANODE, PIN_LED_R, PIN_LED_G, PIN_LED_B);
Ticker led_ticker;
Ticker motor_ticker;
ESP32Encoder encoder;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void print_aa_logo(void){
	Serial.println();
	Serial.println(F(" ______ _       _     _   _           _       "));
	Serial.println(F("|  ____(_)     | |   | | | |         | |      "));
	Serial.println(F("| |__   _  __ _| |__ | |_| |__   ___ | |_ ___ "));
	Serial.println(F("|  __| | |/ _` | '_ \\| __| '_ \\ / _ \\| __/ __|"));
	Serial.println(F("| |____| | (_| | | | | |_| |_) | (_) | |_\\__ \\"));
	Serial.println(F("|______|_|\\__, |_| |_|\\__|_.__/ \\___/ \\__|___/"));
	Serial.println(F("           __/ |                              "));
	Serial.println(F("          |___/                               "));
	Serial.println();
}

void configure_wifi(void){
	Serial.println(F("Configuring WiFi..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!");
		while(1); // TODO: Try to reconnect using a timer
	}

  Serial.print("WiFi Ready. IP address: ");
  Serial.println(WiFi.localIP());


  digitalWrite(PIN_BUZZER, HIGH);
  delay(500);
  digitalWrite(PIN_BUZZER, LOW);
}

void configure_ota(void){
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();

}


void configure_oled(void){
	Serial.println(F("Configuring OLED..."));
	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
		while(1);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
	display.display();
  display.setRotation(0);
}

void configure_rotary_encoder(void){
	Serial.println(F("Configuring rotary encoder..."));
	ESP32Encoder::useInternalWeakPullResistors = puType::up;
	pinMode(PIN_ROT_SW, INPUT_PULLUP);
	encoder.attachHalfQuad(PIN_ROT_A, PIN_ROT_B);
	encoder.clearCount();
}

void configure_buzzer(void){
	Serial.println(F("Configuring buzzer..."));
	pinMode(PIN_BUZZER, OUTPUT);
	digitalWrite(PIN_BUZZER, LOW);

	for (int i = 0; i < 2; i++){
		digitalWrite(PIN_BUZZER, HIGH);
		delay(30);
		digitalWrite(PIN_BUZZER, LOW);
		delay(30);
	}
}

void configure_led(void){
	Serial.println(F("Configuring LED..."));
	led.setBrightness(255);

	for(uint8_t i=0; i<255; i++){
		led.setWheel8(i);
		delay(10);
	}

	led.setBrightness(255);
	led_ticker.attach_ms(500, toggle_led);
}

void configure_motor(void){
  Serial.println(F("Configuring motor..."));
  pinMode(PIN_MOTOR_AIN1, OUTPUT);
  pinMode(PIN_MOTOR_AIN2, OUTPUT);
  pinMode(PIN_MOTOR_BIN1, OUTPUT);
  pinMode(PIN_MOTOR_BIN2, OUTPUT);

  // Stop the motor
  digitalWrite(PIN_MOTOR_AIN1, HIGH);
  digitalWrite(PIN_MOTOR_AIN2, HIGH);
  digitalWrite(PIN_MOTOR_BIN1, HIGH);
  digitalWrite(PIN_MOTOR_BIN2, HIGH);
  motor_ticker.attach_ms(100, change_motor_speed);
}

void toggle_led(void){
	static bool led_state = false;
	led_state = !led_state;

	if(led_state) led.setRGB(0, 0, 0);
	else led.setRGB(255, 255 , 255);
}

void change_motor_speed(){
  static float x = 0;
  // sin wave
  x += 0.1;
  int dutyCycle = 256 * sin(x);
  if (dutyCycle<0){
    analogWrite(PIN_MOTOR_AIN1, abs(dutyCycle));
    analogWrite(PIN_MOTOR_AIN2, 0);
    analogWrite(PIN_MOTOR_BIN1, abs(dutyCycle));
    analogWrite(PIN_MOTOR_BIN2, 0);
  }else{
    analogWrite(PIN_MOTOR_AIN1, 0);
    analogWrite(PIN_MOTOR_AIN2, dutyCycle);
    analogWrite(PIN_MOTOR_BIN1, 0);
    analogWrite(PIN_MOTOR_BIN2, dutyCycle);
  }
}

void setup() {
  Serial.begin(115200);
	print_aa_logo();
	Serial.println(F("Starting..."));
	configure_buzzer();
	configure_led();
	configure_rotary_encoder();

	Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

	configure_oled();
  configure_mpu();

  configure_wifi();
	configure_ota();

  configure_motor();
}

void loop() {
  ArduinoOTA.handle();
	Serial.print("count: ");
	Serial.print(encoder.getCount());
	Serial.print(" SW: ");
	Serial.print(digitalRead(PIN_ROT_SW));
	Serial.println();
	delay(50);
}



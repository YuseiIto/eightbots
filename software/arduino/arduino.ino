#include <Wire.h>
#include <SPI.h>
#include <GRGB.h>
#include <Ticker.h>
#include "pin_config.h"

GRGB led(COMMON_ANODE, PIN_LED_R, PIN_LED_G, PIN_LED_B);
Ticker led_ticker;

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

void toggle_led(void){
	static bool led_state = false;
	led_state = !led_state;

	if(led_state) led.setRGB(0, 0, 0);
	else led.setRGB(255, 255 , 255);
}

void setup() {
  Serial.begin(115200);
	print_aa_logo();
	Serial.println(F("Starting..."));
	configure_buzzer();
	configure_led();
}

void loop() {
	Serial.println(F("Hello World!"));
	delay(2000);
}


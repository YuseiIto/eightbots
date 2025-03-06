#include <Wire.h>
#include <SPI.h>
#include "pin_config.h"


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

void setup() {
  Serial.begin(115200);
	print_aa_logo();
	Serial.println(F("Starting..."));
	configure_buzzer();
}

void loop() {
  Serial.println();
	Serial.println(F("Hello World!"));
	delay(2000);
}


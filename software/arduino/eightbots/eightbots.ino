#include <Wire.h>
#include <SPI.h>
#include <GRGB.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MPU6050_6Axis_MotionApps20.h"
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

MPU6050 mpu;

/*---MPU6050 Control/Status Variables---*/
bool DMPReady = false;  // Set true if DMP init was successful
uint8_t FIFOBuffer[64]; // FIFO storage buffer

/*------Interrupt detection routine------*/
volatile bool MPUInterrupt = false;     // Indicates whether MPU6050 interrupt pin has gone high
void DMPDataReady() {
  MPUInterrupt = true;
}


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

  const int resolution = 8; // bits
  analogWriteResolution(PIN_MOTOR_AIN1, resolution);
  analogWriteResolution(PIN_MOTOR_AIN2, resolution);
  analogWriteResolution(PIN_MOTOR_BIN1, resolution);
  analogWriteResolution(PIN_MOTOR_BIN2, resolution);

  const int frequency = 50000; // Hz
  analogWriteFrequency(PIN_MOTOR_AIN1, frequency); 
  analogWriteFrequency(PIN_MOTOR_AIN2, frequency);
  analogWriteFrequency(PIN_MOTOR_BIN1, frequency);
  analogWriteFrequency(PIN_MOTOR_BIN2, frequency);

  // Stop the motor
  digitalWrite(PIN_MOTOR_AIN1, HIGH);
  digitalWrite(PIN_MOTOR_AIN2, HIGH);
  digitalWrite(PIN_MOTOR_BIN1, HIGH);
  digitalWrite(PIN_MOTOR_BIN2, HIGH);
  motor_ticker.attach_ms(100, change_motor_speed);
}


void configure_mpu(void){
  Serial.println(F("Configuring MPU6050..."));
  mpu.initialize();
  pinMode(PIN_MPU_INT, INPUT);

  if(mpu.testConnection() == false){
    Serial.println(F("MPU6050 connection failed"));
    while(true);
  }

  uint8_t devStatus = mpu.dmpInitialize();   // Return status after each device operation (0 = success, !0 = error)
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);

  if (devStatus == 0) {
    mpu.CalibrateAccel(6);  // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateGyro(6);
    Serial.println("These are the Active offsets: ");
    mpu.PrintActiveOffsets();
    Serial.println(F("Enabling DMP..."));   //Turning ON DMP
    mpu.setDMPEnabled(true);

    /*Enable Arduino interrupt detection*/
    Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
    Serial.print(digitalPinToInterrupt(PIN_MPU_INT));
    Serial.println(F(")..."));
    attachInterrupt(digitalPinToInterrupt(PIN_MPU_INT), DMPDataReady, RISING);

    /* Set the DMP Ready flag so the main loop() function knows it is okay to use it */
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    DMPReady = true;
  } 
  else {
    Serial.print(F("DMP Initialization failed (code ")); //Print the error code    Serial.print(devStatus);
    Serial.println(F(")"));
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
  }
}


void toggle_led(void){
	static bool led_state = false;
	led_state = !led_state;

	if(led_state) led.setRGB(0, 0, 0);
	else led.setRGB(255, 255 , 255);
}

double travel = 0; // positive: forward, negative: backward
double rotation = 0; // positive: right, negative: left

void change_motor_speed(){
  double l = travel-rotation;
  uint8_t lduty = min(abs(l),1.0)*255;
  if (l<0){
    analogWrite(PIN_MOTOR_AIN1, lduty);
    analogWrite(PIN_MOTOR_AIN2, 0);
  }else{
    analogWrite(PIN_MOTOR_AIN1, 0);
    analogWrite(PIN_MOTOR_AIN2, lduty);
  }

  double r = travel+rotation;
  uint8_t rduty = min(abs(r),1.0)*255;
  if (r<0){
    analogWrite(PIN_MOTOR_BIN1, rduty);
    analogWrite(PIN_MOTOR_BIN2, 0);
  }else{
    analogWrite(PIN_MOTOR_BIN1, 0);
    analogWrite(PIN_MOTOR_BIN2, rduty);
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

  if (!DMPReady) return;
  if (mpu.dmpGetCurrentFIFOPacket(FIFOBuffer)) {
    Quaternion q;           // [w, x, y, z]         Quaternion container
    float ypr[3];           // [yaw, pitch, roll]   Yaw/Pitch/Roll container and gravity vector
    VectorFloat gravity;    // [x, y, z]            Gravity vector

    mpu.dmpGetQuaternion(&q, FIFOBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Yaw(rad): ");
    display.println(ypr[0], 1);

    rotation = ypr[0]* 1.05/3.14;
    display.display();
  }

}



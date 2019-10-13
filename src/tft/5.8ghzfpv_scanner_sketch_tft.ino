/*
This file is part of OLED 5.8ghz Scanner project.

    OLED 5.8ghz Scanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OLED 5.8ghz Scanner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

    Copyright © 2016 Michele Martinelli
  */

#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Adafruit_ST7789.h> // TFT driver library by Adafruit
#include <SPI.h>
#include <stdarg.h>
#include <Wire.h>
#include <EEPROM.h>
#include "rx5808.h" // receiver library
#include "const.h" // file with constants - variables are defined here

// enable SPI display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); // CS pin is not actually used, so is lost.

// enable rx5808
RX5808 rx5808(rssi_pin);

byte data0 = 0;
byte data1 = 0;
byte data2 = 0;
byte data3 = 0;

uint8_t curr_status = SCANNER_MODE;
uint8_t curr_screen = 6;
uint8_t curr_channel = 0;
uint32_t curr_freq;

//battery monitor
int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float volt = 0.0;               // calculated voltage
uint16_t VoltReference = 4.568; // voltage on arduino 5V pin.
uint16_t VoltDivider = 11.393; // input voltage from battery / outputvoltage to adc - ex: 10.02v/0.9v=11.133v

#ifdef BUTTONS
  //default values used for calibration
  uint16_t rssi_min = 1024;
  uint16_t rssi_max = 0;
  
  bool exec_receiver = false;
  bool exec_scanner = false;
  bool first_summary = true;
  
  uint32_t last_irq;
  uint8_t changing_freq, changing_mode;
  
  void irq_select_handle() {
    if (millis() - last_irq < 500) //debounce
      return;
  
    if (curr_status == SCANNER_MODE && digitalRead(button_select) == LOW) { //simply switch to the next screen
      curr_screen = (curr_screen + 1) % 7;
      Serial.println("Select button pressed");
      tft.fillScreen(BLACK);
    }
  
    if (curr_status == RECEIVER_MODE) {
      if (digitalRead(button_select) == HIGH && millis() - last_irq > 800) { //long press to reach the next strong cannel
        curr_channel = rx5808.getNext(curr_channel);
        Serial.println("Select button long pressed");
      } else {
        curr_channel = (curr_channel + 1) % CHANNEL_MAX;
        Serial.println("Select button pressed");
        //tft.fillScreen(BLACK);
      }
      changing_freq = 1;
    }
  
    rx5808.abortScan();
    last_irq = millis();
  }
  
  void irq_mode_handle() {
    if (millis() - last_irq < 500) //debounce
      return;
  
    if (digitalRead(button_mode) == LOW) {
      Serial.println("Mode button pressed");
      rx5808.abortScan();
      changing_mode = 1;
  
      if (curr_status == RECEIVER_MODE) {
        curr_status = SCANNER_MODE;
        exec_scanner = true;
      } else {
        curr_status = RECEIVER_MODE;
        exec_receiver = true;
        curr_channel = rx5808.getMaxPos(); //next channel is the strongest
      }
    }
    last_irq = millis();
  }
  
  /*void irq_rssi_handle() { // no IRQ for now on the NANO. Maybe use STM.
    if (millis() - last_irq < 500) //debounce
      return;
  
    if (digitalRead(button_rssi) == LOW) {
      Serial.println("RSSI button pressed");
      rx5808.abortScan();
      changing_mode = 1;
      curr_status = RSSI_MODE;
      u8g.setDefaultForegroundColor();
      calibrationScr();
      rx5808.calibration();
      delay(500); // delay after calibration
      curr_status = SCANNER_MODE; // back to scanner mode
    }
    last_irq = millis();
  }*/ // disabled for now
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("Setup start");

  //button init
  pinMode(button_select, INPUT);
  digitalWrite(button_select, HIGH);
  pinMode(button_mode, INPUT);
  digitalWrite(button_mode, HIGH);
  //pinMode(button_rssi, INPUT);
  //digitalWrite(button_rssi, HIGH);

  //tft initialization
  tft.init(240, 240, SPI_MODE2); //initialize a ST7789 chip, 240x240 pixels (Adafruit). SPI_MODE2 for no CS pin, SPI_MODE0 for CS pin.
  tft.setRotation(TFT_ROTATION);
  
  tft.fillScreen(BLACK);
  tftsplashScr(); // TFT
  delay(500); // show splash screen for 0.5 second

  // initialize SPI:
  pinMode (SSP, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  rx5808.init();

  //power on calibration if button pressed
  while (digitalRead(button_select) == LOW || digitalRead(button_mode) == LOW) {
    tftcalibrationScr();
    rx5808.calibration();
    delay(500);
  }

  /* // disabled for now to save EEPROM writes
  //always calibrate on power on / reset
  tftcalibrationScr();
  rx5808.calibration();
  delay(500); // delay after calibration
  */ 

  //receiver init
  curr_channel = rx5808.getMaxPos();
  changing_freq = 1;
  changing_mode = 0;

  #ifdef BUTTONS
    //rock&roll
    attachInterrupt(digitalPinToInterrupt(button_select), irq_select_handle, CHANGE);
    attachInterrupt(digitalPinToInterrupt(button_mode), irq_mode_handle, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(button_rssi), irq_rssi_handle, CHANGE);
  #endif

  tft.fillScreen(BLACK);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE, BLACK);
}

void loop(void) {
  battery_measure();
//  irq_rssi_handle(); // disabled for now because of no IRQ

  if (curr_status == SCANNER_MODE) {
    rx5808.scan(1, BIN_H);
  }

  if (changing_mode) { //if changing mode "please wait..."
    tft.fillScreen(BLACK);
    tft.fillScreen(BLACK);
  } else {
    if (curr_status == RECEIVER_MODE) {
      if (exec_receiver == true)
      {
        tft.fillScreen(BLACK);
        tft.fillScreen(BLACK);
        exec_receiver = false;
      }
      receiver_draw(curr_channel);
    }
    if (curr_status == SCANNER_MODE) {
      if (exec_scanner == true)
      {
        tft.fillScreen(BLACK);
        tft.fillScreen(BLACK);
        exec_scanner = false;
      }
      scanner_draw(curr_screen);
    }
  }

  //auto select best channel
  if (curr_status == RECEIVER_MODE && changing_freq || changing_mode) {
    changing_freq = changing_mode = 0;
    rx5808.scan(1, BIN_H);
    curr_freq = pgm_read_word_near(channelFreqTable + curr_channel);
    rx5808.setFreq(curr_freq);
  }
}

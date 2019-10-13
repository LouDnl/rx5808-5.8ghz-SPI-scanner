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

#define FRAME_START_X 0
#define FRAME_START_Y 7

char buf[80];

void wait_draw() {
  tft.setCursor(FRAME_START_X, FRAME_START_Y);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print("please wait... ");
}

void receiver_draw( uint32_t channel) {

  //display voltage
  tft.setCursor(FRAME_START_X, FRAME_START_Y);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print("vbatt: ");
  tft.setCursor(FRAME_START_X + 84, FRAME_START_Y);
  tft.setTextColor(RED, BLACK);
  tft.print(volt);
  tft.print(" ");

  //display current freq and the next strong one
  tft.setCursor(FRAME_START_X, FRAME_START_Y + 20);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.println ("Current channel:");
  sprintf (buf, "[%x]%d:%d ", pgm_read_byte_near(channelNames + channel), pgm_read_word_near(channelFreqTable + channel), rx5808.getRssi(channel)); //frequency:RSSI
  tft.setTextSize(3);
  tft.setTextColor(YELLOW, BLACK);
  tft.println(buf);
  tft.setCursor(FRAME_START_X, FRAME_START_Y + 70);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.println("Next strong channel:");
  uint16_t next_chan = rx5808.getNext(channel);
  sprintf (buf, "[%x]%d:%d ", pgm_read_byte_near(channelNames + next_chan), pgm_read_word_near(channelFreqTable + next_chan), rx5808.getRssi(next_chan)); //frequency:RSSI
  tft.setTextSize(3);
  tft.setTextColor(YELLOW, BLACK);
  tft.println(buf);
  
  //histo below
  for (int i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
    uint8_t channelIndex = pgm_read_byte_near(channelList + i); //retrive the value based on the freq order
    //uint16_t rssi_lines = map(rx5808.getRssi(channelIndex), 1, BIN_H, 1, BIN_H * 4);
    uint16_t rssi_lines = map(rx5808.getRssi(channelIndex), 1, 40, 1, 100);
    tft.fillRect(1 + 5 * i, 230 - rssi_lines, 4, 100, BLACK);
    tft.fillRect(1 + 5 * i, 230 - rssi_lines, 4, rssi_lines, GREEN);
  }
}

//draw all the channels of a certain band
void scanner_draw(uint8_t band) {
  int i, x = 0, y = 1, offset = 8 * band;
  tft.setTextSize(3);
  tft.setTextColor(CYAN, BLACK);
  
  switch (band) {
    case 0:
      tft.setCursor(0, 10);
      tft.print("BAND A  ");
      break;
    case 1:
      tft.setCursor(0, 10);
      tft.print("BAND B  ");
      break;
    case 2:
      tft.setCursor(0, 10);
      tft.print("BAND E  ");      
      break;
    case 3:
      tft.setCursor(0, 10);
      tft.print("FATSHARK");      
      break;
    case 4:
      tft.setCursor(0, 10);
      tft.print("RACEBAND");      
      break;
    case 5:
      spectrum_draw();
      return;
    case 6:
      summary_draw();
      return;
  }

  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);

  //make a small histo down left and print the rssi information of the 8 channels of the band
  for (i = 0; i < 8; i++) {
    uint16_t rssi_histo = rx5808.getRssi(offset + i);
    tft.fillRect((FRAME_START_X + 60) + i * 15, (FRAME_START_Y + 210) - rssi_histo, 10, 40, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 15, (FRAME_START_Y + 210) - rssi_histo, 10, rssi_histo, GREEN);

    sprintf (buf, "%d:%d", pgm_read_word_near(channelFreqTable + offset + i), rx5808.getRssi(offset + i)); //frequency:RSSI
    tft.setCursor(FRAME_START_X + 30 + x, y * 20 + (FRAME_START_Y + 25));
    tft.print(buf);
    tft.print(" ");
    y++;
    
    if (i == 3) {
      x = 96;
      y = 1;
    }
  }

  tft.setTextColor(ORANGE, BLACK);

  //computation of the max value
  uint16_t chan = rx5808.getMaxPosBand(band);
  sprintf (buf, "MAX %d", pgm_read_word_near(channelFreqTable + chan));
  tft.setCursor(144, 7);
  tft.print(buf);

  //computation of the min value
  chan = rx5808.getMinPosBand(band);
  sprintf (buf, "MIN %d", pgm_read_word_near(channelFreqTable + chan));
  tft.setCursor(144, 23);
  tft.print(buf);
}


//draw based on the frequency order
void spectrum_draw() {
  tft.setTextSize(3);
  tft.setTextColor(CYAN, BLACK);
  tft.setCursor(0, 10);
  tft.print("SPECTRUM");

  for (int i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
    uint8_t channelIndex = pgm_read_byte_near(channelList + i); //retrive the value based on the freq order
    uint16_t rssi_spectrum = rx5808.getRssi(channelIndex);
    tft.fillRect(1 + 6 * i, 230 - rssi_spectrum, 4, 100, BLACK);
    tft.fillRect(1 + 6 * i, 230 - rssi_spectrum * 3, 4, rssi_spectrum * 3, GREEN);
  }

  //computation of the max value
  uint16_t chan = rx5808.getMaxPos();
  sprintf (buf, "%x:%d", pgm_read_byte_near(channelNames + chan), pgm_read_word_near(channelFreqTable + chan));
  tft.setCursor(0, 60);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);
  tft.print(buf);
  tft.setCursor(0, 40);
  tft.setTextColor(ORANGE, BLACK);
  tft.print("Max. strength chan:");
  
  //computation of the min value
  chan = rx5808.getMinPos();
  sprintf (buf, "%x:%d", pgm_read_byte_near(channelNames + chan), pgm_read_word_near(channelFreqTable + chan));
  tft.setCursor(0, 100);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);
  tft.print(buf);
  tft.setCursor(0, 80);
  tft.setTextColor(ORANGE, BLACK);
  tft.print("Min. strength chan:");
}

//only one screen to show all the channels
void summary_draw() {
  uint8_t i;
  tft.setCursor(FRAME_START_X, FRAME_START_Y);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.print("BAND");
  tft.setCursor(FRAME_START_X + 135, FRAME_START_Y);
  tft.print("FREE CH");
  
  tft.setCursor(FRAME_START_X + 65, FRAME_START_Y);
  tft.print(volt);
  tft.setCursor(FRAME_START_X + 113, FRAME_START_Y);
  tft.print("v");
  
  tft.setCursor(FRAME_START_X + 36, FRAME_START_Y + 40);
  tft.print("A");
  tft.setCursor(FRAME_START_X + 36, FRAME_START_Y + 60);
  tft.print("B");
  tft.setCursor(FRAME_START_X + 36, FRAME_START_Y + 80);
  tft.print("E");
  tft.setCursor(FRAME_START_X + 36, FRAME_START_Y + 100);
  tft.print("F");  
  tft.setCursor(FRAME_START_X + 36, FRAME_START_Y + 120);
  tft.print("C");

  tft.drawLine(55, 0, 55, 140, WHITE); //start
  tft.drawLine(130, 0, 130, 140, WHITE); //end

  #define START_BIN FRAME_START_X+29
  #define BIN_H_LITTLE 16
  #define START_BIN_Y 13

  //computation of the min value
  for (i = 0; i < 5; i++) {
    uint16_t chan = rx5808.getMinPosBand(i);
    sprintf (buf, "%x %d", pgm_read_byte_near(channelNames + chan), pgm_read_word_near(channelFreqTable + chan));
    tft.setCursor(FRAME_START_X + 135, FRAME_START_Y + 20 * i + 40);
    tft.print(buf);
  }

  for (i = 0; i < 8; i++) {
    uint8_t bin = rx5808.getVal(0, i, BIN_H_LITTLE);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 60 - 4) - bin, 4, 20, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 60 - 4) - bin, 4, bin, ORANGE);

    bin = rx5808.getVal(1, i, BIN_H_LITTLE);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 80 - 4) - bin, 4, 20, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 80 - 4) - bin, 4, bin, YELLOW);
    
    bin = rx5808.getVal(2, i, BIN_H_LITTLE);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 100 - 4) - bin, 4, 20, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 100 - 4) - bin, 4, bin, RED);

    bin = rx5808.getVal(3, i, BIN_H_LITTLE);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 120 - 4) - bin, 4, 20, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 120 - 4) - bin, 4, bin, GREEN);

    bin = rx5808.getVal(4, i, BIN_H_LITTLE);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 140 - 4) - bin, 4, 20, BLACK);
    tft.fillRect((FRAME_START_X + 60) + i * 8, (FRAME_START_Y + 140 - 4) - bin, 4, bin, CYAN);    
  }

  if (first_summary) {
    tft.fillScreen(BLACK);
    tft.fillScreen(BLACK);
    first_summary = false;
  }  
}

//initial spash screen
void tftsplashScr() {
  tft.fillScreen(BLACK);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print("5.8Ghz FPV scanner");
  tft.setTextColor(GREEN);
  tft.setCursor(0, 20);
  tft.println("for 240x240 IPS TFT");
  tft.setTextSize(1);
  tft.setTextColor(RED);
  tft.setCursor(0, 40);
  tft.println("edits by LouD");
  tft.setTextColor(BLUE);
  tft.setCursor(0, 60);
  tft.println("original code by mikym0use");
}

//calibration splash screen
void tftcalibrationScr() {
  tft.fillScreen(BLACK);
  tft.setTextColor(CYAN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(4, 100);
  tft.setTextWrap(false);
  tft.println("CALIBRATING");
}

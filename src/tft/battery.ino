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
#include "const.h"

void battery_measure() { //battery voltage measurement
  while (sample_count < NUM_SAMPLES) {
      sum += analogRead(Voltage);
      sample_count++;
      delay(1);
  }
  volt = ((((float)sum / (float)NUM_SAMPLES * VoltReference) / 1023.0) * VoltDivider);
  sample_count = 0;
  sum = 0;  
}

/*!
 *   @file RCWL_1X05.cpp
 * 
 *   ## Author
 *   Copyright (c) 2023 juh
 *   ## License
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 2.
 */

#include "RCWL_1X05.h"


RCWL_1X05::RCWL_1X05() {
  setTemperature(defaultTemperature);
}

bool RCWL_1X05::begin(TwoWire *myWire, uint8_t i2c_addr) {
  wire = myWire;
  address = i2c_addr;
  wire->beginTransmission(address); // test if sensor is responsive
  bool res = (wire->endTransmission() == 0);
  getRawOutput(); // it seems the sensor needs this after the communication test above, else it will return 0 on first measurement
  return res;
}

bool RCWL_1X05::begin() {
  return begin(&Wire, RCWL_1X05_ADDR);
}

void RCWL_1X05::setMode(RCWL_1X05_mode newMode) {
  mode = newMode;
  if (mode == continuous) {
    lastValue = 0;
    trigger();
  }
}

void RCWL_1X05::setTemperature(int16_t newTemperature) {
  temperature = newTemperature;
  speedOfSound = uint32_t(331500) + uint32_t(600) * uint32_t(temperature); // mm per second
}

void RCWL_1X05::setTimeout(unsigned long newTimeout) {
  timeout = newTimeout;
}

bool RCWL_1X05::trigger() {
  lastTrigger = millis();
  wire->beginTransmission(address);
  wire->write(0x01); // start ranging command
  return (wire->endTransmission() == 0);
}

uint32_t RCWL_1X05::getOutput() {
  if (filterOn) {
    return getFilteredOutput();
  } else {
    return getRawOutput();
  }
}

// raw output is in mm/1000 = µm = micrometers
uint32_t RCWL_1X05::getRawOutput() {
  wire->requestFrom(address, uint8_t(3));
  if (wire->available() == 3) {
    uint32_t res = wire->read();      // BYTE_H
    res = (res << 8) | wire->read();  // BYTE_M
    res = (res << 8) | wire->read();  // BYTE_L
    return res;
  } else {
    return 0;
  }
}

void RCWL_1X05::setFilter(bool on) {
  if (not filterOn and on) { // filter is being turned on
    filterBuffer[0] = minDistance; // make sure first few measurements will be (close to) the median
    filterBuffer[1] = maxDistance;
    filterBuffer[2] = minDistance;
    filterBuffer[3] = maxDistance;
    filterBuffer[4] = minDistance;
  }
  filterOn = on;
}

// return median of last 5 measurements
uint32_t RCWL_1X05::getFilteredOutput() {
  uint32_t res = getRawOutput();
  if ((res >= minDistance) and (res <= maxDistance)) { // only enter valid measurements into the filter
    filterBuffer[filterIndex++] = res;
    filterIndex %= 5;
  }
  uint32_t md = median(filterBuffer[0], filterBuffer[1], filterBuffer[2], filterBuffer[3], filterBuffer[4]);
  return (md > 0) ? md : res; // I think after we prefilled the buffer in setFilter(), this should not be neccessary anymore
}

// this code adapted from AsyncSonar.h
uint32_t RCWL_1X05::median(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
  return b < a ? d < c ? b < d ? a < e ? a < d ? e < d ? e : d
  : c < a ? c : a
  : e < d ? a < d ? a : d
  : c < e ? c : e
  : c < e ? b < c ? a < c ? a : c
  : e < b ? e : b
  : b < e ? a < e ? a : e
  : c < b ? c : b
  : b < c ? a < e ? a < c ? e < c ? e : c
  : d < a ? d : a
  : e < c ? a < c ? a : c
  : d < e ? d : e
  : d < e ? b < d ? a < d ? a : d
  : e < b ? e : b
  : b < e ? a < e ? a : e
  : d < b ? d : b
  : d < c ? a < d ? b < e ? b < d ? e < d ? e : d
  : c < b ? c : b
  : e < d ? b < d ? b : d
  : c < e ? c : e
  : c < e ? a < c ? b < c ? b : c
  : e < a ? e : a
  : a < e ? b < e ? b : e
  : c < a ? c : a
  : a < c ? b < e ? b < c ? e < c ? e : c
  : d < b ? d : b
  : e < c ? b < c ? b : c
  : d < e ? d : e
  : d < e ? a < d ? b < d ? b : d
  : e < a ? e : a
  : a < e ? b < e ? b : e
  : d < a ? d : a;
}

// returns new distance, both in mm, assumes distances below 5m
uint32_t RCWL_1X05::applyTemperatureCorrection(uint32_t oldDistance) {
  return (((oldDistance * 1000 * 100) / speedOfSoundUsedByI2Cmode) * speedOfSound / 1000 / 100);
}

uint32_t RCWL_1X05::read() {
  switch (mode) {
    case oneShot : {
      trigger();
      delay(timeout);
      return (applyTemperatureCorrection(getOutput() / 1000));
      //return getOutput();
      break;
    }
    case triggered : {
      return (applyTemperatureCorrection(getOutput() / 1000));
      break;
    }
    case continuous : {
      return lastValue;
      break;
    }
  }
}


bool RCWL_1X05::update() {
  if ((millis() - lastTrigger) > timeout) {
    lastValue = applyTemperatureCorrection(getOutput() / 1000);
    trigger(); // lastTrigger is updated in trigger()
    return true;
  } else {
    return false;
  }
}



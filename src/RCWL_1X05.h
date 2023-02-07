/*!
 * @file RCWL_1X05.h
 * @mainpage RCWL_1X05 library
 * @brief A library for the cheap ultrasonic distance sensors with combined
 * transmitter/receiver with 16mm (RCWL-1605) and 10mm (RCWL-1005) diameter.
 * Provides three modes, blocking one shot mode, triggered mode, and continuous
 * mode of operation (see @ref RCWL_1X05::setMode()). 
 * 
 * Currently only I2C mode is supported, UART/serial mode may be added in the 
 * future. The other two modes, GPIO and one-wire should work just like the
 * ubitiquous HC-SR04 sensors and can be used with libraries like 
 * [AsyncSonar](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar/blob/master/readme.en.md) and others.
 * 
 * Go to the RCWL_1X05 class documentation.
 * 
 * ## Author
 * Copyright (c) 2023 juh
 * 
 * Filter code adapted from [AsyncSonar libarary](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar)
 * by [Luis Llamas](https://github.com/luisllamasbinaburo/).
 * ## License
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 */

#ifndef RCWL_1X05_h
#define RCWL_1X05_h

#include <Wire.h>
#include <Arduino.h>

const uint8_t RCWL_1X05_ADDR = 0x57;
const int16_t defaultTemperature = 20; // Celsius
const unsigned long defaultTimeout = 100; // ms, minimum timeout according to "datasheet"
const uint32_t speedOfSoundUsedByI2Cmode = 343500; // mm/s at 20° Celsius. Note this is only an assumed value, the "datasheet" doesn't say what temperature it uses for conversion into mm




class RCWL_1X05
{

public:
  
  /*! 
   * Mode of operation, used by @ref RCWL_1X05::setMode()
   */
  enum RCWL_1X05_mode : uint8_t {
    oneShot,         //!< blocking one time measurement
    triggered,       //!< non blocking, manually initiated measurement
    continuous        //!< non blocking, automatic measurement, needs update()
  };

  /*!
   * @brief Constructor.
   */
  RCWL_1X05();
  
  /*!
   * @brief Call begin() first. Optionally, pass a I2C class other than Wire to
   * be used for communicating with the sensor.
   * @param myWire Object of type TwoWire. Needs to be initialized and ready
   * for transmission. We do not call TowWire.begin() in this code so that you 
   * can set your own pins etc. if needed.
   * @param i2c_addr Probably useless, as the sensor currently only supports the 
   * fixed address 0x57.
   * @returns true if communication with the sensor was successfully established.
   */
  bool begin(TwoWire *myWire, uint8_t i2c_addr = RCWL_1X05_ADDR);

  /*!
   * @brief Use begin() without parameters to use the standard Wire.h library
   * and the standard address 0x57 for communicating with the sensor. Your code
   * must call Wire.begin() before calling this.
   * @returns true if communication with the sensor was successfully established.
   */
  bool begin();
  
  /*!
   * @brief Choose mode of operation, default is RCWL_1X05::oneShot. Will determine what 
   * will happen when you call read().
   * @param newMode One of (see @ref RCWL_1X05_mode)
   * 
   * - RCWL_1X05::oneShot: read() will initiate a measurement, wait for its completion and 
   * return the measured value. Note that read() is blocking in this mode, it will not return before the measurement cycle is complete.
   * - RCWL_1X05::triggered: You'll need to manually initiate a measurement with trigger(),
   * after that your sketch is free to do something else. You can read() the
   * value after measurement has been completed, but it is your responsibility
   * to wait long enough before reading  (see @ref setTimeout()).
   * - RCWL_1X05::continuous: Measurements will be triggered automatically after the previous
   * measurement has completed. However, this happens not automatically (no 
   * interrupt magic, here), instead your code needs to call update() as often as 
   * possible in this mode. read() will return the most recent completed 
   * measurement, so depending on timeout and your update frequency results may be stale (see @ref setTimeout()).
   */
  void setMode(RCWL_1X05_mode newMode);
  
  /*!
   * @brief As the speed of sound varies with air temperature, all measurements 
   * will be adjusted [according to temperature](https://de.wikipedia.org/wiki/Schallgeschwindigkeit#Schallgeschwindigkeit_im_realen_Gas_/_Ph%C3%A4nomene_in_der_Luftatmosph%C3%A4re)
   * The default is 20° Celsius. Use this method to change the default.
   * @param newTemperature in Celsius
   */
  void setTemperature(int16_t newTemperature);
  
  /*!
   * @brief I2C targets cannot inform the controller if a measurement has
   * completed. So we'll need a timeout, after which the controller can safely
   * assume that a measurement will have completed. Depending on mode, this 
   * timeout determines
   * 
   * 1. the duration after which read() will return in oneShot mode, 
   * 2. the minimum time the controller should do other things after calling 
   * trigger() and before calling read() in triggered mode, and 
   * 3. the maximum staleness of a value in continuous mode.
   * 
   * The default is 100 ms, the recommended minimum value given in the "datasheet".
   * I found that I could safely reduce this to about 30-40 ms without problems for
   * ranges up to ca. 3 m, so play around with this if timing is critical. You 
   * know that your timeout is too short if you read 0 values in raw (non filtered)
   * mode. 
   * 
   * Note that you cannot simply calculate your timeout from the maximum
   * range you need to able to detect in your application, as there is some 
   * unknown overhead involved, so trial and error is probably the best approach 
   * if you want to minimize timeout.
   * @param newTimeout in milliseconds
   */
  void setTimeout(unsigned long newTimeout);
  
  /*!
   * @brief Get one measurement. Operation and usage depends on current mode, see @ref 
   * setMode() for details. Results can be raw or filtered with setFilter(), 
   * default is raw.
   * @returns distance in millimeters. 0 if no data was available (yet), i.e. a 
   * trigger() call was missing in triggered mode, or the timeout set with 
   * setTimeout() was too short. Note that the minimal distance/blind zone of 
   * the sensor is 250(!) mm, so any value less or equal 250 mm cannot be trusted.
   * @sa setFilter()
   */
  uint32_t read();
  
  /*!
   * @brief Initiate a measurement in triggered mode. Don't use in other modes.
   * @returns true if trigger command was successfully transmitted
   * @sa setMode()
   */
  bool trigger();
  
  /*!
   * @brief Update measurement and initiate subsequent measurement if completed
   * in continuous mode. Don't use in other modes. Call as frequently as possible,
   * if you want your measured values to be current. If you omit calling this,
   * you'll get only garbage from read() in continuous mode.
   * @returns true if a measurement was completed since last calling update().
   */
  bool update();
  
  /*!
   * @brief read() reports raw measurements by default. The filter buffers the
   * last five measurements and reports the median value of these five. 
   * Out-of-range values smaller than minDistace and larger than maxDistance are
   * ignored, i.e. not entered into the buffer. Note that depending on your
   * frequency of measurements, the filter comes at the cost of increased 
   * lagginess. Probably it makes most sense to use the filter in continuous mode.
   * @param on true to turn filter on.
   * @note Filter code was adapted from the fine [AsyncSonar libarary](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar)
   * by [Luis Llamas](https://github.com/luisllamasbinaburo/).
   */
  void setFilter(bool on);
  
     
private:
  
  uint32_t speedOfSoundTempCorrected(int16_t temp);
  uint32_t getOutput();
  uint32_t getRawOutput();
  uint32_t getFilteredOutput();
  uint32_t median(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
  
  uint32_t applyTemperatureCorrection(uint32_t oldDistance);
  
  TwoWire *wire;
  uint8_t address = RCWL_1X05_ADDR;
  
  RCWL_1X05_mode mode = oneShot;

  int16_t temperature;
  uint32_t speedOfSound;

  unsigned long timeout = defaultTimeout;
  unsigned long lastTrigger = 0;
  uint32_t lastValue = 0;
  
  uint32_t filterBuffer[5];
  uint8_t filterIndex = 0;
  bool filterOn = false;
  
  // limits used for filtered results, @todo make them settable by user
  uint32_t minDistance = uint32_t(25) * uint32_t (10) * uint32_t(1000); // 25 cm in micrometers
  uint32_t maxDistance = uint32_t(5) * uint32_t(1000) * uint32_t(1000); // 5 m in micrometers
  
};



#endif // #ifndef RCWL_1X05_h

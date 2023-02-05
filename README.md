# RCWL-1005 and RCWL-1605 (and HC-SR04<u>P</u>) I2C-mode Arduino library

I purchased one of each of these cheap sensors recently from aliexpress for testing purposes. Compared with the ubiquitous HC-SR04<sup>(*)</sup>, I liked the idea of having a combined emitter/receiver, i.e. smaller footprint, and the I2C/UART interface. As documentation on the web is sparse, I decided to collect the little information I found and throw my I2C mode test code into a little library.

My overall test results and experiences are a bit mixed, I think in many cases the good old HC-SR04 will still be an adequate choice, due to the 1X05's depressingly long minimum range of 25 cm.

<sup>(*)</sup>Note: only after writing most of this up, I realized there is a "newer" **HC-SR04P** module (notice the "P") which probably uses the same flavor of chip. I expect that most of what I write here, as well as the library and the UART/serial example sketch, will apply.

## Library Usage

The library for the sensors' [I2C interface](#i2c-mode) supports three modes of operation, `oneShot` (blocking), `triggered` (non blocking), and `continuous` (non blocking). See example code below and library documentation for `RCWL_1X05::setMode()`. 

Results can be raw or filtered, see `RCWL_1X05::setFilter()`. 

The default timeout of 100 ms can be changed with `RCWL_1X05::setTimeout()`.

A temperature correction is available, but due to incomplete documentation it might be off, see `RCWL_1X05::setTemperature()`.

By default, `Wire.h` is used, but other I2C libraries derived from `TwoWire` can replace it, see `RCWL_1X05::begin(TwoWire*,uint8_t)`.

Find the [full library documentation here](`RCWL_1X05::).

### Example

```cpp
#include <RCWL_1X05.h>
#include <Wire.h>

RCWL_1X05 sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(); // must(!) be called before sensor.begin()
  if (not sensor.begin()) {
    Serial.println("Sensor not found. Check connections and I2C-pullups and restart sketch.");
    while (true) {}
  } else {
    Serial.println("Sensor ready, let's start\n\n");
    delay(1000);
  }
  
  Serial.print("one shot mode measurement (blocking) = ");
  sensor.setMode(oneShot); // not really needed, it's the default mode
  Serial.print(sensor.read()); Sensor.println(" mm\n\n");
  delay(2000);
  
  Serial.print("triggered mode measurement (non blocking) = ");
  sensor.setMode(triggered);
  sensor.trigger();
  delay(100); // do something meaningful, here, but wait long enough before reading.
  Serial.print(sensor.read()); Sensor.println(" mm\n\n");
  delay(2000);
  
  Serial.println("switching to continous mode measurement with filter and 50 ms timeout\n\n");
  sensor.setFilter(true); // filter is turned off by default
  sensor.setTimeout(50); // 100 ms is recommended, but lower values might work
  sensor.setMode(continuous);  
}
    
void loop() {
  bool newData = sensor.update(); // calling update() repeatedly is crucial in continous mode
  if (newData) {
    Serial.print(sensor.read()); Sensor.println(" mm");
  }
  // do your other stuff, here
}

```

## Information on RCWL-1X05 modules

### "datasheet"

The English "**datasheet**" I included is compiled from different aliexpress product sites, it is not an official datasheet. However, the information  seems to be a more or less direct translation of the Chinese "datasheet", which honestly I can't remember where and how I found it, it might have been a product page at a Chinese only market place, taobao or something.

### Manufacturer homepage

The **company website** of "Wuxi Richen Wulian Technology Co., Ltd." is hard to find, the one mentioned in the "datasheet", www.wx-rcwl.com, is not working. I managed to track them down with the help of the self-diagnostic company info which the modules provide in serial/UART mode (see `Serial_demo.ino`), by googling for the GB18030 encoded Chinese company name 无锡日晨物联, which led to the correct URL https://www.sz-rcwl.com/. However, they don't provide datasheets there, neither in the Chinese, nor the English page version. I tried to contact them, but have not heard back to date.

### RCWL-96XX chip

It seems both module use a **chip** developed by said manufacturer. Its name varies in the sources, RCWL-9624, RCWL-9625, RCWL-9623, RCCL-9624 all can be found. The website only lists a 9600 chip, I assume we are dealing with variants of this chip. Note it seems the new HC-SR04P modules are equipped with the "default" 9600 flavor, [which identifies itself as RCWL-2020 in UART/serial mode.](https://www.mikrocontroller.net/topic/530175)

### Differences between RCWL-1605 and RCWL-1005

I could not find any useful information on the **differences between the two versions**, apart from the diameter of the combined emitter/receiver, which is encoded in the first two digits, i.e. 10 mm (RCWL-1005) and 16 mm (RCWL-1605). I assume this diameter will have an impact on range, angle, and sensitivity, but I did not run systematic tests on that.

Also, it could be both modules use different variants of the aforementioned chip, RCWL-9624 vs. RCWL-9623, but that could just be typos as well. I could not identify any chip label on my own modules. The serial/UART sketch shows that both modules correctly identify themselves as 1605 vs. 1005, so the chips differ at least in that regard.

### Interface modes

You can select the **interface mode** with two resistor bridges by adding 10K resistors or leaving them open. For a change, this is documented quite well in a table in the "datasheet" and on the module itself. The bridges are labeled M1 (= R8) and M2 (= R6). Looking at the module schematic, I think that the resistors are not (only) simple pullups. The 10K value might actually be meaningful, as they are also part of a voltage divider for the emitter/receiver. My knowledge of electronics is limited, though, and I have not tested other values than 10K.

<a id="i2c-mode"></a>

#### I2C mode

The modules' I2C mode is extremely basic, at least as far as it is documented. See the library documentation and example code below for details.

According to the schematic there's no **I2C pullups** included on the module, so be sure to add them to the bus, and of course set the 10K resistor at M2.

#### UART/Serial mode

Serial mode is also very basic, but it includes an additional info/version command. See `Serial_demo.ino` if you want to test it, but don't forget to set the 10K resistor at M1.

#### "GPIO"/Two wire mode

The module's default two-wire mode should work just like the HC-SR04 by using the trigger and echo pins. I have not tested this mode.

#### One-wire/"single bus" mode

One-wire needs both M1 and M2 set with 10K resistors. I assume it uses the "Trig_RX_SCL_I/O" pin. It should work just as the HC-SR04 in pin shared mode, as supported e.g. by [AsyncSonar libarary](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar). I have not tested this mode.

## Comparison with HC-SR04 modules

Major differences are ***highlighted***.

|                              | RCWL-1X06           | HC-SR04          |
| ---------------------------- | ------------------- | ---------------- |
| min. range                   | ***25 cm***         | 2 cm             |
| max. range                   | 4.5 m               | 3/5 m            |
| voltage                      | ***2.8 V - 5.5 V*** | 5 V              |
| resolution                   | 1 mm                | 3 mm             |
| two-wire mode <sup>(1)</sup> | X                   | X                |
| one-wire mode <sup>(2)</sup> | X                   | X <sup>(3)</sup> |
| I2C mode                     | ***X***             | -                |
| UART/serial mode             | ***X***             | -                |
| module footprint             | ***20 x 20 mm***    | 45 x 21 mm       |
|                              |                     |                  |

<sup>(1)</sup> "GPIO mode", emitter (output) and receiver (input) on separate pins

<sup>(2)</sup> shared pin, switched between out- and input during one measurement cycle

<sup>(3)</sup> by connecting trigger and echo pin, implemented e.g. by [AsyncSonar libarary](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar)

# Author

Contact me at ftjuh@posteo.net.

Jan (juh)

# Copyright

This software is Copyright (C) 2023 juh (ftjuh@posteo.net)

# License

This software is distributed under the GNU GENERAL PUBLIC LICENSE Version 2.

# History

see [releases page](https://github.com/ftjuh/RCWL_1X05/releases)




![](https://visitor-badge.laobi.icu/badge?page_id=ftjuh.RCWL_1X05)
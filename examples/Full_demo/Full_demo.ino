#include <RCWL_1X05.h>
#include <Wire.h>

RCWL_1X05 sensor;

void setup() {
  
  Serial.begin(115200); delay (500);
  Serial.println("==============\nRCWL_1X05 demo\n==============\n\n");
  
  Wire.begin(); // must be called before sensor.begin()
  if (not sensor.begin()) {
    Serial.println("Sensor not found. Check connections and I2C-pullups and restart sketch.");
    while (true) {}
  } else {
    Serial.println("Sensor ready, let's start\n\n");
    delay(1000);
  }
  
  Serial.print("One shot mode measurement (blocking) = ");
  sensor.setMode(RCWL_1X05::oneShot); // not really needed, it's the default mode
  Serial.print(sensor.read()); Serial.println(" mm\n\n");
  delay(2000);
  
  Serial.print("Triggered mode measurement (non blocking) = ");
  sensor.setMode(RCWL_1X05::triggered);
  sensor.trigger();
  delay(100); // do something meaningful here, but wait long enough before reading.
  Serial.print(sensor.read()); Serial.println(" mm\n\n");
  delay(2000);
  
  Serial.println("Switching to continuous mode measurement with filter and 50 ms timeout\n\n");
  sensor.setFilter(true); // filter is turned off by default
  sensor.setTimeout(50); // 100 ms is recommended, but lower values might work
  sensor.setMode(RCWL_1X05::continuous);  
}
    
void loop() {
  bool newData = sensor.update(); // calling update() repeatedly is crucial in continuous mode
  if (newData) {
    Serial.print(sensor.read()); Serial.println(" mm");
  }
}

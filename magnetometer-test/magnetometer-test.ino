#include <Wire.h>
#include <Adafruit_LSM303_U.h>
#include <TinyGPS++.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

// statue of liberty 47.486681, 19.048306
// corner office 47.486533, 19.074511
TinyGPSPlus gps;

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}

void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
  
  int newCourseAngle = (int) (atan2(event.magnetic.y,event.magnetic.x) * 180 / PI + 0.5);
  int newTargetAngle = (int) (gps.courseTo(47.486533, 19.074511, 47.486681, 19.048306) + 0.5);

  int servoAngle = (450 - (newTargetAngle - newCourseAngle)) % 360;

  Serial.println();
  Serial.print(F("heading: "));
  Serial.print(newCourseAngle);
  Serial.print(F(" | target: "));
  Serial.print(newTargetAngle);
  Serial.print(F(" | servo: "));
  Serial.print(servoAngle);

  delay(500);
}

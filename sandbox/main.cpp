#include <iostream>
#include "TinyGPS++.h"
#include "SoftwareSerial.h"
#include "Servo.h"

/*
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Servo.h>
*/

/*
 * Model, Coord, Angle
 */
struct Coord {
    Coord(double lat, double lon):
            lat(lat),
            lon(lon) {};
    double lat;
    double lon;
};

struct Angle {
    bool valid;
    int degrees;
};

class Model {
public:
    Model(TinyGPSPlus& gps, SoftwareSerial& sserial, Coord& dest):
            gps(gps),
            sserial(sserial),
            dest(dest) {
        course.valid = false;
        target.valid = false;
    };
    void tick();
    Angle course;
    Angle target;
    TinyGPSPlus& gps;
private:
    const int angleThreshold = 4;
    SoftwareSerial& sserial;
    Coord& dest;
    bool encodeNextFromSerial();
    void updateModel();
};

void Model::tick() {
    if (encodeNextFromSerial()) {
        updateModel();
    }
}

bool Model::encodeNextFromSerial() {
    if (sserial.available()) {
        return gps.encode(sserial.read());
    }
    return false;
}

void Model::updateModel() {
    if ( gps.location.isValid() ) {
        target.valid = true;
        int newTargetAngle = (int) (gps.courseTo(gps.location.lat(), gps.location.lng(), dest.lat, dest.lon) + 0.5);
        if (abs(newTargetAngle - target.degrees) > angleThreshold) {
            target.degrees = newTargetAngle;
        }
    } else {
        target.valid = false;
    }

    if ( gps.course.isValid() ) {
        course.valid = true;
        int newCourseAngle = (int) (gps.course.deg() + 0.5);
        if ( abs(newCourseAngle - course.degrees) > angleThreshold ) {
            course.degrees = newCourseAngle;
        }
    } else {
        course.valid = false;
    }
}

/*
 * Ticker
 */
class Ticker {
public:
    explicit Ticker(unsigned long updateInterval):
            prevMillis(0),
            interval(updateInterval) {};
    bool tick(unsigned long millis);
    virtual void update() = 0;
    unsigned long millis() { return prevMillis;}
private:
    unsigned long prevMillis;
    unsigned long interval;
};

bool Ticker::tick(unsigned long millis) {
    if (millis - prevMillis >= interval) {
        prevMillis = millis;
        update();
        return true;
    }
    return false;
}

/*
 * SerialLogger
 */
class SerialLogger : public Ticker {
public:
    SerialLogger(Model& model, unsigned long updateInterval):
            model(model),
            Ticker(updateInterval) {};
    virtual void update();
private:
    Model& model;
    void logGpsTime();
    void logGpsLocation();
    void logGpsCourse();
    void logModelCourse();
    void logModelTarget();
};

void SerialLogger::update() {
    logGpsTime();
    logGpsLocation();
    logGpsCourse();
    logModelCourse();
    logModelTarget();
    Serial.println();
}

void SerialLogger::logGpsTime(){
    if (model.gps.time.isValid()) {
        Serial.print(F("T: "));
        Serial.print(model.gps.time.hour());
        Serial.print(F(":"));
        Serial.print(model.gps.time.minute());
        Serial.print(F(":"));
        Serial.print(model.gps.time.second());
        Serial.print(F("| "));
    } else {
        Serial.print(F("–Time– | "));
    }
}

void SerialLogger::logGpsLocation(){
    if(model.gps.location.isValid()) {
        Serial.print(F("L: "));
        Serial.print(model.gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(model.gps.location.lng(), 6);
        Serial.print(F("| "));
    } else {
        Serial.print(F("–Location– | "));
    }
}

void SerialLogger::logGpsCourse(){
    if (model.gps.course.isValid()) {
        Serial.print(F("C: "));
        Serial.print(model.gps.course.deg(), 6);
        Serial.print(F("deg |"));
    } else {
        Serial.print(F("–Course– | "));
    }
}

void SerialLogger::logModelCourse(){
    if (model.course.valid) {
        Serial.print(F("MC: "));
        Serial.print(model.course.degrees);
        Serial.print(F("deg | "));
    } else {
        Serial.print(F("–MC– | "));
    }
}

void SerialLogger::logModelTarget(){
    if (model.target.valid) {
        Serial.print(F("MT: "));
        Serial.print(model.target.degrees);
        Serial.print(F("deg | "));
    } else {
        Serial.print(F("–MT–"));
    }
}

/*
 * Compass
 */
class Compass : public Ticker {
public:
    Compass(Model& model, Servo& servo, unsigned long updateInterval):
            model(model),
            servo(servo),
            Ticker(updateInterval) {};
    virtual void update();
private:
    Model& model;
    Servo& servo;
    int wiggleDirection = 1;
    const int wiggleAngle = 15;
};

void Compass::update() {
    if (model.course.valid && model.target.valid) {
        servo.write(90 + model.target.degrees - model.target.degrees);
    } else {
        servo.write(90 + wiggleDirection * wiggleAngle);
        wiggleDirection *= -1;
    }
}

/*
 * Globals
 */
Coord destination(47.486533, 19.074511);

TinyGPSPlus gps;
Servo servo;
SoftwareSerial softwareSerial(3, 4);
Model model(gps, softwareSerial, destination);
SerialLogger logger(model, 1000);
Compass compass(model, servo, 1000);

void setup() {
    softwareSerial.begin(9600);
    Serial.begin(9600);
    servo.attach(9);
}

void loop() {
    model.tick();
    logger.tick(millis());
    compass.tick(millis());
}

int main() {
    setup();
    while (true)
        loop();
}


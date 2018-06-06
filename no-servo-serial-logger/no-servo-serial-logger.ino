#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_LSM303_U.h>

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
    // ToDo! remove sserial
    Model(TinyGPSPlus& gps, SoftwareSerial& sserial, Adafruit_LSM303_Mag_Unified& magnetometer, Coord& dest):
            gps(gps),
            dest(dest),
            magnetometer(magnetometer),
            // ToDo! remove sserial
            sserial(sserial) {
        course.valid = false;
        target.valid = false;
    };
    void tick();
    Angle course;
    Angle target;
    TinyGPSPlus& gps;
    Adafruit_LSM303_Mag_Unified& magnetometer;
    // ToDo! remove sserial when done
    SoftwareSerial& sserial;
private:
    const int angleThreshold = 4;
    
    sensors_event_t event;
    Coord& dest;

    void readSensors();
    void updateModel();
    void updateLocation();
    void updateHeading();
};

void Model::tick() {
    readSensors();
    updateModel();
}

void Model::readSensors() {
    if (Serial.available()) {
        gps.encode(Serial.read());
    }
    magnetometer.getEvent(&event);
}

void Model::updateModel() {
    updateLocation();
    updateHeading();
}

void Model::updateLocation() {
    if ( gps.location.isValid() ) {
        target.valid = true;
        int newTargetAngle = (int) (gps.courseTo(gps.location.lat(), gps.location.lng(), dest.lat, dest.lon) + 0.5);
        if (abs(newTargetAngle - target.degrees) > angleThreshold) {
            target.degrees = newTargetAngle;
        }
    } else {
        target.valid = false;
    }
}

void Model::updateHeading() {
    int newCourseAngle = (int) ((atan2(event.magnetic.y,event.magnetic.x) * 180) / PI + 0.5);
    newCourseAngle = (newCourseAngle + 360) % 360;
    if ( abs(newCourseAngle - course.degrees) > angleThreshold ) {
        course.degrees = newCourseAngle;
    }
    course.valid = true;
}

/*
 * Ticker
 */
class Ticker {
public:
    Ticker(unsigned long updateInterval):
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
    const int wiggleAmplitude = 35;
    const int servoMaxAngle = 170;
};

void Compass::update() {
    if (model.gps.course.isValid() && model.gps.location.isValid()) {
        int servoAngle = (450 - (model.target.degrees - model.course.degrees)) % 360;
        //servoAngle = min(servoMaxAngle, servoAngle);
        if (servoAngle > servoMaxAngle) {
          servoAngle = servoMaxAngle;
          Serial.println();
          Serial.print(F("*"));
        }
        Serial.println();
        Serial.print(F("model.target.degrees "));
        Serial.print(model.target.degrees);
        Serial.print(F(" model.course.degrees "));
        Serial.print(model.course.degrees);
        Serial.println();
        Serial.print(F("Angle: "));
        Serial.print(servoAngle);
        // servo.write(angle);
    } else {
        wiggleDirection = -wiggleDirection;
        int wiggleAngle = 90 + wiggleDirection * wiggleAmplitude;
        Serial.println();
        Serial.print(F("Wiggle: "));
        Serial.print(wiggleAngle);
        //servo.write(wiggleAngle);
    }
}

class SerialLogger : public Ticker {
public:
    SerialLogger(Model& model, unsigned long updateInterval):
            model(model),
            Ticker(updateInterval) {};
    virtual void update();
private:
    Model& model;
};

void SerialLogger::update() {
    Serial.println();
    if (model.gps.date.year() == 2000 && model.gps.date.month() == 0 && model.gps.date.day() == 0) {
        Serial.print(F("No clock "));
    } else {
        Serial.print(model.gps.date.year());
        Serial.print(F("/"));
        Serial.print(model.gps.date.month());
        Serial.print(F("/"));
        Serial.print(model.gps.date.day());
        Serial.print(F(" "));
    }
    if (!model.gps.location.isValid()) {
        Serial.print(F("No Location "));
    } else {
        Serial.print(F("L: "));
        Serial.print(model.gps.location.lat());
        Serial.print(F(","));
        Serial.print(model.gps.location.lng());
    }   
}

/*
 * Globals
 */
Coord destination(47.486533, 19.074511);

TinyGPSPlus gps;
Servo servo;
Adafruit_LSM303_Mag_Unified magnetometer(12345);
// ToDo! remove sserial
SoftwareSerial sserial(3,4);
Model model(gps, sserial, magnetometer, destination);
Compass compass(model, servo, 500);
SerialLogger logger(model, 1000);

void setup() {
    Serial.begin(9600);
    // ToDo! remove sserial
    sserial.begin(9600);
    //servo.attach(11);
    magnetometer.begin();
}

void loop() {
    model.tick();
    compass.tick(millis());
    logger.tick(millis());
}


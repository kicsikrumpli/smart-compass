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
    Model(TinyGPSPlus& gps, Adafruit_LSM303_Mag_Unified& magnetometer, Coord& dest):
            gps(gps),
            dest(dest),
            magnetometer(magnetometer) {
        course.valid = false;
        target.valid = false;
    };
    void tick();
    Angle course;
    Angle target;
    TinyGPSPlus& gps;
    Adafruit_LSM303_Mag_Unified& magnetometer;
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
        target.degrees = (int) (gps.courseTo(gps.location.lat(), gps.location.lng(), dest.lat, dest.lon) + 0.5);
    } else {
        target.valid = false;
    }
}

void Model::updateHeading() {
    course.degrees = (int) (atan2(event.magnetic.y, event.magnetic.x) * 180 / PI + 0.5);
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
    bool isClockReady();
    bool isCourseReady();
};

void Compass::update() {
    if (isClockReady() && isCourseReady() ) {
        int servoAngle = (450 - (model.target.degrees - model.course.degrees)) % 360;
        if (servoAngle > servoMaxAngle) {
            servoAngle = servoMaxAngle;
        }
        servo.write(servoAngle);
    } else {
        wiggleDirection = -wiggleDirection;
        int wiggleAngle = 90 + wiggleDirection * wiggleAmplitude;
        servo.write(wiggleAngle);
    }
}

bool Compass::isClockReady() {
    return !(model.gps.date.year() == 2000 && model.gps.date.month() == 0 && model.gps.date.day() == 0);
}

bool Compass::isCourseReady() {
    return (model.gps.course.isValid() && model.gps.location.isValid());
}

/*
 * Globals
 */
//Corner Office
//Coord destination(47.486533, 19.074511);
//Statue of Liberty
Coord destination(47.486681, 19.048306);

TinyGPSPlus gps;
Servo servo;
Adafruit_LSM303_Mag_Unified magnetometer(12345);
Model model(gps, magnetometer, destination);
Compass compass(model, servo, 500);

void setup() {
    Serial.begin(9600);
    servo.attach(11);
    magnetometer.begin();
}

void loop() {
    model.tick();
    compass.tick(millis());
}

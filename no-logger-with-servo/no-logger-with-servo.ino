#include <Servo.h>
#include <TinyGPS++.h>

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
    Model(TinyGPSPlus& gps, Coord& dest):
            gps(gps),
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
    if (Serial.available()) {
        return gps.encode(Serial.read());
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
Coord destination(47.486533, 19.074511);

TinyGPSPlus gps;
Servo servo;
Model model(gps, destination);
Compass compass(model, servo, 500);

void setup() {
    Serial.begin(9600);
    servo.attach(11);
}

void loop() {
    model.tick();
    compass.tick(millis());
}


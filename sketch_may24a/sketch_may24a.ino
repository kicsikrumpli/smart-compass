#include <SoftwareSerial.h>
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
    Model(TinyGPSPlus& gps, SoftwareSerial& sserial, Coord& dest):
            gps(gps),
            sserial(sserial),
            dest(dest),
            updated(false) {
        course.valid = false;
        target.valid = false;
    };
    void tick();
    bool updated;
    Angle course;
    Angle target;
    TinyGPSPlus& gps;
private:
    const int angleThreshold = 2;
    SoftwareSerial& sserial;
    Coord& dest;
    bool encodeNextFromSerial();
    void updateModel();
};

void Model::tick() {
    updated = false;
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
    updated = false;
    if ( gps.location.isValid() ) {
        target.valid = true;
        int newTargetAngle = (int) gps.courseTo(gps.location.lat(), gps.location.lng(), dest.lat, dest.lon);
        if (abs(newTargetAngle - target.degrees) > angleThreshold) {
            target.degrees = newTargetAngle;
            updated = updated || gps.location.isUpdated();
        }
    } else {
        target.valid = false;
    }
    if ( gps.course.isValid() ) {
        course.valid = true;
        int newCourseAngle = (int) gps.course.deg();
        if ( abs(newCourseAngle - course.degrees) > angleThreshold ) {
            course.degrees = newCourseAngle;
            updated = updated || gps.course.isUpdated();
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
    void logModelUpdateStatus();
    void logModelCourse();
    void logModelTarget();
};

void SerialLogger::update() {
    logGpsTime();
    logGpsLocation();
    logGpsCourse();
    logModelUpdateStatus();
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

void SerialLogger::logModelUpdateStatus(){
    if (model.updated) {
        Serial.print(F("! | "));
    } else {
        Serial.print(F("X | "));
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
// ToDo! - add compass code if serial logger is ok

/*
 * Globals
 */
Coord destination(47.486533, 19.074511);

TinyGPSPlus gps;
SoftwareSerial softwareSerial(3, 4);
Model model(gps, softwareSerial, destination);
SerialLogger logger(model, 1000);

void setup() {
  softwareSerial.begin(9600);
  Serial.begin(9600);
}

void loop() {
  model.tick();
  logger.tick(millis());
}

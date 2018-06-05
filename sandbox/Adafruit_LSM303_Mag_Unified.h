//
// Created by daniel david kovacs on 6/5/18.
//

#ifndef SANDBOX_ADAFRUIT_LSM303_MAG_UNIFIED_H
#define SANDBOX_ADAFRUIT_LSM303_MAG_UNIFIED_H

float atan2(float a, float b) {
    return a + b;
}

struct sensors_event_t {
    struct magnetic {
        float x;
        float y;
    } magnetic;
} sensors_event_t;


class Adafruit_LSM303_Mag_Unified {
public:
    Adafruit_LSM303_Mag_Unified(int id) : id(id) {}
    bool begin();
    void getEvent(sensors_event_t * event) {}
private:
    int id;
};

#endif //SANDBOX_ADAFRUIT_LSM303_MAG_UNIFIED_H

//
// Created by daniel david kovacs on 5/24/18.
//

#ifndef SANDBOX_TINYGPSPLUS_H
#define SANDBOX_TINYGPSPLUS_H

int min(int a, int b) { return a; }

class TinyGPSPlus {
public:
    TinyGPSPlus(): tick(0) {};


    bool encode(char c) {
        tick += 1;
        return tick % 3 == 0;
    }

    struct location {
        bool isValid() { return true; }
        bool isUpdated() { return true; }
        double lat() { return 10.0; }
        double lng() { return 10.0; }
    } location;

    struct course {
        bool isValid() { return true; }
        bool isUpdated() { return true; }
        double deg() { return 10.0; }
    } course;

    struct time {
        bool isValid() { return true; }
        bool isUpdated() { return true; }
        int hour() { return 12; }
        int minute() { return 10; }
        int second() { return 30; }
    } time;

    struct date {
        int year() { return 2000; }
        int month() { return 0; }
        int day() { return 0; }
    } date;

    double courseTo(double lat1, double lon1, double lat2, double lon2) {
        return 10.0;
    }
private:
    int tick;
};

#endif //SANDBOX_TINYGPSPLUS_H

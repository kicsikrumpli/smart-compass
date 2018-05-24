//
// Created by daniel david kovacs on 5/24/18.
//

#ifndef SANDBOX_SOFTWARESERIAL_H
#define SANDBOX_SOFTWARESERIAL_H

#include <iostream>

const char* F(const char *c) {
    return c;
}

unsigned long millis() {
    return 0;
}

class SoftwareSerial {
public:
    SoftwareSerial(char rx, char tx): c('a' - 1), tick(0) {};
    void begin(int baud) {};

    int available() {
        tick ++;
        return tick % 10;
    };

    char read() {
        c = (char) (c - 'a' + 1) % ('z' - 'a') + 'a';
        return c;
    };
private:
    char c;
    int tick;
};

class Serial {
public:
    void begin(int baud) {};

    void print(const char * c) {
        std::cout << c;
    };
    void print(int i) {
        std::cout << i;
    };
    void print(double i, int j) {
        std::cout << i;
    };
    void println() {
        std::cout << std::endl;
    }
private:

} Serial;

#endif //SANDBOX_SOFTWARESERIAL_H

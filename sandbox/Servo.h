//
// Created by daniel david kovacs on 5/24/18.
//

#ifndef SANDBOX_SERVO_H
#define SANDBOX_SERVO_H

#include <iostream>

class Servo {
public:
    void attach(int pin) {
        std::cout << "servo attached to pin " << pin << std::endl;
    };
    void write(int angle) {
        std::cout << "servo: " << angle << std::endl;
    };
private:
};

#endif //SANDBOX_SERVO_H

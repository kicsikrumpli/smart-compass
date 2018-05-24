cmake_minimum_required(VERSION 3.10)
project(sandbox)

set(CMAKE_CXX_STANDARD 11)

add_executable(sandbox main.cpp SoftwareSerial.cpp SoftwareSerial.h TinyGPS++.cpp TinyGPS++.h Servo.cpp Servo.h)
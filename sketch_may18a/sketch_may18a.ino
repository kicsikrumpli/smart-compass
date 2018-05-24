#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

class Coord {
  public:
    long lat;
    long lon;
    Coord(long lat, long lon) : lat(lat), lon(lon) {};
};

class Model {
  public:
    Model(TinyGPSPlus& gps, Coord& destination):
      gps(gps),
      destination(destination) {}
    // ToDo! facade methods???
  private:
    TinyGPSPlus& gps;
    Coord& destination;
};

class Ticker {
  public:
    Ticker(): previousTick(0) {}

  private: 
};

class SerialLogger {
  public:
    SerialLogger(Model& model, SoftwareSerial& sserial):
      model(model),
      sserial(sserial) {}
    bool tick();
  private:
};

class Compass {
  public:
    Compass(Model& model, Servo& servo):
      model(model), 
      servo(servo),
      previousTick(0) {}
    bool tick();
  private:
    const unsigned long TEST_INTERVAL = 1000;
    Model& model;
    Servo& servo;
    unsigned long previousTick;
};

void Compass::update() {
  unsigned long tick = millis();
  if (previousTick - tick > TEST_INTERVAL) {
    previousTick = tick;
  }
  return;
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

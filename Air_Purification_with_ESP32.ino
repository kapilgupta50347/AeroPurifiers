#define REMOTEXY_MODE__WIFI

#include <WiFi.h>
#include <RemoteXY.h>

// WiFi credentials for hotspot/router
#define REMOTEXY_WIFI_SSID "kapil"
#define REMOTEXY_WIFI_PASSWORD "cocomelon"
#define REMOTEXY_SERVER_PORT 6377

// RemoteXY GUI config (paste your RemoteXY_CONF array here)
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255,4,0,0,0,56,0,19,0,0,0,0,31,1,106,200,1,1,3,0,
  2,10,17,44,22,0,2,26,31,31,79,78,0,79,70,70,0,2,60,16,
  44,22,0,2,26,31,31,79,78,0,79,70,70,0,5,26,82,60,60,32,
  2,26,31
};
#pragma pack(pop)

struct {
  uint8_t Humidifier;      // On=1, Off=0
  uint8_t HVGEN_DCFAN;     // On=1, Off=0
  int8_t joystick_01_x;    // -100 to 100
  int8_t joystick_01_y;    // -100 to 100
  uint8_t connect_flag;
} RemoteXY;

// Motor pins
#define ENA 27
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32
#define ENB 21

// Relay pins
#define PIN_HUMIDIFIER 23
#define PIN_HVGEN_DCFAN 22

void setup() {
  Serial.begin(115200);
  RemoteXY_Init();

  pinMode(PIN_HUMIDIFIER, OUTPUT);
  pinMode(PIN_HVGEN_DCFAN, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Correct ledcAttach with freq and resolution arguments
  ledcAttach(ENA, 5000, 8);
  ledcAttach(ENB, 5000, 8);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

void loop() {
  RemoteXY_Handler();

  digitalWrite(PIN_HUMIDIFIER, RemoteXY.Humidifier ? LOW : HIGH);
  digitalWrite(PIN_HVGEN_DCFAN, RemoteXY.HVGEN_DCFAN ? LOW : HIGH);

  controlMotorsWithJoystick(RemoteXY.joystick_01_x, RemoteXY.joystick_01_y);
}

void controlMotorsWithJoystick(int8_t x, int8_t y) {
  int threshold = 20;
  int speed;

  if (y > threshold) {
    speed = map(y, threshold, 100, 0, 255);
    setMotorDirection(true, false, true, false);
    setMotorSpeed(speed, speed);
  } else if (y < -threshold) {
    speed = map(-y, threshold, 100, 0, 255);
    setMotorDirection(false, true, false, true);
    setMotorSpeed(speed, speed);
  } else if (x > threshold) {
    speed = map(x, threshold, 100, 0, 255);
    setMotorDirection(false, true, true, false);  // Inverted: Right
    setMotorSpeed(speed, speed);
  } else if (x < -threshold) {
    speed = map(-x, threshold, 100, 0, 255);
    setMotorDirection(true, false, false, true);  // Inverted: Left
    setMotorSpeed(speed, speed);
  } else {
    setMotorSpeed(0, 0);
    setMotorDirection(false, false, false, false);
  }
}

void setMotorSpeed(int speedA, int speedB) {
  ledcWrite(ENA, speedA);
  ledcWrite(ENB, speedB);
}

void setMotorDirection(bool leftForward, bool leftBackward, bool rightForward, bool rightBackward) {
  digitalWrite(IN1, leftForward ? HIGH : LOW);
  digitalWrite(IN2, leftBackward ? HIGH : LOW);
  digitalWrite(IN3, rightForward ? HIGH : LOW);
  digitalWrite(IN4, rightBackward ? HIGH : LOW);
}
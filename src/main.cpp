#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ArduinoBlue.h>

// Rear motor drivers
const int r_enA = 9;
const int r_in1 = 8;
const int r_in2 = 7;
const int r_enB = 3;
const int r_in3 = 5;
const int r_in4 = 4;

// Front motor drivers
const int f_enA = 11;
const int f_in1 = A0;
const int f_in2 = A1;
const int f_enB = 10;
const int f_in3 = A2;
const int f_in4 = 2;

// Handling
const int innerTurn = 128;

// Crane
const int servoPin = 6;
Servo serv;

// Bluetooth module
// HM-10 RX -> 12
//       TX -> 13
const int bt_tx = 12;
const int bt_rx = 13;
SoftwareSerial bt(bt_rx, bt_tx);
ArduinoBlue phone(bt);
int prevSteer = 49;
int prevThrottle = 49;
int steer, throttle, sliderVal, sliderId, button;
const int craneSlider = 0;

const int BAUD_RATE = 9600;

// Function prototypes
void direct(int motor, int dir);
void leftD(int dir);
void rightD(int dir);
void allD(int dir);
void handleCrane(int val);
void handleMovement(int x, int y);
int mapJoystick(int val);

void setup()
{
    // Set up motor drivers
    pinMode(f_enA, OUTPUT);
    pinMode(f_enB, OUTPUT);
    pinMode(f_in1, OUTPUT);
    pinMode(f_in2, OUTPUT);
    pinMode(f_in3, OUTPUT);
    pinMode(f_in4, OUTPUT);

    pinMode(r_enA, OUTPUT);
    pinMode(r_enB, OUTPUT);
    pinMode(r_in1, OUTPUT);
    pinMode(r_in2, OUTPUT);
    pinMode(r_in3, OUTPUT);
    pinMode(r_in4, OUTPUT);

    // Set up servo
    serv.attach(servoPin);

    // Set motor drivers to brake
    allD(0);

    // Set up communications
    Serial.begin(BAUD_RATE);

    bt.begin(BAUD_RATE);
    delay(100);

    Serial.println("Setup complete.");
}

void loop()
{
    // Get data from app
    throttle = phone.getThrottle();
    steer = phone.getSteering();
    String str = phone.getText();
    sliderId = phone.getSliderId();
    sliderVal = phone.getSliderVal();

    // Handle crane
    if (sliderId == craneSlider)
    {
        handleCrane(sliderVal);
    }

    // Handle movement
    if (prevThrottle != throttle || prevSteer != steer)
    {
        handleMovement(steer, throttle);
        prevSteer = steer;
        prevThrottle = throttle;
    }

    // Handle text
    if (str != "")
    {
        Serial.println(str);
    }

    if (Serial.available())
    {
        Serial.write("send: ");
        String str = Serial.readString();
        phone.sendMessage(str);
        Serial.print(str);
        Serial.write('\n');
    }
}

void handleCrane(int val)
{
    // Serial.print("Crane: ");
    // Serial.println(sliderVal);
    serv.write(map(sliderVal, 100, 0, 0, 180));
}

void handleMovement(int _x, int _y)
{
    int x = mapJoystick(_x);
    int y = mapJoystick(_y);

    if (x == 0)
    {
        leftD(y * 255);
        rightD(y * 255);
    }
    else if (y == 0)
    {
        leftD(-x * 255);
        rightD(x * 255);
    }
    else
    {
        leftD(y * (x > 0 ? 255 : innerTurn));
        rightD(y * (x > 0 ? innerTurn : 255));
    }

    // Serial.print("x: ");
    // Serial.print(x);
    // Serial.print(" y: ");
    // Serial.println(y);
}

void direct(int motor, int dir)
{
    int i1;
    int i2;
    int e;
    switch (motor)
    {
    case 0:
        i1 = f_in1;
        i2 = f_in2;
        e = f_enA;
        break;
    case 1:
        i1 = f_in3;
        i2 = f_in4;
        e = f_enB;
        break;
    case 2:
        i1 = r_in1;
        i2 = r_in2;
        e = r_enA;
        break;
    case 3:
        i1 = r_in3;
        i2 = r_in4;
        e = r_enB;
        break;
    }

    if (dir > 0)
    {
        digitalWrite(i1, HIGH);
        digitalWrite(i2, LOW);
    }
    else if (dir < 0)
    {
        digitalWrite(i1, LOW);
        digitalWrite(i2, HIGH);
    }
    else
    {
        digitalWrite(i1, LOW);
        digitalWrite(i2, LOW);
    }

    analogWrite(e, abs(dir));
    // Serial.println(motor);
    // Serial.println(dir);
}

void leftD(int dir)
{
    direct(0, dir);
    direct(2, dir);
    Serial.print("left: ");
    Serial.println(dir);
}

void rightD(int dir)
{
    direct(1, dir);
    direct(3, dir);
    Serial.print("right: ");
    Serial.println(dir);
}

void allD(int dir)
{
    leftD(dir);
    rightD(dir);
}

int mapJoystick(int val)
{
    switch (val)
    {
    case 0:
        return -1;
    case 49:
        return 0;
    case 99:
        return 1;
    default:
        return 0;
    }
}
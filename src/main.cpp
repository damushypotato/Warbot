#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ArduinoBlue.h>

const int r_enA = 9;
const int r_in1 = 8;
const int r_in2 = 7;
const int r_enB = 3;
const int r_in3 = 5;
const int r_in4 = 4;

const int f_enA = 11;
const int f_in1 = A0;
const int f_in2 = A1;
const int f_enB = 10;
const int f_in3 = A2;
const int f_in4 = 2;

const int servoPin = 6;
Servo serv;

#define bt_tx 12
#define bt_rx 13

#define BAUD_RATE 9600

SoftwareSerial bt(bt_rx, bt_tx);
ArduinoBlue phone(bt);

int prevSteer = 49;
int prevThrottle = 49;

int steer, throttle, sliderVal, sliderId, button;

void direct(int motor, int dir);
void leftD(int dir);
void rightD(int dir);
void handleMovement(int x, int y);

void setup()
{
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

    serv.attach(servoPin);

    digitalWrite(f_in1, LOW);
    digitalWrite(f_in2, LOW);
    digitalWrite(f_in3, LOW);
    digitalWrite(f_in4, LOW);

    digitalWrite(r_in1, LOW);
    digitalWrite(r_in2, LOW);
    digitalWrite(r_in3, LOW);
    digitalWrite(r_in4, LOW);

    Serial.begin(BAUD_RATE);

    bt.begin(BAUD_RATE);
    delay(100);

    Serial.println("setup complete");
}

void loop()
{
    throttle = phone.getThrottle();
    steer = phone.getSteering();
    String str = phone.getText();
    sliderId = phone.getSliderId();
    sliderVal = phone.getSliderVal();

    if (sliderId == 0)
    {
        Serial.print("Crane: ");
        Serial.println(sliderVal);
        serv.write(map(sliderVal, 100, 0, 0, 180));
    }

    if (prevThrottle != throttle || prevSteer != steer)
    {
        handleMovement(steer, throttle);
        prevSteer = steer;
        prevThrottle = throttle;
    }

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

void handleMovement(int _x, int _y)
{
    int x, y;

    switch (_x)
    {
    case 0:
        x = -1;
        break;

    case 49:
        x = 0;
        break;

    case 99:
        x = 1;
        break;
    }

    switch (_y)
    {
    case 0:
        y = -1;
        break;

    case 49:
        y = 0;
        break;

    case 99:
        y = 1;
        break;
    }

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

    Serial.print("x: ");
    Serial.print(x);
    Serial.print(" y: ");
    Serial.println(y);
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
    Serial.println(motor);
    Serial.println(dir);
}

void leftD(int dir)
{
    direct(0, dir);
    direct(2, dir);
}

void rightD(int dir)
{
    direct(1, dir);
    direct(3, dir);
}

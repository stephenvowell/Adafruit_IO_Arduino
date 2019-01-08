// Adafruit IO Time Tracking Cube
// Tutorial Link: https://learn.adafruit.com/time-tracking-cube
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Brent Rubell for Adafruit Industries
// Copyright (c) 2019 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// set DEBUG to True to view accel outputs
#define DEBUG false
// Used for Pizeo
#define PIEZO_PIN 0
// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

// Set up the 'orientation' feed
AdafruitIO_Feed *orientation = io.feed("orientation");

// Set up the 'minutes' feed
AdafruitIO_Feed *cubeminutes = io.feed("cubeminutes");

/* Time Tracking Cube States
 * 0: Neutral, Cube on Base
 * 1: Cube Tilted, Left on X-Axis
 * 2: Cube Tilted, Right on X-Axis 
*/
int cubeState = 0;

// Previous cube orientation state
int lastCubeState = 0;

// Tasks (change these to what you're currently working on)
String taskOne = "Write Learn Guide";
String taskTwo = "Write Code";

// Adafruit IO Sending Delay, in seconds
int sendDelay = 0.5;

// Time-Keeping
unsigned long currentTime;
unsigned long prevTime;
int seconds = 0;
int minutes = 0;
int prevMinutes = 0;

void setup()
{
  // start the serial connection
  Serial.begin(9600);
  // wait for serial monitor to open
  while (!Serial)
    ;
  Serial.println("Adafruit IO Time Tracking Cube");

  // Initialize LIS3DH
  if (!lis.begin(0x18))
  {
    Serial.println("Couldnt start");
    while (1)
      ;
  }
  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_4_G);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
}

void updateTime()
{
  currentTime = millis() / 1000;
  seconds = currentTime - prevTime;
  // increase min. timer
  if (seconds == 60)
  {
    prevTime = currentTime;
    minutes++;
  }
}

void loop()
{
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  // Update the timer
  updateTime();

  // Get a normalized sensor reading
  sensors_event_t event;
  lis.getEvent(&event);

  // Detect Cube Face Orientation
  if (event.acceleration.x > 9 && event.acceleration.x < 10)
  {
    Serial.println("Cube TILTED: Left");
    cubeState = 1;
  }
  else if (event.acceleration.x < -9)
  {
    Serial.println("Cube TILTED: Right");
    cubeState = 2;
  }
  else
  { // orientation not found
  }

  // return if the value hasn't changed
  if (cubeState == lastCubeState)
    return;

  // Send to Adafruit IO based off of the State
  switch (cubeState)
  {
  case 1:
    Serial.println("Playing Task 1 Sound...");
    tone(PIEZO_PIN, 650, 300);
    Serial.print("Sending to Adafruit IO -> ");
    Serial.println(taskOne);
    orientation->save(taskOne);
    // save minutes to a feed
    cubeminutes->save(minutes);
    // reset the timer
    minutes = 0;
    break;
  case 2:
    Serial.println("Playing Sound 2 Sound...");
    tone(PIEZO_PIN, 850, 300);
    Serial.print("Sending to Adafruit IO -> ");
    Serial.println(taskTwo);
    orientation->save(taskTwo);
    // save minutes to a feed
    cubeminutes->save(minutes);
    // reset the timer
    minutes = 0;
    break;
  }

  // store last cube orientation state
  lastCubeState = cubeState;

  // Delay the send to Adafruit IO
  delay(sendDelay * 1000);
}
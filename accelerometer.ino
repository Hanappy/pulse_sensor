/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Simple Accelerometer Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/
////////// All credit goes to author above with word of thanks.


#include <Wire.h>  // for I2C library
#include <MPU6050.h> //put this library folder in arduino-folder>libraries

MPU6050 mpu;

uint8_t Acc_XYZ_precedent[3];
uint8_t Acc_XYZ[3];
uint8_t TURN=1,ACTIVITY=0;
float IMMOBILITY_THRESHOLD;//you may need to change the value depending of the calibration done



// Part added for the project//////////////////////////////////////
void GY_87_movementDetection() 
 {
  if (TURN == 1) 
    {
       ACTIVITY = 0;
       TURN = 0;
    }
  else {
    for (int i = 0; i < 3; i++)// 0 is X axis, 1 is Y axis and 2 is Z axis
    {
      if (i==2)//if it is the Z axis 
      {IMMOBILITY_THRESHOLD=150;}
      else
      {IMMOBILITY_THRESHOLD=15;}
          
      if (fabs(Acc_XYZ_precedent[i] - Acc_XYZ[i]) >= IMMOBILITY_THRESHOLD && Acc_XYZ_precedent[i] != 0)// if the absolute value of the previous value - the next value is above the immobility value and the previous value is different from 0
        {
             ACTIVITY = 1;
        }
      }
  }
  for (int i = 0; i < 3; i++) {Acc_XYZ_precedent[i] = Acc_XYZ[i];}
 
}
///////////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(115200);

  Serial.println("Initialize MPU6050");

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // If you want, you can set accelerometer offsets
  // mpu.setAccelOffsetX();
  // mpu.setAccelOffsetY();
  // mpu.setAccelOffsetZ();
  
  checkSettings();
}

void checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:          ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:         ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  
  Serial.println();
}
float temp=0;
void loop()
{
  Vector rawAccel = mpu.readRawAccel();
  Vector normAccel = mpu.readNormalizeAccel();

  /* remove the comment to print the raw values of the accelerometer
  Serial.print("\t");
  Serial.print(rawAccel.XAxis);
  Serial.print("\t");
  Serial.print("\t ");
  Serial.print(rawAccel.YAxis);
  Serial.print(" \t ");
  Serial.println(rawAccel.ZAxis);*/
  Acc_XYZ[0] -= normAccel.XAxis*10; //X
  Acc_XYZ[1] -= normAccel.YAxis*10; //Y
  Acc_XYZ[2] -= normAccel.ZAxis*10; //Z
//////////Part added///////////////////
  GY_87_movementDetection();//function that put ACTIVITY to 1 if the person is moving
  //Serial.println("ACTIVITY");
  if(ACTIVITY == 1)
  {
    Serial.println("It has moved !");
    ACTIVITY=0;
  }
 ////////////////////////////////////////// 
 
  delay(300);
}

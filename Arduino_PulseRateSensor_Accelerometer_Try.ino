#include <Wire.h>  // for I2C library
#include <MPU6050.h> //put this library folder in arduino-folder>libraries
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.  

int sensor_pin = 0;                
                

volatile int heart_rate;          
volatile int analog_data;              
volatile int time_between_beats = 600;            
volatile boolean pulse_signal = false;    

volatile int beat[10];         //heartbeat values will be sotred in this array    
volatile int peak_value = 512;          
volatile int trough_value = 512;        
volatile int thresh = 400;              
volatile int amplitude = 100;                 

volatile boolean first_heartpulse = true;      
volatile boolean second_heartpulse = false;    
volatile unsigned long samplecounter = 0;   //This counter will tell us the pulse timing
volatile unsigned long lastBeatTime = 0;

MPU6050 mpu;
uint8_t Acc_XYZ_precedent[3];
uint8_t Acc_XYZ[3];
uint8_t TURN=1;
int ACTIVITY=0;
float IMMOBILITY_THRESHOLD;


void setup()
{       
  Serial.begin(115200);  // Initialize serial
  
  //***** FOR THE PULSE RATE SENSOR *****//         
  interruptSetup(); 

  //***** FOR THE ACCELEROMETER *****//
  Serial.println("Initialize MPU6050");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  checkSettings();
}


void loop() {

  //Serial.print("BPM: ");
  //Serial.println(heart_rate);

   Vector rawAccel = mpu.readRawAccel();
   Vector normAccel = mpu.readNormalizeAccel();

   Acc_XYZ[0] -= normAccel.XAxis*10; //X
   Acc_XYZ[1] -= normAccel.YAxis*10; //Y
   Acc_XYZ[2] -= normAccel.ZAxis*10; //Z

   GY_87_movementDetection();
   //Serial.println("ACTIVITY");
   
   if(ACTIVITY == 1){
      //Serial.println("It has moved !");

      String heart_rate_str = String(heart_rate);
      char data[heart_rate_str.length()+4];
      itoa(heart_rate, data, 10);
      data[heart_rate_str.length()] = ';';  //so that it is possible to read sensor data from the Node-MCU code, the data has
      //to be printed into the serial in a certain way.
      data[heart_rate_str.length()+1] = '1';
      data[heart_rate_str.length()+2] = '\n';
      data[heart_rate_str.length()+3] = '\0';
      
      //Serial.print(data);
      Serial.write(data);
      
      delay(1000);
      ACTIVITY=0;
   } else {

      String heart_rate_str = String(heart_rate);
      char data[heart_rate_str.length()+4];
      itoa(heart_rate, data, 10);
      data[heart_rate_str.length()] = ';';
      data[heart_rate_str.length()+1] = '0';
      data[heart_rate_str.length()+2] = '\n';
      data[heart_rate_str.length()+3] = '\0';

      //Serial.print(data);
      Serial.write(data);
      
      delay(1000);
   }
   
  //delay(200); //  take a break
  
}


//interrupt function sets up a timer that permits to interrupt every other millisecond, giving a sample rate of 500 Hz and a beat
/to beat resolution of 2mS. It will disable the PWM output on the pin 3 and 11 and disable the tone() function. sei() ensures that the global interrupts
//are enabled
void interruptSetup()
{    
  TCCR2A = 0x02;  // This will disable the PWM on pin 3 and 11
  OCR2A = 0X7C;   // This will set the top of count to 124 for the 500Hz sample rate
  TCCR2B = 0x06;  // DON'T FORCE COMPARE, 256 PRESCALER
  TIMSK2 = 0x02;  // This will enable interrupt on match between OCR2A and Timer
  sei();          // This will make sure that the global interrupts are enable
}


//this function runs after every 2mS, it takes pulse sensor reading every 2mS and increments the sample counter, which is used to keep track
//of the time. N variable is used to avoid the noise

ISR(TIMER2_COMPA_vect)
{ 
  cli();                                     
  analog_data = analogRead(sensor_pin);  //read pulse sensor analogue data           
  samplecounter += 2;                        
  int N = samplecounter - lastBeatTime;      

  //Keeps track of the highest and lowest values. Thresh variable was adjusted to an initial value of 400 and aims to keep track of the middle
  //point of the analogue input
  if(analog_data < thresh && N > (time_between_beats/5)*3)
    {     
      if (analog_data < trough_value)
      {                       
        trough_value = analog_data;
      }

    }


  if(analog_data > thresh && analog_data > peak_value)
    {        
      peak_value = analog_data;
    }                          


   //here a heart-beat is aimed to find. If the output is above the threshold value and 3/5 of the last time between the bats has passed, then the pulse signal will
   //be true. Lastbeattime is updated by calculating the time since the last beat
   if (N > 250)
  {                            
    if ( (analog_data > thresh) && (pulse_signal == false) && (N > (time_between_beats/5)*3) )
      {       
        pulse_signal = true;          
        time_between_beats = samplecounter - lastBeatTime;
        lastBeatTime = samplecounter;     



       if(second_heartpulse)
        {                        
          second_heartpulse = false;   
          for(int i=0; i<=9; i++)    
          {            
            beat[i] = time_between_beats; //it gets 10 consecutive heart beat values                
          }

        }


        if(first_heartpulse)
        {                        
          first_heartpulse = false;
          second_heartpulse = true;
          sei();            
          return;           

        }  


      word runningTotal = 0;  


      for(int i=0; i<=8; i++)
        {               
          beat[i] = beat[i+1];
          runningTotal += beat[i];
        }

      beat[9] = time_between_beats;             
      runningTotal += beat[9];    //the aim is to calculate the mean value of the RR intervals between 10 consecutive
      //peaks, so all the intervals are summed and then divided by 10
      runningTotal /= 10;        
      heart_rate = 60000/runningTotal; //knowing how many beats have occured in the timeframe "runningTotal" it is estimated how 
      //many have occured within one minute, to get the beats-per-minute
    }                      

  }
  //when the pulse value is less than the threshold value, the threshold variable is updated
  if (analog_data < thresh && pulse_signal == true)
    {  
      pulse_signal = false;             
      amplitude = peak_value - trough_value;
      thresh = amplitude/2 + trough_value; 
      peak_value = thresh;           
      trough_value = thresh;

    }
  //if no beat is found after 2.5mS the variables are initialized to the initial values
  if (N > 2500)
    {                          
      thresh = 512;                     
      peak_value = 512;                 
      trough_value = 512;               
      lastBeatTime = samplecounter;     
      first_heartpulse = true;                 
      second_heartpulse = false;               
    }


  sei();                                

};


void GY_87_movementDetection() 
 {
  if (TURN == 1) 
    {
       ACTIVITY = 0;
       TURN = 0;
    }
  else {
    for (int i = 0; i < 3; i++)
    {
      if (i==2)
      {
        IMMOBILITY_THRESHOLD=150;
      }
      else
      {
        IMMOBILITY_THRESHOLD=15;
      }
      //Serial.println(i);
      //Serial.println(fabs(Acc_XYZ_precedent[i] - Acc_XYZ[i]));
      
      if (fabs(Acc_XYZ_precedent[i] - Acc_XYZ[i]) >= IMMOBILITY_THRESHOLD && Acc_XYZ_precedent[i] != 0)
      {   
        ACTIVITY = 1;
      }
      
    }
  }
  for (int i = 0; i < 3; i++) {Acc_XYZ_precedent[i] = Acc_XYZ[i];}
 
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

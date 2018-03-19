/*
This is the FindMaxChange
this version will try to find the highest number that kick a change and swich from that when running by the sensor.
*/
//#define __DEBUG__
#if defined (__DEBUG__)
#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial MySerial(3);
#endif

#define __USE_COIL_DRIVEN__
#define __USE_COIL_MONITOR__
#if defined (__AVR_ATtiny85__)
  #define SENSOR_PORT_1 A1
  #define SENSOR_PORT_2 A2
  #define SENSOR_PORT_3 A3
  #define C1 0
  #define C2 1
  #define C3 5
#else
  //Arduino
  #define SENSOR_PORT_1 A2
  #define SENSOR_PORT_2 A1
  #define SENSOR_PORT_3 A0
  #define LED_BUILTIN 13
  #define C1 8
  #define C2 9
  #define C3 10
  #define MySerial Serial
#endif

#define INITIAL_PERIOD 1000
#define MIN_PERIOD 14
#define CHANGE_PERIOD_TIME 1000

#define SENSOR_DRIVEN_LIMIT 100
#define SENSOR_TEST_NUMBER 1000

unsigned long timeOld = millis();
unsigned long timeNew = millis();
unsigned long period = INITIAL_PERIOD;
unsigned long timeToChangePeriod = millis() + CHANGE_PERIOD_TIME;

#define DEBUG_TIME_QUIET 10000
unsigned long debugStartTime = 0;


byte currentPosition = 3;


int sensorValue = 0;
int sensorPrevValue = 0;

int sensorIdx = 0;
int sensorMin = 0;
int sensorMax = 0;
int sensorMinNext = 0;
int sensorMaxNext = 0;

boolean sensorDriven = false;

int curveDirection = 0;
int sensorPrevValue1 = 0;
int sensorPrevValue2 = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.


  pinMode(C1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(C3, OUTPUT);
  digitalWrite(C1,LOW);
  digitalWrite(C2,LOW);
  digitalWrite(C3,LOW);

#if defined (__DEBUG__)
  MySerial.begin(19200);
  MySerial.println(F("Setup attiny85"));
/*
  int sensorValue = analogRead(SENSOR_PORT_1);
  float voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A0:")); MySerial.println(voltage);
*/  
  sensorValue = analogRead(A2);
  
  double voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A2:")); MySerial.println(voltage);
  /*
  sensorValue = analogRead(SENSOR_PORT_3);
  voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A2:")); MySerial.println(voltage);
  */
#endif  
  
#if defined (__DEBUG__)
  MySerial.println(F("Test Position 1"));
#endif  

 digitalWrite(C1, HIGH); 
 delay(2000);
 digitalWrite(C1,LOW);
 delay(2000);

#if defined (__DEBUG__)
  MySerial.println(F("Test Position 2"));
#endif  

 digitalWrite(C2, HIGH); 
 delay(2000);
 digitalWrite(C2,LOW);
 delay(2000);

#if defined (__DEBUG__)
  MySerial.println(F("Test Position 3"));
#endif  

 digitalWrite(C3, HIGH); 
 delay(2000);
 digitalWrite(C3,LOW);
 delay(2000);


  //setup time variables
  timeNew = millis();
  timeOld = timeNew;

  timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;
  debugStartTime = timeNew + DEBUG_TIME_QUIET;

#if defined (__DEBUG__)
  MySerial.print(F("MIN_PERIOD:"));
  MySerial.println(MIN_PERIOD);
  MySerial.println(F("Starting attiny85 debug 1"));
#endif
}

// the loop function runs over and over again forever

void loop() {
    timeNew = millis();

    sensorPrevValue = sensorValue;
    
    if( 1 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_2);
    }
    
    else if( 2 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_3);
    }
    else if( 3 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_1);
    } 
    
    //sensorValue = analogRead(A2);
    
  // calculate sensorMax and sensorMin 
  if(  abs(sensorValue - sensorPrevValue) < 20 && (timeNew - timeOld) > 3){
    if( sensorIdx >= SENSOR_TEST_NUMBER  ){
      sensorIdx = 0;
      sensorMin = sensorMinNext;
      sensorMax = sensorMaxNext;
      sensorMinNext = sensorValue;
      sensorMaxNext = sensorValue;
    }
    else {   
      sensorIdx++;
      if(sensorValue > sensorMaxNext){
        sensorMaxNext = sensorValue;
      }
      if(sensorValue < sensorMinNext){
        sensorMinNext = sensorValue;
      }    
    }
  }


  //calculate curve direction and keep prev sensor values
  if( abs(sensorValue - sensorPrevValue) < 20  &&  ((timeNew - timeOld) > 3) ){   
    if ( sensorValue > sensorPrevValue1 && sensorPrevValue1 >= sensorPrevValue2 ){
          curveDirection = 1;  
    }
    else if( sensorValue < sensorPrevValue1 && sensorPrevValue1 <= sensorPrevValue2  )
         curveDirection = -1;
    else curveDirection = 0; //invalid     
         
  }
  else curveDirection = 0; //it is invalid
  
  sensorPrevValue2 = sensorPrevValue1;
  sensorPrevValue1 = sensorValue;

#if defined (__USE_COIL_DRIVEN__)
  if( sensorDriven == false && (sensorMax - sensorMin) > SENSOR_DRIVEN_LIMIT && period < 20 ){
      sensorDriven = true;
#if defined (__DEBUG__)
      MySerial.println(F("CD"));
#endif
  }
#endif


#if defined (__USE_COIL_MONITOR__)  
    if( true == sensorDriven && (sensorMax - sensorMin) < (double)SENSOR_DRIVEN_LIMIT*0.8 ){ // something when wrong
      digitalWrite(C1, LOW);
      digitalWrite(C2, LOW);
      digitalWrite(C3, LOW);


#if defined (__DEBUG__)
      MySerial.println(F("***********************   Error while sensor driving elapsed"));
      MySerial.print(F("\tMinValue:\t"));
      MySerial.print(sensorMinValue[currentPosition-1]);
      MySerial.print(F("\tMaxValue:\t"));
      MySerial.print(sensorMaxValue[currentPosition-1]);
      MySerial.print(F("\tSpan:\t"));
      MySerial.print((sensorMaxValue[currentPosition-1] - sensorMinValue[currentPosition-1]));
      MySerial.print(F("\tsensorDriven\t"));
      MySerial.print(sensorDriven);
      MySerial.print(F("\tperiod:\t"));
      MySerial.println(period);
      MySerial.print(F("currentPosition")); 
      MySerial.println(currentPosition);
      
#endif
      delay(10000);
      period = INITIAL_PERIOD;
      timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;  
      sensorMin = sensorValue;
      sensorMax = sensorValue;      
      sensorDriven = false;
    }
#endif

  //should I acelerate
  if( timeNew > timeToChangePeriod && period > MIN_PERIOD){
    if(period < 30)
      period = period - 1;
    else period = period * 0.90;
    timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;

#if defined (__DEBUG__)
    MySerial.print("p:");
    MySerial.print(period);
    
    MySerial.print("\tMin:");
    MySerial.print(sensorMin);
    MySerial.print("\tMax");
    MySerial.print(sensorMax);
    MySerial.print("\ts:");
    MySerial.println(sensorMax-sensorMin);
    
//    MySerial.print("1 L:"); MySerial.print(sensorMinValue[0]);MySerial.print("\tH:"); MySerial.println(sensorMaxValue[0]);
    //MySerial.print("2 L:"); MySerial.print(sensorMinValue[1]);MySerial.print("\tH:"); MySerial.println(sensorMaxValue[1]);
//    MySerial.print("3 L:"); MySerial.print(sensorMinValue[2]);MySerial.print("\tH:"); MySerial.println(sensorMaxValue[2]);
    sensorMinValue[0] = 0;
    sensorMinValue[1] = 0;
    sensorMinValue[2] = 0;
    sensorMaxValue[0] = 1024;
    sensorMaxValue[1] = 1024;
    sensorMaxValue[2] = 1024;
    //MySerial.print(F("\ttimeNew:"));
    //MySerial.print(timeNew);
    //MySerial.print(F("\ttimeToChangePeriod:"));
    //MySerial.println(timeToChangePeriod);
#endif    
  }  
 

 
  if( sensorDriven == true ){

    double perc = (double)(sensorValue-sensorMin)/(sensorMax - sensorMin);
    
    if(  currentPosition == 3  && curveDirection == 1 && perc < 0.3){ 
      //go to 1
#ifndef __AVR_ATtiny85__      
        digitalWrite(LED_BUILTIN, HIGH);
#endif      
        digitalWrite(C1, HIGH);
        digitalWrite(C2, LOW);
        digitalWrite(C3, LOW);

        currentPosition = 1;
      timeOld = timeNew;
     
    }
    else if( currentPosition == 1 && curveDirection == 1 && perc < 0.3){
      //go to 2
#ifndef __AVR_ATtiny85__      
        digitalWrite(LED_BUILTIN, LOW);
#endif        
        digitalWrite(C2, HIGH);  
        digitalWrite(C1, LOW);
        digitalWrite(C3, LOW);        

        currentPosition = 2; 
      timeOld = timeNew;

    }
    else if( currentPosition == 2  && curveDirection == 1 && perc < 0.5){ 
          //go to 3  

        digitalWrite(C3, HIGH);
        digitalWrite(C2, LOW);
        digitalWrite(C1, LOW);

        currentPosition = 3;  
        
      timeOld = timeNew;

    }
  }
  else
    if( (timeNew - timeOld) > period ){ //this is time driven
      if(3 == currentPosition ){
        digitalWrite(C1, HIGH);
        digitalWrite(C2, LOW);
        digitalWrite(C3, LOW);

        currentPosition = 1;
      }  
      else if(1 == currentPosition ){
        digitalWrite(C2, HIGH);  
        digitalWrite(C1, LOW);
        digitalWrite(C3, LOW);        
        
        currentPosition = 2; 
      }  
      else if(2 == currentPosition ){
        digitalWrite(C3, HIGH);
        digitalWrite(C2, LOW);
        digitalWrite(C1, LOW);
        
        currentPosition = 3;  
      }
      timeOld = timeNew;
    }
/*    
#if defined (__USE_COIL_MONITOR__)    
    if( (timeNew - timeOld) > 100 ){ //this is protection to shootdown the coils y if the period is to long
        digitalWrite(C1, LOW);
        digitalWrite(C2, LOW);
        digitalWrite(C3, LOW);
        sensorDriven = false;
        period = INITIAL_PERIOD;
        timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;  
        sensorMinValue = sensorValue;
        sensorMaxValue = sensorValue;      
    }
#endif 
   */
 // //MySerial.println(timeNew);
}


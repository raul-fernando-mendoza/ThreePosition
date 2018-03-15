/*
This is the find limits branch
working version stable 
a little slow than compare to timing
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
#define SENSOR_TEST_NUMBER 5000

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
int sensorMinValue[3];
int sensorMaxValue[3];
int sensorMinValueNext[3];
int sensorMaxValueNext[3];

boolean coilDriven = false;


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

  int sensorValue = analogRead(SENSOR_PORT_1);
  float voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A0:")); MySerial.println(voltage);
  
  sensorValue = analogRead(SENSOR_PORT_2);
  voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A1:")); MySerial.println(voltage);
  
  sensorValue = analogRead(SENSOR_PORT_3);
  voltage = ( sensorValue * (5.0 / 1023.0) )  ;
  MySerial.print(F("A2:")); MySerial.println(voltage);
  
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
      sensorValue = analogRead(SENSOR_PORT_3);
      //sensorValue = analogRead(A2);
    }
    else if( 2 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_1);
      //sensorValue = analogRead(A2);
    }
    else if( 3 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_2);
      //sensorValue = analogRead(A2);
    } 
    
    
    
  // calculate sensorMax and sensorMin 
  if(  abs(sensorValue - sensorPrevValue) < 20 && (timeNew - timeOld) > 2){
    if( sensorIdx >= SENSOR_TEST_NUMBER  ){
      sensorIdx = 0;
      if( coilDriven == 0 ){
        sensorMinValue[currentPosition-1] = sensorMinValueNext[currentPosition-1];
        sensorMaxValue[currentPosition-1] = sensorMaxValueNext[currentPosition-1];
        sensorMinValueNext[currentPosition-1] = sensorValue;    
        sensorMaxValueNext[currentPosition-1] = sensorValue;
      }
      sensorMin = sensorMinNext;
      sensorMax = sensorMaxNext;
      sensorMinNext = sensorValue;
      sensorMaxNext = sensorValue;
      /*
      if( timeNew > debugStartTime  ){
        MySerial.print(F("m\t"));MySerial.print(sensorMinValue[currentPosition-1]);
        MySerial.print(F("\tM\t"));MySerial.print(sensorMaxValue[currentPosition-1]);
        MySerial.print(F("\ts\t"));MySerial.println(sensorMaxValue[currentPosition-1]-sensorMinValue[currentPosition-1]);
        debugStartTime = timeNew+DEBUG_TIME_QUIET;
      }
      */
    }
    else {   
      sensorIdx++;
      if( coilDriven == 0 ){
        if( sensorValue > sensorMaxValueNext[currentPosition-1] ){
           sensorMaxValueNext[currentPosition-1] = sensorValue;
        }   
        if( sensorValue < sensorMinValueNext[currentPosition-1] ){
           sensorMinValueNext[currentPosition-1] = sensorValue;
        }   
      }  
      if(sensorValue > sensorMaxNext){
        sensorMaxNext = sensorValue;
      }
      if(sensorValue < sensorMinNext){
        sensorMinNext = sensorValue;
      }    
    }
  }



#if defined (__USE_COIL_DRIVEN__)
  //calculate if the coild driven should start
  if( coilDriven == false && (sensorMaxValue[currentPosition-1] - sensorMinValue[currentPosition-1]) > SENSOR_DRIVEN_LIMIT && period <= MIN_PERIOD+3 ){
      coilDriven = true;
#if defined (__DEBUG__)
      MySerial.println(F("CD"));
#endif
  }
#endif


#if defined (__USE_COIL_MONITOR__)  
    if( true == coilDriven && (sensorMax - sensorMin) < (double)SENSOR_DRIVEN_LIMIT*0.8 ){ // something when wrong
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
      MySerial.print(F("\tcoilDriven\t"));
      MySerial.print(coilDriven);
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
      coilDriven = false;
    }
#endif


  //should I acelerate
  if( timeNew > timeToChangePeriod && period > MIN_PERIOD){
    if(period < 25)
      period = period - 1;
    else period = period * 0.90;
    timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;
 /*   //MySerial.print(F("period:"));
    //MySerial.print(period);
    //MySerial.print(F("\ttimeNew:"));
    //MySerial.print(timeNew);
    //MySerial.print(F("\ttimeToChangePeriod:"));
    //MySerial.println(timeToChangePeriod);
 */   
    
  }  
 
 
  if( coilDriven == true ){
    double percent = (double)(sensorValue-sensorMinValue[currentPosition-1])/(double)(sensorMaxValue[currentPosition-1]-sensorMinValue[currentPosition-1]);
    
    if( currentPosition == 3  && percent < 0.1 ){ // && curveDirection == 1
      //go to 1
#ifndef __AVR_ATtiny85__      
        digitalWrite(LED_BUILTIN, HIGH);
#endif      
        digitalWrite(C3, LOW);
        digitalWrite(C1, HIGH);
        digitalWrite(C2, HIGH);

        currentPosition = 1;
      timeOld = timeNew;
     
    }
    else if( currentPosition == 1  && percent < 0.1 ){// && curveDirection == 1
      //go to 2
#ifndef __AVR_ATtiny85__      
        digitalWrite(LED_BUILTIN, LOW);
#endif        
        digitalWrite(C1, LOW);
        digitalWrite(C2, HIGH);        
        digitalWrite(C3, HIGH);        

        currentPosition = 2; 
      timeOld = timeNew;

    }
    else if( currentPosition == 2   && percent < 0.1 ){ //&& curveDirection == 1
          //go to 3  

        digitalWrite(C2, LOW);
        digitalWrite(C3, HIGH);
        digitalWrite(C1, HIGH);

        currentPosition = 3;  
        
      timeOld = timeNew;

    }
  }
  else
    if( (timeNew - timeOld) > period ){ //this is time driven

      if(3 == currentPosition ){
        
        if( timeNew > debugStartTime ){
          //MySerial.print("3\t");MySerial.println((sensorValue-sensorMinValue[currentPosition-1])/(sensorMaxValue[currentPosition-1]-sensorMinValue[currentPosition-1]));
#if defined (__DEBUG__)
          MySerial.print(currentPosition); MySerial.print("\t"); MySerial.print(sensorValue); MySerial.print("\t"); MySerial.print(sensorMinValue[currentPosition-1]); MySerial.print("\t"); MySerial.println(sensorMaxValue[currentPosition-1]);
          debugStartTime = timeNew + DEBUG_TIME_QUIET;
#endif
        }
       //digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(C3, LOW);
        digitalWrite(C1, HIGH);
        digitalWrite(C2, HIGH);
        currentPosition = 1;
      }  
      else if(1 == currentPosition ){
        if( timeNew > debugStartTime ){
          //MySerial.print("1\t");MySerial.println((sensorValue-sensorMinValue[currentPosition-1])/(sensorMaxValue[currentPosition-1]-sensorMinValue[currentPosition-1]));
#if defined (__DEBUG__)
          MySerial.print(currentPosition); MySerial.print("\t"); MySerial.print(sensorValue); MySerial.print("\t"); MySerial.print(sensorMinValue[currentPosition-1]);MySerial.print("\t"); MySerial.println(sensorMaxValue[currentPosition-1]);
#endif
          debugStartTime = timeNew + DEBUG_TIME_QUIET;
        }
        ////MySerial.println(F("2"));
        //digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(C1, LOW);
        digitalWrite(C2, HIGH);  
        digitalWrite(C3, HIGH);        

        currentPosition = 2; 
      }  
      else if(2 == currentPosition ){
        if( timeNew > debugStartTime ){
          //MySerial.print("2\t");MySerial.println((sensorValue-sensorMinValue[currentPosition-1])/(sensorMaxValue[currentPosition-1]-sensorMinValue[currentPosition-1]));
#if defined (__DEBUG__)
          MySerial.print(currentPosition); MySerial.print("\t"); MySerial.print(sensorValue); MySerial.print("\t"); MySerial.print(sensorMinValue[currentPosition-1]);MySerial.print("\t");MySerial.println(sensorMaxValue[currentPosition-1]);
#endif          
          debugStartTime = timeNew + DEBUG_TIME_QUIET;
        }
        ////MySerial.println(F("3"));
        
        //digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(C2, LOW);
        digitalWrite(C3, HIGH);
        digitalWrite(C1, HIGH);
        
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
        coilDriven = false;
        period = INITIAL_PERIOD;
        timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;  
        sensorMinValue = sensorValue;
        sensorMaxValue = sensorValue;      
    }
#endif 
   */
 // //MySerial.println(timeNew);
}


/*
This is the ATTiny44A
this version will try to find the highest number that kick a change and swich from that when running by the sensor.
*/
#define __DEBUG__
#if defined (__DEBUG__)
#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial MySerial(3);
#endif
/*
  #include <SoftwareSerial.h>

  SoftwareSerial MySerial(0, 3); // RX, TX
*/
  

//#define __USE_COIL_DRIVEN__
//#define __USE_COIL_MONITOR__

#define SENSOR_PORT_1 A0
#define SENSOR_PORT_2 A1
#define SENSOR_PORT_3 A2
#define C1 10
#define C2 9
#define C3 8

#define INITIAL_PERIOD 1000
#define MIN_PERIOD 18
#define CHANGE_PERIOD_TIME 1000

#define SENSOR_DRIVEN_LIMIT 100
#define SENSOR_TEST_NUMBER 1000

unsigned long timeOld;
unsigned long timeNew;
unsigned long period = INITIAL_PERIOD;
unsigned long timeToChangePeriod;

#define DEBUG_TIME_QUIET 10000
#define DEBUG_SIZE 30
unsigned long debugStartTime = 0;
byte debugIdx = 0;
byte debugIdxToPrint = 0;
byte logIdx = 0;
//unsigned long logTime[DEBUG_SIZE];
byte logSensor[DEBUG_SIZE]; //sensorvalue
byte logCurrentPosition[DEBUG_SIZE];
//byte logCurveDirection[DEBUG_SIZE];
boolean debugActivated = false;


byte currentPosition = 3;

unsigned int sensorValue;
unsigned int sensorPrevValue;

unsigned int sensorIdx;
unsigned int sensorMin;
unsigned int sensorMax;
unsigned int sensorMinNext;
unsigned int sensorMaxNext;

boolean sensorDriven = false;

byte curveDirection;
int sensorPrevValue1;
int sensorPrevValue2;

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
#endif  
/*  
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
*/

  //setup time variables
  timeNew = millis();
  timeOld = timeNew;

  timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;
  debugStartTime = timeNew + DEBUG_TIME_QUIET;

#if defined (__DEBUG__)
/*
  MySerial.print(F("MIN_PERIOD:"));
  MySerial.println(MIN_PERIOD);
  MySerial.println("attiny44");

  MySerial.println( analogRead(SENSOR_PORT_1) );
  MySerial.println( analogRead(SENSOR_PORT_2) );
  MySerial.println( analogRead(SENSOR_PORT_3) );
*/  
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

//    sensorValue = analogRead(SENSOR_PORT_1);
/*
 if (timeNew >= debugStartTime ){ 
    debugStartTime =  timeNew + DEBUG_TIME_QUIET;
    MySerial.print(currentPosition);
    MySerial.print("\t");
    MySerial.println(sensorValue);
 }
 */    
    //sensorValue = analogRead(A2);
    
  // calculate sensorMax and sensorMin 
  if(  abs(sensorValue - sensorPrevValue) < 20 && (timeNew - timeOld) > 3){
    if( sensorIdx >= SENSOR_TEST_NUMBER  ){
      sensorIdx = 0;
      sensorMin = sensorMinNext;
      sensorMax = sensorMaxNext;
      sensorMinNext = sensorValue;
      sensorMaxNext = sensorValue;
/*
      if (timeNew >= debugStartTime ){ 
        debugStartTime =  timeNew + DEBUG_TIME_QUIET;
        MySerial.print(sensorMin);
        MySerial.print(" ");
        MySerial.print(sensorMax);
        MySerial.print(" ");
        MySerial.println(sensorMax-sensorMin);
      }     
 */    
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
    if ( sensorValue > sensorPrevValue1 && sensorPrevValue1 > sensorPrevValue2 ){
          curveDirection = 1;  
    }
    else if( sensorValue < sensorPrevValue1 && sensorPrevValue1 < sensorPrevValue2  )
         curveDirection = 2;
    else curveDirection = 0; //invalid     
  }
  else curveDirection = 0; //it is invalid
  
  sensorPrevValue2 = sensorPrevValue1;
  sensorPrevValue1 = sensorValue;

#if defined (__USE_COIL_DRIVEN__)
  if( sensorDriven == false && (sensorMax - sensorMin) > SENSOR_DRIVEN_LIMIT && period < 20 ){
      sensorDriven = true;
      
#if defined (__DEBUG__)
      MySerial.println("CD");
#endif
  }
#endif

/*
#if defined (__USE_COIL_MONITOR__)  
    if( true == sensorDriven && (sensorMax - sensorMin) < (double)SENSOR_DRIVEN_LIMIT*0.8 ){ // something when wrong
      digitalWrite(C1, LOW);
      digitalWrite(C2, LOW);
      digitalWrite(C3, LOW);


#if defined (__DEBUG__)
      MySerial.println(F("***********************   Error while sensor driving elapsed"));
      MySerial.print(F("\tMinValue:\t"));
      MySerial.print(sensorMin);
      MySerial.print(F("\tMaxValue:\t"));
      MySerial.print(sensorMax);
      MySerial.print(F("\tSpan:\t"));
      MySerial.print(sensorMax - sensorMin);
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
*/
  //should I acelerate
  if( timeNew > timeToChangePeriod && period > MIN_PERIOD){
    if(period < 30)
      period = period - 1;
    else period = period * 0.90;
    timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;

#if defined (__DEBUG__)
/*
    MySerial.print("p:");
    MySerial.println(period);
    
    MySerial.print("\tMin:");
    MySerial.print(sensorMin);
    MySerial.print("\tMax");
    MySerial.print(sensorMax);
    MySerial.print("\ts:");
    MySerial.println(sensorMax-sensorMin);    
*/    
#endif    

  }  


#if defined (__USE_COIL_DRIVEN__) 
  if( sensorDriven == true ){
    byte perc = ((float)(sensorValue-sensorMin)/(sensorMax-sensorMin))*100;
    
    if(  currentPosition == 3 && curveDirection == 1  && perc < 30){ //&&  && curveDirection != 2
      //go to 1
        digitalWrite(C1, HIGH);
        digitalWrite(C2, LOW);
        digitalWrite(C3, LOW);

        currentPosition = 1;
        timeOld = timeNew;
     
    }
    else if( currentPosition == 1 && curveDirection == 1 && perc < 30){ //&& 
      //go to 2      
      digitalWrite(C2, HIGH);  
      digitalWrite(C1, LOW);
      digitalWrite(C3, LOW);        

      currentPosition = 2; 
      timeOld = timeNew;

    }
    else if( currentPosition == 2 && curveDirection == 1  && perc < 50){  // && curveDirection != 2
      //go to 3  
      digitalWrite(C3, HIGH);
      digitalWrite(C2, LOW);
      digitalWrite(C1, LOW);

      currentPosition = 3;  
      timeOld = timeNew;
    }
  }
  else
#endif  
    if( (timeNew - timeOld) > period ){ //this is time driven
/*
      if (timeNew >= debugStartTime ){ 
        debugStartTime =  timeNew + DEBUG_TIME_QUIET;
        int f = ((float)(sensorValue-sensorMin)/(sensorMax-sensorMin))*100;
        MySerial.print(sensorValue);
        MySerial.print("\t");
        MySerial.print(sensorMin);
        MySerial.print("\t");
        MySerial.print(sensorMax);
        MySerial.print("\t");
        MySerial.print(f);
        MySerial.print("\t");
        MySerial.print(currentPosition);
        MySerial.print("\t");
        MySerial.println(curveDirection);
      }
  */    
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


  // this section controls de debug  
  //first save the current values for debugging 
  if( logIdx + 1 < DEBUG_SIZE  ) logIdx++; else logIdx=0;
//  logTime[logIdx] = timeNew;
  byte perc = ((float)(sensorValue-sensorMin)/(sensorMax-sensorMin))*100;;
  logSensor[logIdx] = perc;
  logCurrentPosition[logIdx] = currentPosition; 
//  logCurveDirection[logIdx] = curveDirection;
  

  if (timeNew >= debugStartTime && false == debugActivated && false == sensorDriven  ){ //&& period <= MIN_PERIOD
    debugActivated = true;
//    Serial.println("***************************************");
//    Serial.print("timeNew:");
//    Serial.println(timeNew);

    debugStartTime =  timeNew + DEBUG_TIME_QUIET;

//    Serial.print("debugStartTime:");
//    Serial.println(debugStartTime);

//    Serial.print("logIdx");
//    Serial.println(logIdx);

    debugIdx = 0; 
    if( logIdx + 1 < DEBUG_SIZE )
      debugIdxToPrint = logIdx + 1;
    else debugIdxToPrint = 0;

#if defined (__DEBUG__)  
/*

    //MySerial.print("\tMin:\t");
//    MySerial.print(sensorMin);
//    MySerial.print("\t");
//    MySerial.println(sensorMax);
 
    MySerial.print("debugIdxToPrint:\t");
    MySerial.print(debugIdxToPrint);
    MySerial.print("\tcoilDriven\t");
    MySerial.print(sensorDriven);
    MySerial.print("\tperiod:\t");
    MySerial.println(period);

    MySerial.print("logIdx");
    MySerial.print("\ttimeNew");
    MySerial.print("\tsensorValue");
    MySerial.print("\tposition");
    
    MySerial.print("\tcurve");
*/
    MySerial.print("\n");
    
#endif
  }
  else if( true==debugActivated){
    if(debugIdx < DEBUG_SIZE ){
      debugIdx++;
      //Serial.print("debugIdx:"); Serial.println(debugIdx);
      if( debugIdxToPrint + 1 < DEBUG_SIZE )
        debugIdxToPrint = logIdx + 1;
      else debugIdxToPrint = 0;
      //Serial.print("\tdebugIdxToPrint:"); Serial.println(debugIdxToPrint);
    }
    else{
      debugActivated = false;
    }
  }
  

  if( true == debugActivated ) { 
    //calculate the index to print from the debugIndex
#if defined (__DEBUG__)
//    MySerial.print(debugIdxToPrint);
//    MySerial.print("\t");
    
//    MySerial.print(logTime[debugIdxToPrint]);
//    MySerial.print("\t");

    MySerial.print(logSensor[debugIdxToPrint]);
    MySerial.print("\t");
    MySerial.print(logCurrentPosition[debugIdxToPrint]);    
    MySerial.print("\t");
/*    
    MySerial.print(logCurveDirection[debugIdxToPrint]);
    MySerial.print("\t");
*/
    MySerial.print(sensorMin);
    MySerial.print("\t");
    MySerial.println(sensorMax);
    
#endif
    
  }
  // end debug logic 
    
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
 //MySerial.println(timeNew);
}


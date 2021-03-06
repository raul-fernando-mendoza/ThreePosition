/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/
/*
#include <SendOnlySoftwareMySerial.h>
SendOnlySoftwareSerial //MySerial.(3);
*/

#define __DEBUG__
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

#define INITIAL_PERIOD 100
#define MIN_PERIOD 14
#define CHANGE_PERIOD_TIME 1000

#define DEBUG_TIME_QUIET 10000
#define SENSOR_DRIVEN_LIMIT 60
#define SENSOR_TEST_NUMBER 1000

unsigned long timeOld = millis();
unsigned long timeNew = millis();
unsigned long period = INITIAL_PERIOD;
unsigned long timeToChangePeriod = millis() + CHANGE_PERIOD_TIME;

byte currentPosition = 3;


int sensorValue = 0;
int sensorValue1 = 0;
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorMinValue = 0;
int sensorMaxValue = 0;
int sensorMinValueNext = 0;
int sensorMaxValueNext = 0;
int sensorIdx = 0;
int sensorPrevValue = 0;
int sensorPrevValue1 = 0;
int sensorPrevValue2 = 0;

byte currentDegrees = 0;

byte curveDirection = 0;

boolean coilDriven = false;

#define DEBUG_SIZE 100 // 35 for ittiny
unsigned long debugStartTime = 0;
byte debugIdx = 0;
byte debugIdxToPrint = 0;
unsigned long logTime[DEBUG_SIZE];
int logSensor[DEBUG_SIZE]; //sensorvalue

byte logCalculated[DEBUG_SIZE][3]; //0 currentPosition, 1 currentDegrees, 2 directions
boolean debugActivated = false;
byte  logIdx = 0;


#ifndef __AVR_ATtiny85__
unsigned long timeToReportSpeed = 0;
long timeLastTurn = 0;
#endif


// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.


  pinMode(C1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(C3, OUTPUT);
  digitalWrite(C1,LOW);
  digitalWrite(C2,LOW);
  digitalWrite(C3,LOW);

#ifndef __AVR_ATtiny85__      
    for(int i=0; i<10; i++){   
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
    };    
#endif   
#if defined (__DEBUG__)
  pinMode(LED_BUILTIN, OUTPUT);
  MySerial.begin(19200);
  MySerial.println(F("Setup attiny85"));
#endif  

#if defined (__DEBUG__)
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
  

  delay(2000);

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


  pinMode(SENSOR_PORT_1,INPUT);
  pinMode(SENSOR_PORT_2,INPUT);
  pinMode(SENSOR_PORT_3,INPUT);


  //initalize send min and max used to set the sensor driven flag
  sensorMinValue = analogRead(SENSOR_PORT_1);
  sensorMaxValue = analogRead(SENSOR_PORT_1);



  //debug initialize
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
    }
    else if( 2 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_1);
    }
    else if( 3 == currentPosition ){
      sensorValue = analogRead(SENSOR_PORT_2);
    } 
    
  // calculate sensorMax and sensorMin 
  if( abs(sensorValue - sensorPrevValue) < 20 && (timeNew - timeOld) > 2){
    if( sensorIdx >= SENSOR_TEST_NUMBER  ){
      sensorIdx = 0;
      sensorMinValue = sensorMinValueNext;
      sensorMaxValue = sensorMaxValueNext;
   //     MySerial.print("MinValue:");
   //     MySerial.print(sensorMinValue);
   //     MySerial.print("\tMaxValue:");
   //     MySerial.println(sensorMaxValue);      
      sensorMinValueNext = sensorValue;    
      sensorMaxValueNext = sensorValue;
    }
    else {   
      sensorIdx++;
      if( sensorValue > sensorMaxValueNext ){
         sensorMaxValueNext = sensorValue;
      }   
      if( sensorValue < sensorMinValueNext ){
         sensorMinValueNext = sensorValue;
      }   
    }
  }

  //calculate current degrees
  currentDegrees = ((double(sensorValue) - double(sensorMinValue)) * 180) / (sensorMaxValue - sensorMinValue);
  
  //calculate curve direction and keep prev sensor values
  if( abs(sensorValue - sensorPrevValue) < 20  &&  ((timeNew - timeOld) > 5) ){   
    if ( sensorValue > sensorPrevValue1 && sensorPrevValue1 >= sensorPrevValue2 ){
          curveDirection = 1;  
    }
    else if( sensorValue < sensorPrevValue1 && sensorPrevValue1 <= sensorPrevValue2  )
         curveDirection = 2;
         
  }
  else curveDirection = 0; //it is invalid
  

  sensorPrevValue2 = sensorPrevValue1;
  sensorPrevValue1 = sensorValue;

#if defined (__USE_COIL_DRIVEN__)
  //calculate if the coild driven should start
  if( coilDriven == false && (sensorMaxValue - sensorMinValue) > SENSOR_DRIVEN_LIMIT && period < 30 ){
      coilDriven = true;
#if defined (__DEBUG__)
      MySerial.println("CD");
#endif
  }
#endif

#if defined (__USE_COIL_MONITOR__)  
    if( true == coilDriven && (sensorMaxValue - sensorMinValue) < SENSOR_DRIVEN_LIMIT*0.8 ){ // something when wrong
      digitalWrite(C1, LOW);
      digitalWrite(C2, LOW);
      digitalWrite(C3, LOW);


#if defined (__DEBUG__)

      
      MySerial.println("***********************   Error while sensor driving elapsed");

      MySerial.print(F("\tMinValue:\t"));
      MySerial.print(sensorMinValue);
      MySerial.print(F("\tMaxValue:\t"));
      MySerial.print(sensorMaxValue);
      MySerial.print(F("Span:\t"));
      MySerial.print((sensorMaxValue - sensorMinValue));
      MySerial.print(F("\tcoilDriven\t"));
      MySerial.print(coilDriven);
      MySerial.print(F("\tperiod:\t"));
      MySerial.println(period);
      MySerial.print("currentPosition"); 
      MySerial.println(currentPosition);
      
      MySerial.print("idx");
      MySerial.print("\ttime");    
      MySerial.print("\tsensorValue:");
      MySerial.print("\tcurrentPosition");
      MySerial.print("\tcurrentDegrees");
      MySerial.println("\tcurveDirection");

      for(int i=logIdx+1; i<100; i++){
         MySerial.print(i);
         MySerial.print("\t");         
         MySerial.print(logTime[i]);           
         MySerial.print("\t");         
         MySerial.print(logCalculated[i][0]);
         MySerial.print("\t");
         MySerial.print(logCalculated[i][1]);
         MySerial.print("\t");         
         MySerial.print(logCalculated[i][2]);
         MySerial.print("\n");         
      }
      for(int i=0; i<logIdx+1; i++){
         MySerial.print(i);
         MySerial.print("\t");         
         MySerial.print(logTime[i]);           
         MySerial.print("\t");         
         MySerial.print(logCalculated[i][0]);
         MySerial.print("\t");
         MySerial.print(logCalculated[i][1]);
         MySerial.print("\t");         
         MySerial.print(logCalculated[i][2]);
         MySerial.print("\n");          
      }
#endif
      delay(10000);
      period = INITIAL_PERIOD;
      timeToChangePeriod =  timeNew + CHANGE_PERIOD_TIME;  
      sensorMinValue = sensorValue;
      sensorMaxValue = sensorValue;      
      coilDriven = false;
    }
#endif

#if defined (__DEBUG__)
  // this section controls de debug  
  //first save the current values for debugging 
  if( logIdx + 1 < DEBUG_SIZE  ) logIdx++; else logIdx=0;
  logTime[logIdx] = timeNew;
  logSensor[logIdx] = sensorValue;
  
  logCalculated[logIdx][0] = currentPosition; 
  logCalculated[logIdx][1] = currentDegrees;  
  logCalculated[logIdx][2] = curveDirection;
  
  //this part starts the debug and print the debug header
  if (timeNew >= debugStartTime && false == debugActivated  && false == coilDriven ){ //&& false == coilDriven && period <= MIN_PERIOD && logCalculated[debugIdxToPrint][0] == 2 && logCalculated[DEBUG_SIZE-1][0] == 3
    debugActivated = true;
//    MySerial.println("***************************************");
//    MySerial.print("timeNew:");
//    MySerial.println(timeNew);

    debugStartTime =  timeNew + DEBUG_TIME_QUIET;

//    MySerial.print("debugStartTime:");
//    MySerial.println(debugStartTime);

//    MySerial.print("logIdx");
//    MySerial.println(logIdx);

    debugIdx = 0; 
    if( logIdx + 1 < DEBUG_SIZE )
      debugIdxToPrint = logIdx + 1;
    else debugIdxToPrint = 0;

    MySerial.print(F("debugIdxToPrint:\t"));
    MySerial.print(debugIdxToPrint);
    MySerial.print(F("\tMinValue:\t"));
    MySerial.print(sensorMinValue);
    MySerial.print(F("\tMaxValue:\t"));
    MySerial.print(sensorMaxValue);
    MySerial.print(F("\tcoilDriven\t"));
    MySerial.print(coilDriven);
    MySerial.print(F("\tperiod:\t"));
    MySerial.println(period);

    MySerial.print(F("logIdx"));
    MySerial.print(F("\ttimeNew"));
    MySerial.print(F("\tsensorValue"));
    MySerial.print(F("\tcurrentPosition"));
    MySerial.print(F("\tcurrentDegrees"));
    MySerial.print(F("\tcurveDirection"));
    MySerial.print(F("\n"));
   
  }
  else if( true==debugActivated){
    if(debugIdx < DEBUG_SIZE ){
      debugIdx++;
      //MySerial.print("debugIdx:"); MySerial.println(debugIdx);
      if( debugIdxToPrint + 1 < DEBUG_SIZE )
        debugIdxToPrint = logIdx + 1;
      else debugIdxToPrint = 0;
      //MySerial.print("\tdebugIdxToPrint:"); MySerial.println(debugIdxToPrint);
    }
    else{
      debugActivated = false;
    }
  }
#endif
  
#if defined (__DEBUG__)
  if( true==debugActivated && coilDriven==0) { //
    //Print the values on debugIdxToPrint
/*
    MySerial.print(debugIdxToPrint);
    MySerial.print("\t");
    MySerial.print(logTime[debugIdxToPrint]);
    MySerial.print("\t");
*/    
    MySerial.print(logSensor[debugIdxToPrint]);
    MySerial.print("\t");
  
    MySerial.print(logCalculated[debugIdxToPrint][0]);
    MySerial.print("\t");         
    MySerial.print(logCalculated[debugIdxToPrint][1]);
    MySerial.print("\t");         
    MySerial.print(logCalculated[debugIdxToPrint][2]);
    MySerial.print("\n");         

  }
  // end debug logic 
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
    if(  currentPosition == 3 && 2 == curveDirection  &&  currentDegrees >= 0 && currentDegrees <= 30 ){ //
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
    else if( currentPosition == 1  && 2 == curveDirection &&  currentDegrees >= 0 && currentDegrees <= 30  ){ //
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
    else if( currentPosition == 2  && 2 == curveDirection &&  currentDegrees >= 0 && currentDegrees <= 30  ){ //
          //go to 3  

#if defined (__DEBUG__)
      if( timeNew > timeToReportSpeed ){
        int currentRPM = (1000 / (timeNew - timeLastTurn)) * 60;
        MySerial.print(F("RPM:"));
        MySerial.println(currentRPM);
        MySerial.print(F("SensorSpan:"));
        MySerial.println(sensorMaxValue - sensorMinValue);
        timeToReportSpeed = timeNew + 10000;
      } 
      timeLastTurn = timeNew; 
#endif
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
        ////MySerial.println(F("1"));
       //digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(C3, LOW);
        digitalWrite(C1, HIGH);
        digitalWrite(C2, HIGH);
        currentPosition = 1;
      }  
      else if(1 == currentPosition ){
        ////MySerial.println(F("2"));
        //digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(C1, LOW);
        digitalWrite(C2, HIGH);  
        digitalWrite(C3, HIGH);        

        currentPosition = 2; 
      }  
      else if(2 == currentPosition ){
        ////MySerial.println(F("3"));
        
        //digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(C2, LOW);
        digitalWrite(C3, HIGH);
        digitalWrite(C1, HIGH);
        
        currentPosition = 3;  
      }
      timeOld = timeNew;
    }
    
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
   
 // //MySerial.println(timeNew);
}


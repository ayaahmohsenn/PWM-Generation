#include <stdio.h>
#include <stdlib.h>

int main()
{
 //Tiva c microcontroller was used since it is more reliable, faster and has more memory than the arduino
//define the ouputpins
int S1 = 6; //tivaC pin
int S3 = 9;
int S5 = 10;

float ma = 0.9; // sine amplitude modulation index

//prelocate the vector for the sine and triangle wave values
float sine[2000];
float triangle[2000];

//intialize sine indexing paramters for the three phases
long int sineIndex = 0;
long int sineIndexb = 0;
long int sineIndexc = 0;

double sineFreq = 10.0;
double period = 1/sineFreq;
double sinePeriodMicrod =  period *1000.0*1000.0 ; // 20000 is the 0.02 sec or 50Hz time 10^6
long int sinePeriodMicro = sinePeriodMicrod;
float sinInc = 2 * 3.1459 / 2000.0; // split the period of 0 to 2*pi to 2000 points

//  value of the sine index in the current iteration
float sineVal = 0;
float sineValb = 0;
float sineValc = 0;

// time since the start of the current sine period
long int sineTimeMicro = 0;
long int sineTimeMicrob = 0;
long int sineTimeMicroc = 0;


//intialize triangle wave paramaters
long int triFreq = 50 * 80;
long int triIndex;
long int triPeriodMicro = 250;
long int triTimeMicro;
float triVal = 0;

// current time measurements in microseconds
long int currTime = 0;
long int currTimeb = 0;
long int currTimec = 0;

//Speed Calculations
int state =0;
int prevState = 0;
long int prevPulseTime;
int prox = 11;
int currState =0;
long int timeStep = 0;
float RPM = 0;
int count= 0;
long int startTime =0;
long int stopTime =0;
int speedtime = 12;
int getSpeed();
int refRPM = 250;
//int omegaRPM = 0;



void setup() {
  // three signals to the gate drivers
  // each one is connected to a NOT gate to produce its complementary signal
  pinMode(S1, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S5, OUTPUT);
  pinMode(prox,INPUT);
  pinMode(speedtime,OUTPUT);

  // create an array to save sine values
  // this is done to reduce computations during runtime
  //2000 elements is a reasonable compromise between resolution and memory
  for (int i = 0; i < 2000; i++) {
    sine[i] = ma * sin((i * sinInc));
  }

  // create an array to save triangular wave values
  // from 0 to 1
  for (int i = 0; i < 500; i++) {
    triangle[i] = i / 500.0  ;
  }
  //from 1 to -1
  for (int i = 500; i < 1500; i++) {
    triangle[i] = (500.0 - i) / 500.0 + 1;
  }

  // from -1 to 0
  for (int i = 1500; i < 2000; i++) {
    triangle[i] = (i - 1500.0) / 500.0 - 1;
  }

  Serial.begin(9600);
}


void loop() {

  currTime = micros(); // get current Time
  currTimeb = currTime + (sinePeriodMicro*0.333333333); // phase shift phase b by 6.6667 ms (120 deg)
  currTimec = currTime +(sinePeriodMicro*0.66666667) ; // phase shift phase b by 13.333 ms (240 deg)

  // determine time since the current period started (Tp)
  // this is used to access the values array
  triTimeMicro = currTime % triPeriodMicro;
  sineTimeMicro = currTime % sinePeriodMicro;
  sineTimeMicrob = currTimeb % sinePeriodMicro;
  sineTimeMicroc = currTimec % sinePeriodMicro;

  //determine index in the values array
  sineIndex = sineTimeMicro * 2000* sineFreq / (1000*1000); // Index = Tp(in seconds)* Number of elements * freq
  sineIndexb = sineTimeMicrob *  2000* sineFreq / (1000*1000);
  sineIndexc = sineTimeMicroc *  2000* sineFreq / (1000*1000);

  //triangle index calculations
  triIndex = triTimeMicro * 8; // same calculation but 80x the frequency

  // determine the value in the current iteration
  sineVal = sine[sineIndex];
  sineValb = sine[sineIndexb];
  sineValc = sine[sineIndexc];

  triVal = triangle[triIndex];


  //currTime = micros();
  currState = digitalRead(prox);  //current state

  if (currState == 1 && prevState ==0) {
    timeStep = currTime - prevPulseTime;
    RPM = 60/(timeStep * 0.001* 0.001);
    prevPulseTime = currTime;
  }

  prevState = currState;



  if (currTime < stopTime && currTime > startTime)
     {
      digitalWrite(speedtime,HIGH);
     }

   else
   {

    startTime = currTime;
    stopTime = currTime + RPM;
    digitalWrite(speedtime,LOW);


  }


  if (RPM < refRPM){

    sineFreq = sineFreq + 0.5;
  }
  else if (RPM > refRPM) {
    sineFreq = sineFreq - 0.5;
  }
  else if (RPM == refRPM){
    sineFreq = sineFreq;
  }

  if (sineFreq > 15){
    sineFreq = 15;
  }

  else if (sineFreq < 3){
    sineFreq = 3;
  }






  //compare the sine value with the triangular value for each phase to perfrom sine pwm
  if (sineVal > triVal) {
    digitalWrite(S1, 1); // S4 goes to LOW since it's connected with S1 NOT

  }
  else {
    digitalWrite(S1, 0); //S4 goes to HIGH since it's connected with S1 NOT
  }

  if (sineValb > triVal) {
    digitalWrite(S3, 1); //S6 goes to LOW since it's connected with S3 NOT

  }
  else {
    digitalWrite(S3, 0); //S6 goes to HIGH since it's connected with S3 NOT
  }

  if (sineValc > triVal) {
    digitalWrite(S5, 1); //S2 goes to LOW since it's connected with S5 NOT
  }
  else {
    digitalWrite(S5, 0); //S2 goes to HIGH since it's connected with S5 NOT
  }


}



 /* int getSpeed() {
   //Speed calculations
  currTime = micros();
  currState = digitalRead(prox);  //current state

  if (currState == 1 && prevState ==0) {
    timeStep = currTime - prevPulseTime;
    RPM = 60/(timeStep * 0.001* 0.001);
    prevPulseTime = currTime;
  }

  prevState = currState;



  if (currTime < stopTime && currTime > startTime)
     {
      digitalWrite(speedtime,HIGH);
     }

   else
   {

    startTime = currTime;
    stopTime = currTime + RPM;
    digitalWrite(speedtime,LOW);


  }

  return RPM;
  }*/
}

/*
  
  (c) Andrew Ke, Jie Ke, and the Grid Team 2014
  All rights reserved.
  Work may not be reproduced in any way without the makers' permission

  Raspberry Post Ny Update #1
  1/14/13 Version 2.0.1

  **IMPORTANT CHANGES**:
    - Pulse color optimized to only accept end color
    - Nice pulse effects for Message Type 8 and Flow
    - Additional Message Type 8 support for multiple directions besides CIRCLE
    -VERY IMPORTANT Changes to "Discard mechanism"
      ~aditional "break comment"
      ~no more annoying print messages
    
  KNOWN BUG ALERT!
    -Flow NO LONGER HAS COLOR DETECITON!?!
  
*/


#include <SoftwareSerial.h>

SoftwareSerial softSerial4(50,51); 


#define RED	0xFF0000
#define GREEN	0x00FF00
#define BLUE	0x0000FF
#define YELLOW	0xFFFF00
#define CYAN    0x00FFFF
#define ORANGE	0xFF4400
#define MAGENTA	0xFF00FF
#define WHITE	0xFFFFFF
#define BLANK   0x000000

#define CIRCLE 0
#define BOTTOM 1
#define LEFT   2
#define TOP    3
#define RIGHT  4

#define SEPARATOR ','
#define MESSAGE_SEPERATOR '\n'

#define SOFT_IDX 2

#define SIMULATION
#ifdef SIMULATION
  #define STRIP_LENGTH 5 
#else
  #define STRIP_LENGTH 48
#endif

//#define TileTester
#ifdef TileTester
  #define TileTester 1
#else
  #define TileTester 0
#endif

#define MAX_CLOCK 8192
#define MAX_VALUES 100
#define NUMBER_OF_PORTS 5

long stripColors[STRIP_LENGTH];
long stripStates[] = {0,0,0,0,0};
long colorsArray[] = {RED, GREEN, BLUE, CYAN, YELLOW, ORANGE, MAGENTA, WHITE, BLANK};
long currentColor;

int CKI = 42;
int SDI = 43;
int colour_id = 0;
int button = 53;

int myCoords[] = {0,0};
boolean originBroadcast = false;
boolean sentCoordinates = false;

String TILE_NAMES[] = {"Self", "Bottom", "Left", "Top", "Right"};
int lenOfGameVars;
int gameVars[MAX_VALUES];

void *ports[] = {&Serial, &Serial3, &softSerial4, &Serial2, &Serial1};

boolean isFlowInitialized = false;
int gameMode = 1;

long prevState = -1;

int numOfGameModes = 7;
String GAME_MODE_NAMES[] = {"NOTHING", "INTERACTIVE", "SHOW", "FLOW", "CLEAR", "SPIN", "TEST COMMUNICATION"};

void resetCommonState() {
  for (int i = 0; i < MAX_VALUES; i++) {
    gameVars[i] = 0;
  }
  lenOfGameVars = 0;
  
}

void setup(){
  
  Serial.println("Welcome to the Grid! Start Up Successful!");

  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  pinMode(button,INPUT);
  pinMode(13,OUTPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  softSerial4.begin(9600);
  
  randomSeed(analogRead(0));

  resetCommonState();
  gameMode = 1;
  
  gameVars[0] = 1;
  gameVars[1] = 2;
  gameVars[2] = 5;

}

int newState = false;
void loop(){
  newState = false;

  processMessages();
      
  
  if (gameMode < 0 || gameMode >= numOfGameModes) {
    // not supported game mode
    return;
  }

  if (newState) {
    Serial.println("In game mode: " + GAME_MODE_NAMES[gameMode]);
  }

  switch(gameMode){     
    case 0: // NOTHING
      doNothing();
      break;
    case 1: // INTERACTIVE
      interactive(numberToColor(gameVars[0]),numberToColor(gameVars[1]), gameVars[2]);
      break;
    case 2: // SHOW
      playShow(numberToColor(gameVars[0]),gameVars[1], gameVars[2]);
      break;
    case 3: // FLOW
      if(newState){
        prevState = -1;
        isFlowInitialized = false;
      }
      flow();
      break;
    case 4: // CLEAR
      clearAll();
      pushPixels();
      break;
    case 5: // SPIN
      spin(gameVars[0]);
      break;
    case 6: // TEST COMMUNICATION
      communicationTest();
      break;
  }
  delay(5);
}

int spinColor = 1;
void spin(int spd){
  spinColor = (spinColor + 1);
  if(spinColor == 9) spinColor = 1;
  rotateLight(spd, numberToColor(spinColor));
}

void tileTester(){
  gameMode = 0;
  /*int values[4] = {8,0,0,9};
  portWrite(4,values,4);
  pushPixels();
  delay(500);*/ 
  Serial.println("TileTester");
  
  for(int i=1; i<NUMBER_OF_PORTS; i++){
      int values[5] = {8,0,0,2,2};
      if(i==4){
        values[4] = LEFT;
      }if(i == 3){
        values[4] = BOTTOM;
      }
      portWrite(i,values,5);
  }

  delay(1000);
}

void rotateLight(int spd, long color){
  for(int i = 0; i < 5; i++){
    turnOn(i, color);
    pushPixels();
    clearAll();
    pushPixels();
    delay(spd);
  }
}

void doNothing(){
  if(newState){
    clearAll();
    pushPixels();
  }

}

//The Show
long showColor;
boolean showInitialized = false;
void playShow(long mycolor, int pulseSpeed, int wait){
  if(!showInitialized){
    delay(random(0,1000));
    showInitialized = true;
  }
  showColor = mycolor;
  if(stripStates[CIRCLE] == showColor){
          pulseColor(BLANK, pulseSpeed,-1);
  }else if(stripStates[CIRCLE] != BLANK){
      pulseColor( showColor, pulseSpeed,-1);
  }
  else{
      pulseColor(showColor,pulseSpeed,-1);
  }
  pushPixels();
  delay(wait);
  delay(random(0,400));
  
  
}

int iState;
int iLastState;

void interactive(long offColor, long onColor, int spd){
    iLastState = iState;
    if(spd == 0) spd = 5;
    if(digitalRead(button) == true){
      iState = 1;
      if(iLastState != iState){
        pulseColor(onColor, spd,-1);  
      }
    }
    else{
      iState = 0;
      if(iLastState != iState){
        pulseColor(offColor, spd,-1);  
      }
    }
    if(newState || (currentColor != offColor && currentColor != onColor)){
      if(digitalRead(button)){
          pulseColor(onColor, spd,-1);
        }else{
          pulseColor(offColor, spd,-1);
     }
   }
}

int states[5];
long colors[5];
long color = RED;
int state;
boolean changed = true;
boolean isStartEnd = false;
boolean selfChanged = true;

void flow(){
  if(!isFlowInitialized){
    Serial.println("Initializing");
    initFlow();
  }
  flowDebugLight();
  
  if(digitalRead(button)){
    if(state == 1){ // We gain a person
      state = 4;
      selfChanged = true;
    } 
    if(state == 2){
      state = 3;
      selfChanged = true;
    }
  }else{
    if(state == 3){ // We lose a person
      state = 2;
      selfChanged = true;
    }
    if(state == 4){
      state = 1;
      selfChanged = true;
    }
  }
  
  if(state == 2 || state == 4){
  /*  if(state == 4 && digitalRead(button) == false){
      Serial.println("Accident Forgiveness 4=>1!");
      selfChanged = true;
      state = 1;
      return; 
    }
    if(state == 2 && digitalRead(button)){ 
      Serial.println("Accident Forgiveness 2=>3!");
      selfChanged = true;
      state = 3;
      return;
    }*/
    for(int i =1; i<5; i++){
      if(( state == 4 && states[i] == 2) || (state == 2 && states[i] == 4)){
        broadcastStateColor();
        delay(200);
        
        selfChanged = true;
        Serial.println("Connecting to: " + TILE_NAMES[i]);
        if(state == 4){
          color = colors[i];
          state = 3;
        }else{
          state = 1;  
        }
        if(stripStates[i] == BLANK){
          Serial.println("gain");
          pulseColor(color,5,i);
        }else{
          pulseColor(BLANK, 5, i); 
        }
        
      } 
    }  
  }

  if(changed || selfChanged){ //Prints out states of neighboring tiles
    pushPixels();
    states[0] = state;
    for(int i = 0; i<5; i++){
      Serial.print(TILE_NAMES[i]); Serial.print(String(states[i]) + " ");
    }
    Serial.println();
    changed = false;
  }
  
  if(selfChanged){
    broadcastStateColor();
    selfChanged = false; 
  } 
  
  //delay(20);
}

void prepFlow(){
   
}

void flowDebugLight(){
  if(state%2 == 0){
    digitalWrite(13,HIGH); 
  }else{
    digitalWrite(13, LOW);  
  }
}

void initFlow(){
  for(int i=0; i<5; i++){
    states[i] = 0;
    colors[i] = 0;
  }
  if(digitalRead(button)){
    state = 3;
    //turnOn(CIRCLE, color);
  }else{
    state = 1;
    /*if (!isStartEnd) {
      turnOn(CIRCLE, BLANK);
    }*/
  }
  selfChanged = true;
  changed = true;
  isFlowInitialized = true;   
}

//update state of myself based on the state of button for simulator,
//or switch for real tile.
void communicationTest() {
  int prevState = state;
  
  if (digitalRead(button)){
    state = 3;
  } else {
    state = 1;
  }
  if (state == 3) {
    turnOn(CIRCLE, color);
  } else {
    turnOn(CIRCLE, BLANK);
  }
  if(prevState!= state)     pushPixels();
  if (prevState == 1 && state == 3) {
    broadcastStateColor();
  }
}

void broadcastStateColor(){
    int values[3] = {10, state, colorToNumber(color)};
    Serial.print("Sending state: "); Serial.print(values[1]);
    Serial.print(" color: "); Serial.println(values[2]);
    for (int i=1; i<NUMBER_OF_PORTS; i++) {
      portWrite(i, values, 3);
    }
}

//END
void pulseColor (long color2, int spd, int location){
  long color1;
  if(location == -1){
    color1 = stripStates[0];
  }else{
    color1 = stripStates[location];
  }
  long strt[] = {(color1>>16) & 255, (color1>>8) & 255, color1 & 255};
  long stp[] = {(color2>>16) & 255, (color2>>8) & 255, color2 & 255};
  long dif[] = {strt[0]-stp[0], strt[1]-stp[1], strt[2]-stp[2]};

  
  int ldif = max( max(abs(dif[0]), abs(dif[1])), max(abs(dif[1]), abs(dif[2])));
  for(int i = 0; i<ldif/spd; i++){
      for(int i = 0; i<3; i++){
        if(strt[i]!= stp[i]){
          strt[i] = strt[i] - (((strt[i]-stp[i])/abs((strt[i]-stp[i])))*spd);
        }
      }  
      
    //Serial.println(String(strt[0])+ " " + String(strt[1]) + " " + String(strt[2]));
    long mycolor = strt[0]<<16 | strt[1]<<8 | strt[2];
    //Serial.println(mycolor);
    if(location == -1){
      for(int i =0; i<5; i++){
        turnOn(i, mycolor);  
      } 
    }else{
      turnOn(location, mycolor);
    }
    pushPixels();
    delayMicroseconds(5000);
  }
  if(location == -1){
    turnOnAll(color2);
  }else{
    turnOn(location, color2);
  }
  pushPixels();

}





void processMessages(){
  int values[MAX_VALUES]; // holds message from neighboring tile in format: [type, int, int, ...]

  for(int i =0; i<MAX_VALUES; i++){
    values[i] = 0; //Clears MAX VALUES
  }
  int num; //numbers of integers stored in the message including type
  //Serial.println("LOOKING");
  for (int i = 0; i < NUMBER_OF_PORTS; i++) {
    num = portRead(i, values, MAX_VALUES);
    if (num == 0) {
      continue;
    }
    printMessage(i, values, num);


    if (num == 3 && values[0] == 10) {
      if(states[i] != values[1]){
        changed = true;
        states[i] = values[1];
      }
      colors[i] = numberToColor(values[2]);
      if (gameMode == 6) { // TEST COMMUNICATION MODE
        flashLight(i, CYAN);
      }
    }
    if(values[0] == 5 && num >= 2){
      newState = true;
      gameMode = values[1];
      for(int i = 2; i< num; i++){
        gameVars[i-2] = values[i];
        Serial.println(gameVars[i-2]); 
      }
      broadcastForward(values, num);
    }
    if(values[0] == 6){
        Serial.println("Im the Origin");
        int topSend[] = {7, myCoords[0], myCoords[1] + 1};
        //Serial.println(String(topSend[1]) + String(topSend[2]));
        portWrite(3, topSend, 3);
        int rightSend[] = {7,myCoords[0]+1, myCoords[1]};
        portWrite(4, rightSend, 3);
    }
    if(values[0] == 7){
       Serial.println("HELLO: " + String(values[1]) +" " +  String(values[2]));
       myCoords[0] = values[1];
       myCoords[1] = values[2];
       int topSend[] = {7, myCoords[0], myCoords[1] + 1};
       portWrite(3, topSend, 3);
       int rightSend[] = {7,myCoords[0]+1, myCoords[1]};
       portWrite(4, rightSend, 3);
       delay(20);
    }
    if(values[0] == 8){
       Serial.println("My Coords:" + String(myCoords[0]) + String(myCoords[1]));
       Serial.println("Recieved message for " + String(values[1]) + "," + String(values[2]) );
       if(values[1] == myCoords[0] && values[2] == myCoords[1]){
         Serial.println("Thats Me! Direction is " + String(values[4]));

         if(values[4]==0){
            Serial.println("Option 1: Light Center");
            turnOn(CIRCLE, numberToColor(values[3]));
            //pulseColor(numberToColor(values[3]),5,CIRCLE);
         }else if(values[4]<=4){
            Serial.println("Option2");
            turnOn(values[4],numberToColor(values[3]));
            //pulseColor(numberToColor(values[3]),5,values[4]);
         }
         pushPixels();
         //color = numberToColor(values[3]);
       }
       portWrite(3,values, num);
       portWrite(4,values, num);
    }
  }
}

//****************
// Serial Communication Code
//****************

void broadcastForward(int* values, int len) {
  for(int i = 3; i < 5; i++){
    portWrite(i, values, len);
  }
}

void portWrite(int portIdx, char ch){
  if (portIdx == SOFT_IDX) {
    ((SoftwareSerial *)ports[portIdx])->print(ch);
  } else {
    ((HardwareSerial *)ports[portIdx])->print(ch);
  }
}

void portWrite(int portIdx, int value){
  if (portIdx == SOFT_IDX) {
    ((SoftwareSerial *)ports[portIdx])->print(value);
  } else {
    ((HardwareSerial *)ports[portIdx])->print(value);
  }
}

void portWrite(int portIdx, int* values, int len) {
  for (int i=0; i<len; i++) {
    portWrite(portIdx, values[i]);
    if (i == len - 1) {
      portWrite(portIdx, MESSAGE_SEPERATOR);
    } else {
      portWrite(portIdx, SEPARATOR);
    }
  }
}

char portPeek(int portIdx) {
  char ch;
  if (portIdx == SOFT_IDX) {
    ch = ((SoftwareSerial *)ports[portIdx])->peek();
  } else {
    ch = ((HardwareSerial *)ports[portIdx])->peek();
  }
  return ch;
}

char portForcePeek(int portIdx) {
  char ch = portPeek(portIdx);
  while (ch == -1) {
    //Serial.println("in force peek");
    ch = portPeek(portIdx);
  }
  return ch;
}

char portRead(int portIdx) {
  char ch;
  if (portIdx == SOFT_IDX) {
    ch = ((SoftwareSerial *)ports[portIdx])->read();
  } else {
    ch = ((HardwareSerial *)ports[portIdx])->read();
  }
  return ch;
}

char portForceRead(int portIdx) {
  char ch = portRead(portIdx);
  while (ch == -1) {
    ch = portRead(portIdx);
    //Serial.println("in force read");
  }
  return ch;
}


// it's expected that the Serial port has at least one char available to read.
// integer must be followed by non-digit char. 
int portReadInt(int portIdx) {
  String s;
  char ch = portForcePeek(portIdx);
  while(isDigit(ch)) {
    s = s + portForceRead(portIdx);
    ch = portForcePeek(portIdx);
  }
  if (s.length() == 0) {
    return -1;
  } else {
    return s.toInt();
  }
}

// reads a list of int values from port, returns # of int values read.
// input is expected to be int values separated by single non-digit char,
// ended with MESSAGE_SEPARATOR
int portRead(int portIdx, int* values, int maxLen) {
  int idx = 0;
  char ch = portPeek(portIdx);
  while (ch != -1 && !isDigit(ch)) {
    //discard any none digit chars
    ch = portRead(portIdx);
    //Serial.print("Discarded:"); Serial.print(ch); Serial.print("| from "); Serial.println(TILE_NAMES[portIdx]);
    ch = portPeek(portIdx);
    break;
  }
  if (ch != -1) {
    while (idx < maxLen && (isDigit(ch) || ch == SEPARATOR)) {
      values[idx] = portReadInt(portIdx);
      //Serial.print("Value read:"); Serial.print(values[idx]);
      ch = portForceRead(portIdx);
      //Serial.print(" separator:"); Serial.print(ch); 
      idx++;
    }
  }
  //Serial.println();
  return idx;
}

void printMessage(int portIdx, int* values, int len) {
  if (len > 0) {
    Serial.println();
    Serial.print("MESSAGE TYPE:  "); Serial.print(values[0]);
    Serial.print(" from "); Serial.println(TILE_NAMES[portIdx]);
    Serial.print("****Values: ");
    for (int i = 0; i < len; i++) {
      Serial.print(values[i]);
      if (i < len -1) {
        Serial.print(",");
      }
    }
    Serial.println("****\n");
  }
}

//****************
// LED CODE
//****************

void turnOnAll(long color){
  currentColor = color;
  for(int i =0; i<5; i++){
    turnOn(i,color);  
  }
}
void turnOn (byte Direction, long color){
  currentColor = color;
  stripStates[Direction] = color;
  if(STRIP_LENGTH != 5){
    turnOnTile(Direction, color);
    return;  
  }
  switch (Direction){
    case CIRCLE:
      stripColors[0] = color;
      break;
    case BOTTOM:
      stripColors[1] = color;
      break;
    case LEFT:
      stripColors[2] = color;
      break;
    case TOP:
      stripColors[3] = color;
      break;
    case RIGHT:
      stripColors[4] = color;
      break;
  }
}

void turnOnTile (byte Direction, long color){

  switch (Direction){
    case CIRCLE:
      turnOn2(0,20, color);
      stripColors[37] = color;
      stripColors[23] = color;
      stripColors[44] = color;
      stripColors[30] = color;
      break;
    case BOTTOM:
      turnOn2(20,27, color);
      break;
    case LEFT:
      turnOn2(41,48, color);
      break;
    case TOP:
      turnOn2(27, 34, color);
      break;
    case RIGHT:
      turnOn2(34,41, color);
      break;
  }
}


void turnOn2(int start, int stop, long color){
  for(int i = start; i<stop; i++){
    stripColors[i] = color;
  }  
}

void pushPixels (){
  for(int LED_number = 0; LED_number < STRIP_LENGTH; LED_number++)
    pushPixel(stripColors[LED_number]);
  digitalWrite(CKI, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}

void clearAll () {
  for (int i = 0; i < NUMBER_OF_PORTS; i++) {
    turnOn(i, BLANK);
  }
}

void pushPixel ( long Color ) {
  for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
    digitalWrite(CKI, LOW); //Only change data when clock is low
    long mask = 1L << color_bit;
    if(Color & mask) 
      digitalWrite(SDI, HIGH);
    else
      digitalWrite(SDI, LOW);
    digitalWrite(CKI, HIGH); //Data is latched when clock goes high
  }
}

void flashLight(int idx, long color) {
  for(int i = 0; i < 4; i++) {
    clearAll();
    turnOn(idx, i % 2 == 0 ? color : BLANK);
    pushPixels();
    delay(400);
  }
}

long numberToColor(byte number){
  return colorsArray[number-1];
  
}

int colorToNumber(long color){
  for(int i = 0; i<9; i++){
    if(colorsArray[i] == color){
      return i+1;
    }
  }
}






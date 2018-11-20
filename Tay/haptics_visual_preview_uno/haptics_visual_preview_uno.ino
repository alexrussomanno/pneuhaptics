
// For Arduino UNO
int SER_Pin = 3;  // data
int SRCLK_Pin = 4; // clock
int RCLK_Pin = 5;  // latch

/*        TPIC6C595      
 *           ___
 *      5V -| U |- GND
 *  SER_IN -|   |- GND
 *       0 -|   |- 7
 *       1 -|   |- 6
 *       2 -|   |- 5
 *       3 -|   |- 4
 *      5V -|   |- RCLK
 *     GND -|   |- SER_OUT (to next SER_IN)
 *           ```
 */


// Define number of registers and an array to hold boolean values for each register value
#define num_registers 32
boolean register_value[num_registers];

// --------CONSTANTS (won't change)---------------
const int valve[] = {2,3,4,5,8,9,10,1138,40,42,44,39,41,43,45};  // assign valves to pin
const int led[] = {22,24,26,28,30,32,34,36,23,25,27,29,31,33,35,37};  // assign led to pin
const int FSR_PIN[] = {A0,A1,A2,A3};

const unsigned long rampInterval = 300  ; // number of millisecs in each ramp steps
const int activateButton = 100;  // this is the minimum register value when a button is pressed. This value is based on analogRead of the FSR_PIN

// 1 = visual cues, 2 = haptics cues, 3 = both haptics and visual 
const int gameMode = 1;

//------------ VARIABLES (will change)---------------------

byte cueState[] = {LOW,LOW,LOW,LOW};           //   LOW = off

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previousMillis[] = {0,0,0,0};


// Set minimum and maximum wait time between press-button-cues
unsigned long minWaitTime = 1000 + rampInterval*5;
unsigned long maxWaitTime = 8000;
unsigned long waitTime[] = {0,0,0,0};  // Time LED is OFF (wait time)

int tooEarly = 0;

//========================================

void setup(){
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  
  Serial.begin(9600);
  
  set_all_registers(LOW);
  update_register_values();
}               

void loop(){

// Example code that alternates between setting select_register HIGH and LOW 
// every time "ENTER" is pressed in the Serial port

  int select_register=31;
  while (Serial.read() != 10);
//  if(register_value[select_register]==HIGH) register_value[select_register]=LOW;
//  else register_value[select_register]=HIGH;    
//  Serial.println(register_value[select_register]);
    
  update_register_values();
}

// Updates the state of the power shift registers to correspond to the boolean array register_value
void  update_register_values(){
  digitalWrite(RCLK_Pin, LOW);

  for(int i = num_registers - 1; i >=  0; i--){

    int val = register_value[i];
    digitalWrite(SER_Pin, val);
 
    digitalWrite(SRCLK_Pin, HIGH);
    digitalWrite(SRCLK_Pin, LOW);
  }
  digitalWrite(RCLK_Pin, HIGH);

}

// Sets all registers HIGH or LOW
void set_all_registers(boolean state){
  for(int i = 0; i < num_registers; i++){
      register_value[i] = state;
  }
} 

void updateCueState(int x) {

  if (cueState[x] == LOW) {
    // Count down before press-button-cue-trigger 
    // Serial.println(pos_A_waitTime - currentMillis + previousPos_A_Millis+2);
   
    // Start Ramping when waitTime expires
    tooEarly = ramping(x,previousMillis[x],waitTime[x]);    
    if(tooEarly == 1){ // This means that the user pressed too early    
      // Generate new wait time and restart
      waitTime[x] = random(minWaitTime,maxWaitTime);
      previousMillis[x] = currentMillis;
    } 
  }
  else {  // if cueState is HIGH  
    // If the button is pressed, turn off LED 
    if(analogRead(FSR_PIN[x]) > activateButton){
      // Print button press time
      Serial.print("response");Serial.print(x);Serial.print(":"); Serial.print(currentMillis); Serial.println(";");
      // Turn off LED
      cueState[x] = LOW;      
      // Check which gameMode we are in and shut off valve/led
      activateLedAndOrValve(x,cueState[x],gameMode);    
      // Update and save the time when the button is pressed
      previousMillis[x] = currentMillis;
      // Generate new wait time
      waitTime[x] = random(minWaitTime,maxWaitTime);
    }
  }
}

//========================================

int ramping(int x, unsigned long previousMillis, unsigned long waitTime, boolean cueState) {
  // This function ramps the LED and valves 

  if(analogRead(FSR_PIN[x]) > activateButton) {
    activateLedAndOrValve(x, LOW, gameMode);
    return 1;
  }
  else{
    if (currentMillis - previousMillis >= waitTime + 3 * rampInterval){
      // light the 4th LED
      activateOneDot(3,x,HIGH,gameMode);   
      // Time is up, change cueState to HIGH
      cueState[x] = HIGH;
      // Check which gameMode we are in and trigger cue
      activateLedAndOrValve(x,cueState[x],gameMode);
      
      // Update and save the time when cue is triggered
      //previousMillis[x] = currentMillis;   
      
      // Print cue trigger time
      Serial.print("cue");Serial.print(x);Serial.print(":"); Serial.print(currentMillis ); Serial.println(";");    
      return 0;  
    }
    else if (currentMillis - previousMillis >= waitTime + 2 * rampInterval){
      // turn on 3rd dot of valve/led
      activateOneDot(2,x,HIGH,gameMode);    
      return 0;
    }
    else if (currentMillis - previousMillis >= waitTime + rampInterval){
      // turn on 2nd dot of valve/led
      activateOneDot(1,x,HIGH,gameMode);
      return 0;
    }  
    else if (currentMillis - previousMillis >= waitTime){
      // turn on 1st dot of valve/led
      activateOneDot(0,x,HIGH,gameMode);    
      return 0;
    }    
  } 
}

//======================================

// This function controls the state of 4 LEDs
void ledControl(int xy, byte state){
  for(int i=xy; i < xy+4; i++){
    register_value[i]=state;
//    digitalWrite(led[i],state);
  }  
}

//======================================

// This function controls the state of 4 valves
void valveControl(int xy, byte state){
  for(int i=xy; i < xy+4; i++){
    register_value[i+16]=state;
  }  
}

//======================================

// This function controls the state of 4 LEDs and Valves
void ledValveControl(int xy, byte state){
  for(int i=xy; i < xy+4; i++){
    register_value[i]=state;
    register_value[i+16]=state;
  }  
}

//======================================

void activateLedAndOrValve(int x, byte state, int gameMode){
  switch (gameMode){
    case 1:    // visual cue only
      ledControl(4*x,state);
      break;
    case 2:    // haptics cue only
      valveControl(4*x,state);
      break;
    case 3:    // both haptics and visual cue
      ledValveControl(4*x,state);
      break;
    default:
      Serial.println("gameMode out of bounds");   
  }
} 

//======================================

void activateOneDot(int dot, int x, byte state, int gameMode){
  switch (gameMode) {
    case 1:    // visual cue only
      register_value[4*x+dot]=state;
//      digitalWrite(led[4*x+dot],state);
      break;
    case 2:    // haptics cue only
      register_value[4*x+dot+16]=state;
//      digitalWrite(valve[4*x+dot],state);
      break;
    case 3:    // both haptics and visual cue
      register_value[4*x+dot]=state;
      register_value[4*x+dot+16]=state;
      break;
    default:
      Serial.println("gameMode out of bounds");
  }    
} 



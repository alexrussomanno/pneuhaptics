
                                                                                
// For Arduino UNO
int SER_Pin = 3;  // data
int SRCLK_Pin = 4; // clock
int RCLK_Pin = 5;  // latch
int piezoPin = 9; // position of speaker

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
const int FSR_PIN[] = {A2,A3,A4,A5};

unsigned long rampInterval = 500  ; // number of millisecs in each ramp steps (500 is the standard)
const int activateButton = 500;  // this is the minimum register value when a button is pressed. This value is based on analogRead of the FSR_PIN
const int deactivateButton = 80;
unsigned long flutter = 50;   // the rate at which the fourth dot of led or valve turns on and off

//------------ VARIABLES (will change)---------------------

// 1 = visual cues, 2 = haptics cues, 3 = both haptics and visual 
int gameMode = 1;
// Set minimum and maximum wait time between press-button-cues
unsigned long minWaitTime = rampInterval;
unsigned long maxWaitTime = 5000;
unsigned long waitTime[] = {0,0,0,0};  // Initialize waitTime
int cycle = 40;    // number of cycle before game ends
boolean clickFeel = HIGH;
int onBeat = 0;    // This variable subtracts the remainder from waitTime so that waitTime is a multiple of rampInterval

int numPress = 0;    // sum of the number of times button 0,1,2,3 has been pressed
boolean cueState[] = {LOW,LOW,LOW,LOW};
boolean dotState[] = {LOW,LOW,LOW,LOW};  
int toggle[] = {0,0,0,0};
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previousMillis[] = {0,0,0,0};
unsigned long previousFlutterMillis[] = {0,0,0,0};

//========================================

void setup(){ 
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  for(int i=0; i < 4; i++){
    pinMode(FSR_PIN[i], INPUT);
  }
  randomSeed(analogRead(0));
  Serial.begin(9600);
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.    
  Serial.print("gameMode\t"); Serial.println(gameMode);
  Serial.print("cycle\t"); Serial.println(cycle);
  Serial.print("rampInterval\t"); Serial.println(rampInterval);
  Serial.print("minWaitTime\t"); Serial.println(minWaitTime);
  Serial.print("maxWaitTime\t"); Serial.println(maxWaitTime);
  // Generate random waitTime 
  unsigned long waitTime[] = {random(10)*rampInterval,random(10)*rampInterval,random(10)*rampInterval,random(10)*rampInterval};
  set_all_registers(LOW);
  update_register_values();
  
  // Start loop once enter is pressed
  //while (Serial.read() != 10);
}               

void loop(){
  // Notice that none of the action happens in loop() apart from reading millis()
  // updateCueState checks if a user presses a button on time and updates a new waitTime
  if(numPress <= cycle){
    // Keep running until total button press is equal to the variable 'cycle'
    currentMillis = millis();   // capture the latest value of millis(). This is equivalent to noting the time from a clock
    for (int x=2; x < 4; x++) {
      updateCueState(x);
        if(gameMode == 1 && clickFeel == HIGH){
          if(analogRead(FSR_PIN[x]) > activateButton){
            activateLedAndOrValve(x,LOW,2);
          }
          else{
            activateLedAndOrValve(x,HIGH,2);
          }
        }
    }    
  }
  else{
    // Shut all lights and valves
    activateLedAndOrValve(0,LOW,3);
    activateLedAndOrValve(1,LOW,3);
    activateLedAndOrValve(2,LOW,3);
    activateLedAndOrValve(3,LOW,3);
  }
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

//========================================================================

void updateCueState(int x) {

  if (cueState[x] == LOW) {    
    // Start ramping when waitTime expires
    ramping(x,previousMillis[x],waitTime[x]);   
  }
  else {  // if cueState is HIGH  
    // Start flutter
    if (currentMillis-previousFlutterMillis[x] >= flutter){
      previousFlutterMillis[x]=currentMillis;
      if(dotState[x]==LOW){
        dotState[x]=HIGH;
      }
      else{
        dotState[x]=LOW;
      }
      // start flutter
      activateOneDot(3,x,dotState[x],gameMode);
    }
    // Wait for button to be pressed
    if(buttonStatus(x)==1){
      // Turn off LED or Valves
      cueState[x] = LOW;      
    }
  }
}

//========================================================================

int buttonStatus(int x){
  if(analogRead(FSR_PIN[x]) > activateButton && toggle[x] == 0) {
    Serial.print(x); Serial.print("\t"); Serial.print(previousMillis[x] + waitTime[x] + 3*rampInterval); Serial.print("\t"); Serial.println(currentMillis);
    activateLedAndOrValve(x, LOW, gameMode);
    toggle[x] = 1;
    waitTime[x] = random(minWaitTime,maxWaitTime);
    onBeat = waitTime[x] % rampInterval;
    waitTime[x] = waitTime[x]- onBeat;
    previousMillis[x] = currentMillis;   
    numPress++;
    return 1;
  }
  else if (analogRead(FSR_PIN[x]) < deactivateButton && toggle[x] == 1){
    toggle[x] = 0;
    return 0;
  }
}

//========================================================================
                                                                          
void ramping(int x, unsigned long previousMillis, unsigned long waitTime) {
  
  // This function ramps the LED and valves 
  // Check and update button status. If a button is pressed, it returns a 1
  if(buttonStatus(x)==1){
    if(currentMillis - previousMillis < waitTime + 3 * rampInterval && currentMillis - previousMillis >= waitTime){
      // Button is pressed before cue
      // Beep at the user for 250 millis
      tone(piezoPin,1000,250);      
    }
  }
  else{
    // Start Ramp
    if (currentMillis - previousMillis >= waitTime && currentMillis - previousMillis < waitTime + rampInterval){   
      // turn on 1st dot of valve/led
      activateOneDot(0,x,HIGH,gameMode); 
    }
    else if (currentMillis - previousMillis >= waitTime + rampInterval && currentMillis - previousMillis < waitTime + 2 * rampInterval){
      // turn on 2nd dot of valve/led
      activateOneDot(1,x,HIGH,gameMode);
    }  
    else if (currentMillis - previousMillis >= waitTime + 2 * rampInterval && currentMillis - previousMillis < waitTime + 3 * rampInterval){
      // turn on 3rd dot of valve/led
      activateOneDot(2,x,HIGH,gameMode);    
    }    
    else if (currentMillis - previousMillis >= waitTime + 3 * rampInterval){
      // light the 4th LED
      //activateOneDot(3,x,HIGH,gameMode);
      activateLedAndOrValve(x,HIGH,gameMode);  
      // Time is up, change cueState to HIGH
      cueState[x] = HIGH;
      // Update and save the time when cue is triggered
      // previousMillis[x] = currentMillis;   
    }       
  }
}

//========================================================================

// This function controls the state of 4 LEDs
void ledControl(int xy, boolean state){
  for(int i=xy; i < xy+4; i++){
    register_value[i]=state;
    update_register_values();
  }  
}

//========================================================================

// This function controls the state of 4 valves
void valveControl(int xy, boolean state){
  for(int i=xy; i < xy+4; i++){
    register_value[i+16]=state;
    update_register_values();
  }  
}

//========================================================================

// This function controls the state of 4 LEDs and Valves
void ledValveControl(int xy, boolean state){
  for(int i=xy; i < xy+4; i++){
    register_value[i]=state;
    register_value[i+16]=state;
    update_register_values();
  }  
}

//========================================================================

void activateLedAndOrValve(int x, boolean state, int gameMode){
  switch (gameMode){
    case 1:    // visual cue only
      ledControl(4*x,state);
      update_register_values();
      break;
    case 2:    // haptics cue only
      valveControl(4*x,state);
      update_register_values();
      break;
    case 3:    // both haptics and visual cue
      ledValveControl(4*x,state);
      update_register_values();
      break;
    default:
      Serial.println("gameMode out of bounds");   
  }
} 

//========================================================================

void activateOneDot(int dot, int x, boolean state, int gameMode){
  switch (gameMode) {
    case 1:    // visual cue only
      register_value[4*x+dot]=state;
      update_register_values();
//      digitalWrite(led[4*x+dot],state);
      break;
    case 2:    // haptics cue only
      register_value[4*x+dot+16]=state;
      update_register_values();
//      digitalWrite(valve[4*x+dot],state);
      break;
    case 3:    // both haptics and visual cue
      register_value[4*x+dot]=state;
      register_value[4*x+dot+16  ]=state;
      update_register_values();
      break;
    default:
      Serial.println("gameMode out of bounds");
  }    
} 

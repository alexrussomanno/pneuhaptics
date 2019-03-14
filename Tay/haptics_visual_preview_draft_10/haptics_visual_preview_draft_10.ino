#include "pitches.h"

int SER_Pin = 3;  // data
int SRCLK_Pin = 4; // clock
int RCLK_Pin = 5;  // latch
int piezoPin = 9; // position of speaker
int brightness_Pin = 10;
const int FSR_PIN[] = {A2,A3,A4,A5};

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
 
/*        TPIC6B595      
 *           ___
 *      NC -| U |- NC
 *      5V -|   |- GND
 *  SER_IN -|   |- SER_OUT (to next SER_IN)
 *       0 -|   |- 7
 *       1 -|   |- 6
 *       2 -|   |- 5
 *       3 -|   |- 4
 *      5V -|   |- SRCLK
 *       G -|   |- RCLK
 *     GND -|   |- GND
 *           
 */


// Define number of registers and an array to hold boolean values for each register value
#define num_registers 32
boolean register_value[num_registers];

// --------GAME SETTINGS---------------
// 1 = visual cues, 2 = haptics cues, 3 = audio
// reward=1 no clickfeel/visual, = 2 click feel/visuals

int gameMode = 3;
int reward = 3;

#define activate_threshold 300
#define deactivate_threshold 100

int brightness = 150;

// --------CONSTANTS (won't change)---------------

unsigned long rampInterval = 500; // number of millisecs in each ramp steps (500 is the standard)
int lowerBound = 0;   // Lower range of button that's activated during the game
int upperBound = 4;   // The upper range of activated buttons
const int activateButton[] = {activate_threshold,activate_threshold,activate_threshold,activate_threshold};  // this is the minimum register value when a button is pressed. This value is based on analogRead of the FSR_PIN
const int deactivateButton[] = {deactivate_threshold,deactivate_threshold,deactivate_threshold,deactivate_threshold};
unsigned long flutter = 100;   // the rate at which the fourth dot of led or valve turns on and off
unsigned long cushionTime = 50; // the time in millis a user is allowed to be early before it beeps 
float blinkOn = .8;
boolean beeper = LOW;


//------------ VARIABLES (will change)---------------------

// Set minimum and maximum wait time between press-button-cues
unsigned long minWaitTime = 500;
unsigned long maxWaitTime = 3000;
unsigned long waitTime[] = {3000,3000,3000,3000};  // Initialize waitTime
unsigned long gameLength = 60000;    // number of cycle before game ends

int onBeat = 0;    // This variable subtracts the remainder from waitTime so that waitTime is a multiple of rampInterval. Initialize at 0
int no_response[]={0,0,0,0};
int numPress = 0;    // sum of the number of times button 0,1,2,3 has been pressed
boolean cueState[] = {LOW,LOW,LOW,LOW};
boolean dotState[] = {LOW,LOW,LOW,LOW};  
int toggle[] = {0,0,0,0};
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previousMillis[] = {0,0,0,0};
unsigned long previousFlutterMillis[] = {0,0,0,0};
unsigned long cueTime[] = {3000,3000,3000,3000};  // Initialize cueTime

//========================================

void setup(){ 
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  for(int i=0; i < 4; i++){
    pinMode(FSR_PIN[i], INPUT);
  }
  pinMode(brightness_Pin,OUTPUT);
  
  randomSeed(analogRead(0));
  Serial.begin(9600);
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.    
  Serial.print("gameMode\t"); Serial.println(gameMode);
//  Serial.print("gameLength\t"); Serial.println(gameLength);
  Serial.print("rampInterval\t"); Serial.println(rampInterval);
  Serial.print("minWaitTime\t"); Serial.println(minWaitTime);
  Serial.print("maxWaitTime\t"); Serial.println(maxWaitTime);
  Serial.print("cushionTime\t"); Serial.println(cushionTime); 
  Serial.print("reward\t"); Serial.println(reward);  

  set_all_registers(LOW);

  activateLedAndOrValve(0, HIGH, 2);
  activateLedAndOrValve(1, HIGH, 2);
  activateLedAndOrValve(2, HIGH, 2);
  activateLedAndOrValve(3, HIGH, 2);
  analogWrite(brightness_Pin,brightness);
  
  // Start loop once enter is pressed
  //while (Serial.read() != 10);
}               

void loop(){
  // Notice that none of the action happens in loop() apart from reading millis()
  // updateCueState checks if a user presses a button on time and updates a new waitTime
  if(numPress < 21){
    // Keep running until total button press is equal to the variable 'cycle'
    currentMillis = millis();   // capture the latest value of millis(). This is equivalent to noting the time from a clock
    updateCueState(2);
//    for (int x=lowerBound; x < upperBound; x++) {     
//      updateCueState(x);                    
//    }    
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
  update_register_values();
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
      if(gameMode == 2 || gameMode == 3){
        //activateOneDot(3,x,dotState[x],2);
      }
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
  if (currentMillis > previousMillis[x] + waitTime[x] + (6+blinkOn)*rampInterval) no_response[x]=1;
  if(analogRead(FSR_PIN[x]) > activateButton[x] && toggle[x] == 0 || no_response[x]) {   
    Serial.print(x); Serial.print("\t"); Serial.print(previousMillis[x] + waitTime[x] + (4+blinkOn)*rampInterval); Serial.print("\t"); 
    
    if(no_response[x]) {
      Serial.println(0);
      no_response[x]=0;
    }
    else Serial.println(currentMillis);

    if(reward == 2){
      if(gameMode == 1){
        //activate LED
        activateLedAndOrValve(x, HIGH, 1);
        activateLedAndOrValve(x, LOW, 2);
      }
      else if (gameMode == 2){
        activateLedAndOrValve(x, LOW, 2);
      }     
      else if (gameMode == 3){
        sound(NOTE_D5);
        activateLedAndOrValve(x, LOW, 2);
      }  
    }
    else if(reward == 3){
      if(currentMillis < previousMillis[x]+waitTime[x]+ (4+blinkOn)*rampInterval+cushionTime && currentMillis > previousMillis[x]+waitTime[x]+ (4+blinkOn)*rampInterval - cushionTime){
        if(gameMode == 1){
          //activate click feel and flash LED
          activateLedAndOrValve(x, HIGH, 1);
          activateLedAndOrValve(x, LOW, 2);
        }
        else if (gameMode == 2){
          activateLedAndOrValve(x, LOW, 2);
        }   
        else if (gameMode == 3){
          sound(NOTE_D5);
          activateLedAndOrValve(x, LOW, 2);
        }                
      }
      else{
        if(beeper == HIGH){
          // Beep at user for being too early or too late
          tone(piezoPin,1000,250); 
        }
      }
    }
    else if(reward == 1){
      // Do nothing
    }
    toggle[x] = 1;
    // If preview mode is on generate a waitTime that falls in the same beat as rampInterval
    if (rampInterval != 0){
      onBeat = currentMillis % rampInterval;
      waitTime[x] = random(1,11) * rampInterval - onBeat;  
    }
    else{
      // generate random waitTime if no preview
      waitTime[x] = random(minWaitTime,maxWaitTime);
    }
    previousMillis[x] = currentMillis;   
    numPress++;
    return 1; 
  }
  if (analogRead(FSR_PIN[x]) < deactivateButton[x] && toggle[x] == 1){
    toggle[x] = 0;
    if(gameMode == 1 || gameMode == 3){
      activateLedAndOrValve(x, HIGH, 2);
      activateLedAndOrValve(x, LOW, 1);
    }   
    else if (gameMode == 2){
      activateLedAndOrValve(x, HIGH, 2);
    }
    return 0;
  }
}

//========================================================================
                                                                          
void ramping(int x, unsigned long previousMillis, unsigned long waitTime) {
  
  // This function ramps the LED and valves 
  // Check and update button status. If a button is pressed, it returns a 1
  if(buttonStatus(x)==1){
    // If user presses button anytime between ramping and cueTime BEEP at user
    if(currentMillis < previousMillis + waitTime + 4 * rampInterval + cushionTime){
      // If button is pressed before cue
      // Beep at the user!
    }
  }
  else{
    // Start Ramp  
    if (previousMillis + waitTime <= currentMillis && currentMillis < previousMillis + waitTime + blinkOn*rampInterval){
      //activateLedAndOrValve(x, LOW, gameMode);   
      // turn on 1st dot of valve/led       
      if (gameMode==1){
        activateOneDot(0,x,LOW,1);
      }
      else if (gameMode==2){
        activateOneDot(0,x,HIGH,2);
      }
      else if (gameMode==3){
//        activateOneDot(0,x,LOW,1);
//        activateOneDot(0,x,HIGH,2);
      }
    }
    else if (previousMillis + waitTime + blinkOn*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + rampInterval){   
      // turn off 1st dot of valve/led 
      if (gameMode==1){
        activateOneDot(0,x,HIGH,1);
      }
      else if (gameMode==2){
        activateOneDot(0,x,LOW,2);
      }
      else if (gameMode==3){
        sound(NOTE_C4);
//        activateOneDot(0,x,HIGH,1);
//        activateOneDot(0,x,LOW,2);
      }      
    }
    else if (previousMillis + waitTime + rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + (1+blinkOn)*rampInterval){   
      // turn on 2nd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,LOW,1);
        activateOneDot(1,x,LOW,1); 
      }
      else if (gameMode==2){
        activateOneDot(0,x,HIGH,2);
        activateOneDot(1,x,HIGH,2);
      }
      else if (gameMode==3){
        activateOneDot(0,x,LOW,1);
        activateOneDot(1,x,LOW,1);         
        activateOneDot(0,x,HIGH,2);
        activateOneDot(1,x,HIGH,2);
      }      
    }    
    else if (previousMillis + waitTime + (1+blinkOn)*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + 2*rampInterval){   
      // turn off 1st,2nd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,HIGH,1);
        activateOneDot(1,x,HIGH,1); 
      }
      else if (gameMode==2){
        activateOneDot(0,x,LOW,2);
        activateOneDot(1,x,LOW,2);
      }
      else if (gameMode==3){
        sound(NOTE_E4);
//        activateOneDot(0,x,HIGH,1);
//        activateOneDot(1,x,HIGH,1); 
//        activateOneDot(0,x,LOW,2);
//        activateOneDot(1,x,LOW,2);        
      }        
    }
    else if (previousMillis + waitTime + 2*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + (2+blinkOn)*rampInterval){   
      // turn on 1st,2nd,3rd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,LOW,1);
        activateOneDot(1,x,LOW,1); 
        activateOneDot(2,x,LOW,1);
      }
      else if (gameMode==2){
        activateOneDot(0,x,HIGH,2);
        activateOneDot(1,x,HIGH,2);
        activateOneDot(2,x,HIGH,2);
      }
      else if (gameMode==3){
//        activateOneDot(0,x,LOW,1);
//        activateOneDot(1,x,LOW,1); 
//        activateOneDot(2,x,LOW,1);
//        activateOneDot(0,x,HIGH,2);
//        activateOneDot(1,x,HIGH,2);
//        activateOneDot(2,x,HIGH,2);        
      }  
    }
    else if (previousMillis + waitTime + (2+blinkOn)*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + 3*rampInterval){   
      // turn on 1st,2nd,3rd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,HIGH,1);
        activateOneDot(1,x,HIGH,1); 
        activateOneDot(2,x,HIGH,1); 
      }
      else if (gameMode==2){
        activateOneDot(0,x,LOW,2); 
        activateOneDot(1,x,LOW,2); 
        activateOneDot(2,x,LOW,2); 
      }
      else if (gameMode==3){
        sound(NOTE_G4);
//        activateOneDot(0,x,HIGH,1);
//        activateOneDot(1,x,HIGH,1); 
//        activateOneDot(2,x,HIGH,1);
//        activateOneDot(0,x,LOW,2); 
//        activateOneDot(1,x,LOW,2);         
//        activateOneDot(2,x,LOW,2);               
      }  
    }  
    else if (previousMillis + waitTime + 3*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + (3+blinkOn)*rampInterval){   
      // turn on 1st,2nd,3rd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,LOW,1);
        activateOneDot(1,x,LOW,1); 
        activateOneDot(2,x,LOW,1);
        activateOneDot(3,x,LOW,1);         
      }
      else if (gameMode==2){
        activateOneDot(0,x,HIGH,2); 
        activateOneDot(1,x,HIGH,2);
        activateOneDot(2,x,HIGH,2);
        activateOneDot(3,x,HIGH,2); 
      }
      else if (gameMode==3){
//        activateOneDot(0,x,LOW,1);
//        activateOneDot(1,x,LOW,1); 
//        activateOneDot(2,x,LOW,1);
//        activateOneDot(3,x,LOW,1); 
//        activateOneDot(0,x,HIGH,2); 
//        activateOneDot(1,x,HIGH,2); 
//        activateOneDot(2,x,HIGH,2);
//        activateOneDot(3,x,HIGH,2); 
      }
    } 
    else if (previousMillis + waitTime + (3+blinkOn)*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + 4*rampInterval){   
      // turn on 1st,2nd,3rd dot of valve/led
      if (gameMode==1){
        activateOneDot(0,x,HIGH,1);
        activateOneDot(1,x,HIGH,1); 
        activateOneDot(2,x,HIGH,1);
        activateOneDot(3,x,HIGH,1); 
      }
      else if (gameMode==2){
        activateOneDot(0,x,LOW,2); 
        activateOneDot(1,x,LOW,2); 
        activateOneDot(2,x,LOW,2);
        activateOneDot(3,x,LOW,2);  
      }
      else if (gameMode==3){
        sound(NOTE_B4);
//        activateOneDot(0,x,HIGH,1);
//        activateOneDot(1,x,HIGH,1); 
//        activateOneDot(2,x,HIGH,1);
//        activateOneDot(3,x,HIGH,1); 
//        activateOneDot(0,x,LOW,2); 
//        activateOneDot(1,x,LOW,2); 
//        activateOneDot(2,x,LOW,2);
//        activateOneDot(3,x,LOW,2);               
      }       
      
    }            
    else if (previousMillis + waitTime + (4)*rampInterval <= currentMillis && currentMillis < previousMillis + waitTime + (4+blinkOn) * rampInterval){
      if (gameMode==1){
        activateOneDot(0,x,LOW,1);
        activateOneDot(1,x,LOW,1); 
        activateOneDot(2,x,LOW,1);
        activateOneDot(3,x,LOW,1);         
      }
      else if (gameMode==2){
        activateOneDot(0,x,HIGH,2);
        activateOneDot(1,x,HIGH,2); 
        activateOneDot(2,x,HIGH,2); 
        activateOneDot(3,x,HIGH,2);  
      }
      else if (gameMode==3){
//        activateOneDot(0,x,LOW,1);
//        activateOneDot(1,x,LOW,1); 
//        activateOneDot(2,x,LOW,1);
//        activateOneDot(3,x,LOW,1); 
//        activateOneDot(0,x,HIGH,2);
//        activateOneDot(1,x,HIGH,2); 
//        activateOneDot(2,x,HIGH,2); 
//        activateOneDot(3,x,HIGH,2);   
      }
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
  for(int i=xy; i < xy+4; i++) register_value[i]=state;
  update_register_values();  
}

//========================================================================

// This function controls the state of 4 valves
void valveControl(int xy, boolean state){
  for(int i=xy; i < xy+4; i++) register_value[i+16]=state;
  update_register_values();  
}

//========================================================================

// This function controls the state of 4 LEDs and Valves
void ledValveControl(int xy, boolean state){
  for(int i=xy; i < xy+4; i++){
    register_value[i]=state;
    register_value[i+16]=state;    
  }
  update_register_values();  
}

//========================================================================

void activateLedAndOrValve(int x, boolean state, int gameMode){
  switch (gameMode){
    case 1:    // visual cue only
      ledControl(4*x,state);
      break;
    case 2:    // haptics cue only
      valveControl(4*x,state);
      break;
    case 3:    // both haptics and visual cue
      ledControl(4*x,state);
      valveControl(4*x,state);
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
      break;
    case 2:    // haptics cue only
      register_value[4*x+dot+16]=state;
      break;
    case 3:    // both haptics and visual cue
      register_value[4*x+dot]=state;
      register_value[4*x+dot+16]=state;
      break;
    default:
      Serial.println("gameMode out of bounds");
  }
  update_register_values();  
} 

//========================================================================

void sound(int note){
  tone(piezoPin, note, (1-blinkOn)*rampInterval);
}

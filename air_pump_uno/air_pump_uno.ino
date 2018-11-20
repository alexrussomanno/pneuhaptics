/*
Update push
Last Edit: 11/18/18
hello this is a test
Control code for air pump. Uses pressure gage to keep output pressure range between pressure_max and pressure_min.


*/

int transducer = A0;
int button = 2;
int pump = 3;
int valve[]={8,9,10,11};
boolean valve_state[] = {LOW,LOW,LOW,LOW};

int pressure_max = 10;
int pressure_min = 8;
  
float calibration_pressure = 15;
float slope1; 
float int1;

void setup() {
  for (int i=0; i < 4; i++){
    pinMode(valve[i], OUTPUT);
  }
  pinMode(button,INPUT);
  pinMode(pump,OUTPUT);
  
  Serial.begin(9600); 
  delay(1000);
  Serial.println("Let's go!");
  // Run the following, enter slope and int values below, then comment out "run_pressure_calibration()"
//  run_pressure_calibration();
  slope1 = 0.0179211483;
  int1 = -1.8817205429;
}

void loop() {
  
  float output_pressure;
  int toggle;
    
  unsigned long current_time;
  unsigned long wait_time = 0;
  unsigned long last_time = 0;
  int output_pwm=0;
  
  digitalWrite(pump,LOW);
  Serial.println("Press Enter to start program.");
  while(Serial.read() != 10);
  
  while(Serial.read() != 10){
    current_time=millis();
    
    output_pressure = (float) slope1*analogRead(A0)+int1;
    Serial.print(output_pressure);
    Serial.print(" ");
//    if(output_pressure<pressure_max && toggle==0){
//      digitalWrite(pump,255);
//    }
//    else {
//      digitalWrite(pump,140);
//      toggle=1;
//      if(output_pressure < pressure_min) toggle=0;
//    }
//    
    if (current_time > last_time + wait_time){
      last_time=current_time;
      int diff = output_pressure - pressure_max;
      if(diff < 0 && output_pwm <= 255){
        if(diff < -2) output_pwm = output_pwm + 5;
        else output_pwm++;
      }
      else if (diff >=0 && output_pwm > 0) output_pwm--;      
//      if (output_pwm == 255){
//        output_pwm--;
//        Serial.print("Max output!!!"); 
//        Serial.print(" ");
//      }
      analogWrite(pump,output_pwm);
    }
    Serial.print(output_pwm);
    Serial.print(" ");
    
//    if (current_time > last_time + wait_time){
//      last_time=current_time;
//      
//      for(int i = 0; i<4;i++){
//        valve_state[i] =! valve_state[i];
//        digitalWrite(valve[i],valve_state[i]);
//      }
//      
//    }
//    for(int i = 0; i<4;i++){
//      Serial.print(valve_state[i]);
//      Serial.print(" ");
//    }
    Serial.println();
  }
  
  
  
}

void run_pressure_calibration(){
  Serial.println("Set pressure to 0 and press enter.");
  while (Serial.read() != 10);
  int v1 = (float)analogRead(transducer);

//   Serial.print("v1: ");
//   Serial.println(v1,DEC); 

  Serial.println("Set pressure to 15 and press enter");
  while (Serial.read() != 10);
  int v2 = (float)analogRead(transducer);
  
//   Serial.print("v2: ");
//   Serial.println(v2,DEC); 

  slope1 = (float) calibration_pressure/(v2-v1);
  Serial.print("slope1 = ");
  Serial.println(slope1,DEC); 

  int1 = (float)-v1*slope1;
  Serial.println("int1 = ");
  Serial.println(int1,DEC);
}

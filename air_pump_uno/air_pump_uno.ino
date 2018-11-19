/*

Last Edit: 11/18/18

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
//  pinMode(regulator,OUTPUT);
  for (int i=0; i < 4; i++){
    pinMode(valve[i], OUTPUT);
  }
  pinMode(button,INPUT);
  pinMode(pump,OUTPUT);
  
  Serial.begin(9600); 
  Serial.println("Let's go!");
  // Run the following, enter slope and int values below in loop, then comment out this section
//  pressure_calibration();
  slope1 = 0.0179211483;
  int1 = -1.8817205429;
}

void loop() {
  
  float output_pressure;
  int toggle;
    
  unsigned long current_time;
  unsigned long wait_time = 1000;
  unsigned long last_time = 0;
  
  
  digitalWrite(pump,LOW);
  Serial.println("Press Enter to start program.");
  while(Serial.read() != 10);
  
  while(Serial.read() != 10){
    current_time=millis();
    
    output_pressure = (float) slope1*analogRead(A0)+int1;
    Serial.print(output_pressure);
    Serial.print(" ");
    if(output_pressure<pressure_max && toggle==0){
      digitalWrite(pump,HIGH);
    }
    else {
      digitalWrite(pump,LOW);
      toggle=1;
      if(output_pressure < pressure_min) toggle=0;
    }
    
    if (current_time > last_time + wait_time){
      last_time=current_time;
      
      for(int i = 0; i<4;i++){
        valve_state[i] =! valve_state[i];
        digitalWrite(valve[i],valve_state[i]);
      }
      
    }
    for(int i = 0; i<4;i++){
      Serial.print(valve_state[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  
  
}
//  unsigned long prev_time = millis();
//  unsigned long current_time = millis();
//  long start_time = current_time;
//  int val =0;
//  while (Serial.read() != 10){
//    while (Serial.available() == 0){
//      current_time = millis();
//      int output_volt1 = analogRead(transducer1);
//      int output_volt2 = analogRead(transducer2);
//      int output_volt3 = analogRead(transducer3);
//      //      Serial.print(millis()-start_time);        Serial.print(" ");
//      //  //      Serial.print((float)i/255*5);  Serial.print(" ");
//      Serial.print(millis()-start_time);   
//      Serial.print(" ");
//      
//      for (int i=1; i < 8; i++){
//        Serial.print(digitalRead(valve[i]));
//      }
//      Serial.print(digitalRead(valve[8])); 
//      Serial.print("   ");
//      Serial.print(abs((float)output_volt1/1023*5.0*slope1+int1)); 
//      Serial.print(" ");
//      Serial.print(abs((float)output_volt2/1023*5.0*slope2+int2));  
//      Serial.print(" ");
//      Serial.println(abs((float)output_volt3/1023*5.0*slope3+int3));
////      Serial.print("   ");
//    }
//    val = Serial.parseInt();
//    Serial.read();
//    if (digitalRead(valve[val])==HIGH){
//      digitalWrite(valve[val],LOW);
//    }
//    else {
//      digitalWrite(valve[val],HIGH);
//    }
////    val = Serial.parseInt();
//  }

  //  int x = 1;
  //  for (int i = 0; i > -1; i = i + x){
  //      analogWrite(regulator,i);
  //      if (i == max_pressure) x = -1;             // switch direction at peak
  //      delay(time_step);
  //      int output_volt1 = analogRead(transducer1);
  //      int output_volt2 = analogRead(transducer2);
  //      Serial.print(millis());        Serial.print(" ");
  ////      Serial.print((float)i/255*5);  Serial.print(" ");
  //      Serial.print((float)output_volt1/1023*5.0*slope1+int1); Serial.print(" ");
  //      Serial.println((float)output_volt2/1023*5.0*slope2+int2); 
  //   } 



  //  while (Serial.read() != 10);
  //  int x = 1;
  //  unsigned long prev_time = millis();
  //  unsigned long current_time = millis();
  //  long start_time = current_time;
  //  int i = 1;

  ////  Electronic Pressure regulator
  //    while (i > -1){
  //      current_time = millis();
  //      if (current_time > prev_time + time_step){
  //        analogWrite(regulator,i);
  //        if (i == max_pressure) x = -1;
  //        i = i + x;
  //        prev_time = current_time;
  //      }





////    Manual pressure regulator
//  while (Serial.read() != 10){
//    current_time = millis();
//    int output_volt1 = analogRead(transducer1);
//    int output_volt2 = analogRead(transducer2);
//    Serial.print(millis()-start_time);        Serial.print(" ");
//    Serial.print((float)output_volt1/1023*5.0); Serial.print(" ");
//    Serial.println((float)output_volt2/1023*5.0); 
//  }

//   int x = 1;
//   for (int i = 0; i > -1; i = i + x){
//      analogWrite(regulator,i);
//      if (i == max_pressure) x = -1;             // switch direction at peak
//      delay(time_step);
//      int output_volt1 = analogRead(transducer1);
//      int output_volt2 = analogRead(transducer2);
//      Serial.print(millis());        Serial.print(" ");
////      Serial.print((float)i/255*5);  Serial.print(" ");
//      Serial.print((float)output_volt1/1023*5.0); Serial.print(" ");
//      Serial.println((float)output_volt2/1023*5.0); 
//   } 


//  for(int i=0;i<=150;i++){
//    analogWrite(2,i);
//    delay(time_step);
//    int output_volt = analogRead(transducer1);
//    Serial.print(millis());
//    Serial.print(" ");
//    Serial.print((float)i/255*5);
//    Serial.print(" ");
//    Serial.println((float)output_volt/1023*5.0);    
//  }
//  for(int i=99;i>=0;i--){
//    analogWrite(2,i);
//    delay(time_step);
//    int output_volt = analogRead(transducer1);
//    Serial.print(millis());
//    Serial.print(" ");
//    Serial.print((float)i/255*5);
//    Serial.print(" ");
//    Serial.println((float)output_volt/1023*5.0);
//  }

//  analogWrite(2,100);
//  Serial.println(analogRead(input));
void pressure_calibration(){
  Serial.println("Set pressure to 0 and press enter");
  while (Serial.read() != 10);
  int v1 = (float)analogRead(transducer);

  Serial.print("v1: ");
  Serial.println(v1,DEC); 

  Serial.println("Set pressure to 15 and press enter");
  while (Serial.read() != 10);
  int v2 = (float)analogRead(transducer);
  
  Serial.print("v2: ");
  Serial.println(v2,DEC); 

  slope1 = (float) calibration_pressure/(v2-v1);
  Serial.println("Slope: ");
  Serial.println(slope1,DEC); 

  int1 = (float)-v1*slope1;
  Serial.print("Y-int: ");
  Serial.println(int1,DEC); 
}


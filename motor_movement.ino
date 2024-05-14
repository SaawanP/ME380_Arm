#include <Servo.h>

Servo base; // gear ratio is 26:20
Servo link1;
Servo link2;
Servo claw;

int e_stop = 5;
int start_button = 6;

int time_needed = 0;
int start_time = millis();
int completed_object = 0;
int phase = 1;
int color = -1;
bool start = true;
int run_time;
int freq = 0;
int freq_r = 0;
int freq_g = 0;
int freq_b = 0;

int S0 = 48;
int S1 = 46;
int S2 = 44;
int S3 = 42;
int sensorOut = 40;

void calibrate() {
  
}


int getobjectcolor() {
  int ret = -1;
  
  do {
    // Setting red filtered photodiodes to be read
    digitalWrite(S2,LOW);
    digitalWrite(S3,LOW);
    // Reading the output frequency
    freq = pulseIn(sensorOut, LOW);
    freq_r = freq;
  
    // Setting Green filtered photodiodes to be read
    digitalWrite(S2,HIGH);
    digitalWrite(S3,HIGH);
    // Reading the output frequency
    freq = pulseIn(sensorOut, LOW);
    freq_g = freq;
  
    // Setting Blue filtered photodiodes to be read
    digitalWrite(S2,LOW);
    digitalWrite(S3,HIGH);
    // Reading the output frequency
    freq = pulseIn(sensorOut, LOW);
    freq_b = freq;
    
    if (freq_r >= 17 && freq_r < 30 && freq_b > 17) {
      ret = 0;
    }
    if (freq_r > 5 && freq_r <= 13 && freq_g > 5 && freq_g < 15) {
      ret = 1;
    }
    if (freq_r >= 15 && freq_r < 21 && freq_g >= 17 && freq_g < 25 && freq_b < 18) {
      ret = 2;
      Serial.println("light blue");
    }
    if (freq_r >= 13 && freq_r < 21 && freq_g >= 12 && freq_g < 16 && freq_b < 14 && freq_b > 7){
      ret = 2;
      Serial.println("blue");
    }
    if (freq_r >= 13 && freq_r < 21 && freq_g >= 12 && freq_g < 25 && freq_b < 18){
      ret = 2;
      Serial.println("combined");
    }
  }while(ret == -1);
  
  return ret;
}

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // Setting frequency-scaling to 100%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,HIGH);
  Serial.begin(9600);
  
  base.attach(8);
//  link1.attach(9);
  link2.attach(10);
  claw.attach(11);

  base.write(0);
//  link1.write(45);
  link2.write(35); 
  claw.write(70);
  delay(1000);
  Serial.begin(9600);
  run_time = millis();
}

void loop() {
//  base.write(0);
//  delay(1000);
//  base.write(85*20/26);
//  delay(1000);
//  return;

  if (start && millis()-start_time > time_needed) {
    switch (phase) {
      case 0: // go to start
        base.write(0); 
        link2.write(35);
        claw.write(70);
        phase = 1;
        time_needed = 1000;
        break;
      case 1: // go down for object
        link2.write(20);
        phase = 2;
        time_needed = 200;
        break;
      case 2: // grab object
        claw.write(0);
        phase = 3;
        time_needed = 100;
        break;
      case 3: // go to end
        color = getobjectcolor();
        if (color == 0) { // black
          base.write(90*20/26); 
          link2.write(62);
        }
        else if (color == 1) { // white
          base.write(70*20/26);
          link2.write(20);
        }
        else if (color == 2) { // blue
          base.write(40*20/26); 
          link2.write(80);
          delay(150);
          claw.write(70);
        }
        phase = 4;
        time_needed = 1000;
        break;
      case 4: // drop object
        claw.write(90);
        phase = 5;
        completed_object++;
        time_needed = 100;
        if (completed_object == 3) {
          completed_object = 0;
          start = false;
          Serial.println("completed");
          
          Serial.println(millis()-run_time);
        }
        break;
      case 5: // raise away from object
        time_needed = 0;
        if (color != 2) {
          int height = link2.read();
          link2.write(height + 20);
          time_needed = 200;
        }
        phase = 0;
        break;
    }
    start_time = millis();
  }
}

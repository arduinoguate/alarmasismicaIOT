int sound = 13;
int vibr_Pin =3;

void setup(){
  pinMode(sound , OUTPUT);
  pinMode(vibr_Pin, INPUT); 
  Serial.begin(9600); 
}
void loop(){
  long measurement =TP_init();
  delay(50);
 // Serial.print("measurment = ");
  Serial.println(measurement);
  if (measurement > 1000){
    digitalWrite(sound, HIGH);
  }
  else{
    digitalWrite(sound, LOW); 
  }
}

long TP_init(){
  delay(10);
  long measurement=pulseIn (vibr_Pin, HIGH);  //
}

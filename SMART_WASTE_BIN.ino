
#define BLYNK_PRINT DebugSerial


// You could use a spare Hardware Serial on boards that have it (like Mega)
#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(11,12),gpsSerial(13,12); // RX, TX

#include <BlynkSimpleStream.h>
#include<LiquidCrystal.h>
#include<TinyGPS.h>
char auth[] = "wmtU7utGn4P4nS8iwsQwQpe04v5aN6Pr";

LiquidCrystal lcd(2,3,4,5,6,7);
int ultra_pins[]={9,8};
int mois_pin=A0;
int plate_pin[]={A1,A2};


TinyGPS gps;
WidgetMap myMap(V9);

void setup()
{
  // Debug console
  DebugSerial.begin(9600);
  gpsSerial.begin(9600);
  // Blynk will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
  lcd.begin(16,2);
  pinMode(mois_pin,INPUT);
  pinMode(plate_pin,OUTPUT);
  pinMode(ultra_pins[0],OUTPUT);
  pinMode(ultra_pins[1],INPUT);
}

void loop()
{
  Blynk.run();
  sendLocation();
  check_status();
}

BLYNK_WRITE(V8){
  if(param.asInt()){
    check_garbage(800);
  }else{
    set_default(100);
  }
}


void set_default(int duration){
  
}

void check_garbage(int duration){
  int d;
  if((d=digitalRead(mois_pin))){
    Blynk.virtualWrite(V20,d);
    Blynk.virtualWrite(V10,".....");
    Blynk.virtualWrite(V11,"True");
    lcd.setCursor(0,0);
    lcd.print("Waste: Wet");
    digitalWrite(plate_pin[0],HIGH);
    delay(duration);
    digitalWrite(plate_pin[0],LOW);
    delay(1000);

    digitalWrite(plate_pin[1],HIGH);
    delay(duration);
    digitalWrite(plate_pin[1],LOW);
  }else{
    lcd.setCursor(0,0);
    lcd.print("Waste: DRY");
    Blynk.virtualWrite(V11,".....");
    Blynk.virtualWrite(V10,"DRY");
    digitalWrite(plate_pin[1],HIGH);
    delay(duration);
    digitalWrite(plate_pin[1],LOW);
    delay(1000);

    digitalWrite(plate_pin[0],HIGH);
    delay(duration);
    digitalWrite(plate_pin[0],LOW);
  }
}

void check_status(){
  digitalWrite(ultra_pins[0],LOW);
  delayMicroseconds(2);
  digitalWrite(ultra_pins[0],HIGH);
  delayMicroseconds(10);
  digitalWrite(ultra_pins[0],LOW);
  int duration=pulseIn(ultra_pins[1],HIGH);
  float dist=(duration*0.0343)/2;
  Blynk.virtualWrite(V12,dist);
  //Serial.print("Dist: ");
  //Serial.println(dist);
  if(dist<=3.0){
    //sendSMS();
    sendLocation();
    lcd.setCursor(0,1);
    lcd.print("BIN: Full");
    Blynk.notify("Dustbin Overflow");
    Blynk.email("Dustbin Status","Dustbin FUll");
    Blynk.virtualWrite(V13,"FULL");
    Blynk.virtualWrite(V14,dist);
        sendLocation();
  }else{
    Blynk.virtualWrite(V13,".....");
    lcd.setCursor(0,1);
    lcd.print("BIN: ....");
    Blynk.virtualWrite(V14,".....");
  }
}


void sendLocation(){
  while(gpsSerial.available()){
    char ch=gpsSerial.read();
    if(gps.encode(ch)){
      float lat,lon;
      gps.f_get_position(&lat,&lon);
      myMap.location(1,lat,lon,"BIN");
      Blynk.virtualWrite(V15,lat);
      Blynk.virtualWrite(V16,lon);
    }
  }
}

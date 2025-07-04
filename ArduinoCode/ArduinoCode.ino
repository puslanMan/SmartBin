#include <DHT.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define IR_PIN 9
#define INDUCTIVE_PIN 10

#define MAIN_SERVO_PIN 4
Servo mainServo;

#define FLOOR_SERVO_PIN_1 2
Servo floorServo1;

#define FLOOR_SERVO_PIN_2 8
Servo floorServo2;

#define LED_PIN A3

#define BUZZER_PIN 13

#define TRIG_PIN 12

#define METALIC_ECHO_PIN 5
#define DRY_ECHO_PIN 6
#define WET_ECHO_PIN 11

unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 2000; // 2 seconds

int mainServoAngle = 100; //current angle of mainServo;

char STATE_IDLE = 'a',
     STATE_IDENTIFYING = 'b',
     STATE_DUMPING = 'c',
     STATE_ERROR = 'e';

char currentState = STATE_IDLE;

char TYPE_WET = 'w',
     TYPE_DRY = 'd',
     TYPE_METAL = 'm';

char garbageType = TYPE_DRY;

int DETECTIONPHASE_1 = 1,
    DETECTIONPHASE_2 = 2;

int currentDetectionPhase = DETECTIONPHASE_1;

int detectionCount = 0,
    metallicCount = 0,
    nonMetallicCount = 0;;

float lastHumidityReading = 0,
      firstReading = -1,
      secondReading = -1,
      thirdReading= -1;
int dhtCount = 0;

int MOISTURE_PIN = A0;

char REQUEST_INFO = '!';

boolean isMetallicFull,
        isDryFull,
        isWetFull;

int metallicFullCount,
    dryFullCount,
    wetFullCount;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  dht.begin();

  mainServo.attach(MAIN_SERVO_PIN);
  floorServo1.attach(FLOOR_SERVO_PIN_1);
  floorServo2.attach(FLOOR_SERVO_PIN_2);

  pinMode(IR_PIN, INPUT);
  pinMode(INDUCTIVE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  mainServo.write(mainServoAngle);
  floorServo1.write(180);
  floorServo2.write(0);
  currentState = STATE_IDLE;
  garbageType = TYPE_DRY;
  currentDetectionPhase = DETECTIONPHASE_1;

  detectionCount = 0;
  metallicCount = 0;
  nonMetallicCount = 0;

  lastHumidityReading = 0,
  firstReading = -1,
  secondReading = -1,
  thirdReading= -1,
  dhtCount = 0;

  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(METALIC_ECHO_PIN, INPUT);
  pinMode(WET_ECHO_PIN, INPUT);
  pinMode(DRY_ECHO_PIN, INPUT);

  isMetallicFull = false;
  isDryFull = false;
  isWetFull = false;
  metallicFullCount=0;
  dryFullCount=0;
  wetFullCount=0;

  mainServo.detach();
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");

}

//------------------------------------ function to read the distance using HC-SR04 sensor -----------------------------------------
long readDistance(int echoPin) {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}
//---------------------------------------------------------------------------------------------------------------------------------

//------------------------------- function to read the moisture using YL-100 sensor -----------------------------------------------
char identifyBasedOnMoisture(int moisturePin){
  int moistureState = analogRead(moisturePin);
  if(moistureState < 200){
    return TYPE_DRY;
  }else{
    return TYPE_WET;
  }
}
//---------------------------------------------------------------------------------------------------------------------------------

//------------------------------------- function to inform bin is full ------------------------------------------------------------
void binIsFullError(char garbageType){
  Serial.print("tE");
  Serial.println(garbageType);
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  mainServo.write(50);
  delay(500);
  floorServo1.write(90);
  floorServo2.write(90);

  delay(1000);
  int currentAngle1 = 90,
  currentAngle2 = 90;
  while(currentAngle1<180){
    currentAngle1=currentAngle1+1;
    floorServo1.write(currentAngle1);
    currentAngle2=currentAngle2-1;
    floorServo2.write(currentAngle2);
    delay(5);
  }
  delay(100);
}
//---------------------------------------------------------------------------------------------------------------------------------


void loop() {
  lcd.clear();
    String info="";
    if(isMetallicFull){
      info="Metal";
    }
    if(isDryFull){
      if(info.length() == 0){
        info="Dry";
      }else{
        info=info+",Dry";
      }
    }
    if(isWetFull){
      if(info.length() == 0){
        info="Wet";
      }else{
        info=info+",Wet";
      }
    }
    lcd.setCursor(0,1);
    if(info.length()==0){
      lcd.print("All bins are ok");
    }else{
      lcd.print("!["+info+"]");
    }
    
  unsigned long currentMillis = millis();
  if(currentState == STATE_IDENTIFYING){
    lcd.setCursor(0, 0);
    lcd.print("Identifying...");
    int sensedMetal = digitalRead(INDUCTIVE_PIN);
    if(currentDetectionPhase == DETECTIONPHASE_1){
      if (sensedMetal == LOW) {
        metallicCount++;
      }else{
        nonMetallicCount++;  
      }

      if(detectionCount>3){
        if(metallicCount>nonMetallicCount){
          garbageType = TYPE_METAL;
          currentState = STATE_DUMPING;
        }else{
          currentDetectionPhase = DETECTIONPHASE_2;
        }
      }
      
      detectionCount++;
    }else{
      if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
        float value = dht.readHumidity();  
        if (!isnan(value)){
          if(dhtCount == 0){
            firstReading = value;
          }else if(dhtCount == 1){
            secondReading = value;
          }else{
            thirdReading = value;
          }
          dhtCount++;                
        }

        lastDHTRead = currentMillis;
        if(dhtCount>=3){
          if(firstReading >= lastHumidityReading){
            if((thirdReading >= secondReading) && (secondReading >= firstReading) && (thirdReading-firstReading >= 0.3)){
              garbageType = TYPE_WET;
            }else{
              garbageType = identifyBasedOnMoisture(MOISTURE_PIN);
            }
          }else{
            garbageType = identifyBasedOnMoisture(MOISTURE_PIN);
          }
          
          lastHumidityReading=thirdReading;
          currentState = STATE_DUMPING;
        }
      }

    }
    
  }else if(currentState == STATE_DUMPING){
    lcd.setCursor(0, 0);
    mainServo.attach(MAIN_SERVO_PIN);
    if(garbageType == TYPE_WET){
      if(isWetFull){
        lcd.print("Wet Bin Full");
        binIsFullError(garbageType);
      }else{
        lcd.print("Wet Trash");
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(800);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        mainServo.write(0);
        delay(500);
        floorServo1.write(90);
        floorServo2.write(90);

        delay(1000);
        int currentAngle1 = 90,
            currentAngle2 = 90;
        while(currentAngle1<180){
          currentAngle1=currentAngle1+1;
          floorServo1.write(currentAngle1);
          currentAngle2=currentAngle2-1;
          floorServo2.write(currentAngle2);
          delay(5);
        }
        delay(100);
        Serial.print("tS");
        Serial.println(garbageType);
        delay(100);
      }
    }else if(garbageType == TYPE_DRY){
      if(isDryFull){
        lcd.print("Dry Bin Full");
        binIsFullError(garbageType);
      }else{
        lcd.print("Dry Trash");
        delay(500);
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        mainServo.write(120);
        delay(500);
        floorServo1.write(90);
        floorServo2.write(90);

        delay(1000);
        int currentAngle1 = 90,
            currentAngle2 = 90;
        while(currentAngle1<180){
          currentAngle1=currentAngle1+1;
          floorServo1.write(currentAngle1);
          currentAngle2=currentAngle2-1;
          floorServo2.write(currentAngle2);
          delay(5);
        }
        Serial.print("tS");
        Serial.println(garbageType);
        delay(100);
      }
      
    }else{
      if(isMetallicFull){
        lcd.print("Metal Bin Full");
        binIsFullError(garbageType);
      }else{
        lcd.print("Metallic Trash");
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1500);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        delay(100);
        mainServo.write(180);
        delay(500);
        floorServo1.write(90);
        floorServo2.write(90);

        delay(1000);
        int currentAngle1 = 90,
            currentAngle2 = 90;
        while(currentAngle1<180){
          currentAngle1=currentAngle1+1;
          floorServo1.write(currentAngle1);
          currentAngle2=currentAngle2-1;
          floorServo2.write(currentAngle2);
          delay(5);
        }
        Serial.print("tS");
        Serial.println(garbageType);
        delay(100);
      }
      
    }
    mainServo.write(mainServoAngle);
    garbageType = TYPE_DRY;
    currentDetectionPhase = DETECTIONPHASE_1;
    detectionCount = 0;
    metallicCount = 0;
    nonMetallicCount = 0;

    firstReading = -1;
    secondReading = -1;
    thirdReading= -1;
    dhtCount = 0;
    currentState = STATE_IDLE;
    delay(1000);
    mainServo.detach();
  }else if(currentState == STATE_ERROR){
    lcd.setCursor(0, 0);
    lcd.print("Error!");
    garbageType = TYPE_DRY;
    currentDetectionPhase = DETECTIONPHASE_1;
    detectionCount = 0;
    metallicCount = 0;
    nonMetallicCount = 0;

    firstReading = -1;
    secondReading = -1;
    thirdReading= -1;
    dhtCount = 0;
    currentState = STATE_IDLE;
  }else{
    lcd.setCursor(0, 0);
    lcd.print("Idle");
    if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
      lastHumidityReading = dht.readHumidity();
      lastDHTRead = currentMillis;
    }

    if(currentState == STATE_IDLE){
      int sensedHand = digitalRead(IR_PIN);
      if(sensedHand == LOW){
        delay(100);
        garbageType = TYPE_DRY;
        currentDetectionPhase = DETECTIONPHASE_1;
        detectionCount = 0;
        metallicCount = 0;
        nonMetallicCount = 0;
    
        firstReading = -1;
        secondReading = -1;
        thirdReading= -1;
        dhtCount = 0;

        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1000);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);

        currentState = STATE_IDENTIFYING;
      }
    }
    
  }
///*
  long dist = readDistance(METALIC_ECHO_PIN);
  /*
  Serial.print("metallicFullCount: ");
  Serial.print(metallicFullCount);
  Serial.print(" - ");
  Serial.print("Metallic distance: ");
  Serial.print(dist);
  Serial.println(" cm");//*/
  if(dist<=10){
    if(!isMetallicFull){
      if(metallicFullCount<5){
        metallicFullCount++;
      }else{
        isMetallicFull=true;
        Serial.println("iFm");
      }
    }
  }else{
    if(isMetallicFull){
      if(metallicFullCount>0){
        metallicFullCount--;
      }else{
        isMetallicFull=false;
        Serial.println("iRm");
      }
    }
  }
  //*/
  delay(60);

  dist = readDistance(DRY_ECHO_PIN);
  /*
  Serial.print("dryFullCount: ");
  Serial.print(dryFullCount);
  Serial.print(" - ");
  Serial.print("Dry distance: ");
  Serial.print(dist);
  Serial.println(" cm");//*/
  ///*
  if(dist<=10){
    if(!isDryFull){
      if(dryFullCount<5){
        dryFullCount++;
      }else{
        isDryFull=true;
        Serial.println("iFd");
      }
    }
  }else{
    if(isDryFull){
      if(dryFullCount>0){
        dryFullCount--;
      }else{
        isDryFull=false;
        Serial.println("iRd");
      }
    }
  }//*/
  delay(60);

  dist = readDistance(WET_ECHO_PIN);
  /*
  Serial.print("wetFullCount: ");
  Serial.print(wetFullCount);
  Serial.print(" - ");
  Serial.print("Wet distance: ");
  Serial.print(dist);
  Serial.println(" cm");//*/
  
  ///*
  if(dist<=10){
    if(!isWetFull){
      if(wetFullCount<5){
        wetFullCount++;
      }else{
        isWetFull=true;
        Serial.println("iFw");
      }
    }
  }else{
    if(isWetFull){
      if(wetFullCount>0){
        wetFullCount--;
      }else{
        isWetFull=false;
        Serial.println("iRw");
      }
    }
  }//*/
  delay(60);
  if(Serial.available()){
    char requestChar = Serial.read();
    if(requestChar == REQUEST_INFO){
      Serial.print("i");
      Serial.print(isMetallicFull);
      Serial.print(isDryFull);
      Serial.println(isWetFull);
    }
  }

  delay(500);
}
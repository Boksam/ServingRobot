/*
  모터 PWM 255 / 2.5 로 고정
  IR센서 하얀색: 0, 검은색: 1
*/
// Motor pin Setting
#define MOTOR1_DIR1 A0//12
#define MOTOR1_DIR2 A1//13
#define MOTOR2_DIR1 A2//7
#define MOTOR2_DIR2 A3//8
#define MOTOR1_ENA 10
#define MOTOR2_ENA 11

// HyperSonic Sensor Setting
#define HC_TRIG 9
#define HC_ECHO 8

// Line-Tracer IR Sensor Setting
#define IR_MIDDLE_LEFT 5
#define IR_MIDDLE_RIGHT 6
#define IR_RIGHT 7

#define CUSTOMER1_BTN 2
#define CUSTOMER2_BTN 3
#define RETREIVE_BTN 4

bool isStop = false;
volatile int targetCustomer = -1;
int intersectionCount = 0;
int previousIrRight = LOW;


void motorInit() {
  pinMode(MOTOR1_DIR1, OUTPUT);
  pinMode(MOTOR1_DIR2, OUTPUT);
  pinMode(MOTOR1_ENA, OUTPUT);
  pinMode(MOTOR2_DIR1, OUTPUT);
  pinMode(MOTOR2_DIR2, OUTPUT);
  pinMode(MOTOR2_ENA, OUTPUT);

  analogWrite(MOTOR1_ENA, 0);
  analogWrite(MOTOR2_ENA, 0);
}

void hcInit() {
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
}

void irInit() {
  pinMode(IR_MIDDLE_LEFT, INPUT);
  pinMode(IR_MIDDLE_RIGHT, INPUT);
  pinMode(IR_RIGHT, INPUT);
}

void setTargetCustomer1() {
  targetCustomer = 1;
}

void setTargetCustomer2() {
  targetCustomer = 2;
}

void btnInit() {
  pinMode(CUSTOMER1_BTN, INPUT_PULLUP);
  pinMode(CUSTOMER2_BTN, INPUT_PULLUP);
  pinMode(RETREIVE_BTN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(2), setTargetCustomer1, RISING);
  attachInterrupt(digitalPinToInterrupt(3), setTargetCustomer2, RISING);
}

void moveForward() {
  analogWrite(MOTOR1_ENA, 255 / 2.5);
  analogWrite(MOTOR2_ENA, 255 / 2.5);

  digitalWrite(MOTOR1_DIR1, LOW);
  digitalWrite(MOTOR1_DIR2, HIGH);
  digitalWrite(MOTOR2_DIR1, LOW);
  digitalWrite(MOTOR2_DIR2, HIGH);
}

void moveBackward() {
  analogWrite(MOTOR1_ENA, 255 / 2.5);
  analogWrite(MOTOR2_ENA, 255 / 2.5);

  digitalWrite(MOTOR1_DIR1, HIGH);
  digitalWrite(MOTOR1_DIR2, LOW);
  digitalWrite(MOTOR2_DIR1, HIGH);
  digitalWrite(MOTOR2_DIR2, LOW);
}

void stop() {
  analogWrite(MOTOR1_ENA, 0);
  analogWrite(MOTOR2_ENA, 0);
}

void turnRight() {
  analogWrite(MOTOR1_ENA, 255 / 2.5);
  analogWrite(MOTOR2_ENA, 255 / 2.5);

  digitalWrite(MOTOR1_DIR1, LOW);
  digitalWrite(MOTOR1_DIR2, HIGH);
  digitalWrite(MOTOR2_DIR1, HIGH);
  digitalWrite(MOTOR2_DIR2, LOW);
}

void turnLeft() {
  analogWrite(MOTOR1_ENA, 255 / 2.5);
  analogWrite(MOTOR2_ENA, 255 / 2.5);

  digitalWrite(MOTOR1_DIR1, HIGH);
  digitalWrite(MOTOR1_DIR2, LOW);
  digitalWrite(MOTOR2_DIR1, LOW);
  digitalWrite(MOTOR2_DIR2, HIGH);
}

void uTurn() {
  analogWrite(MOTOR1_ENA, 255 / 2.5);
  analogWrite(MOTOR2_ENA, 255 / 2);

  digitalWrite(MOTOR1_DIR1, LOW);
  digitalWrite(MOTOR1_DIR2, HIGH);
  digitalWrite(MOTOR2_DIR1, HIGH);
  digitalWrite(MOTOR2_DIR2, LOW);
}

int getDistance() {
  long duration, distance;

  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);

  duration = pulseIn(HC_ECHO, HIGH);
  distance = duration * 17 / 1000;
  return distance;
}

void detectIntersection() {
  int irRight = digitalRead(IR_RIGHT);
  if (irRight == HIGH) {
    if (previousIrRight == LOW) {
      intersectionCount++;
      previousIrRight = HIGH;
    }
  }
  else {
    previousIrRight = LOW;
  }
}

void move() {
  int irMiddleLeft = digitalRead(IR_MIDDLE_LEFT);
  int irMiddleRight = digitalRead(IR_MIDDLE_RIGHT);
  if (irMiddleLeft == HIGH && irMiddleRight == HIGH) {
    moveForward();
  }
  else if (irMiddleLeft == LOW && irMiddleRight == HIGH) {
    turnRight();
  }
  else if (irMiddleLeft == HIGH && irMiddleRight == LOW) {
    turnLeft();
  }
  else {
    stop();
  }
}

void waitUntilRetrieve() {
  while (true) {
    int retrieveBtnPressed = digitalRead(RETREIVE_BTN);
    if (retrieveBtnPressed == LOW) {
      return;
    }
  }
}

void goToCustomer1() {
  while (targetCustomer != intersectionCount) {
    detectIntersection();
    move();
  }
}

void goToCustomer2() {
  turnRight();
  delay(500);
  while (digitalRead(IR_MIDDLE_RIGHT) == HIGH) {continue;}
  while (digitalRead(IR_MIDDLE_RIGHT) == LOW) {continue;}
  while (true) {
    if (digitalRead(IR_MIDDLE_LEFT) == LOW
      && digitalRead(IR_MIDDLE_RIGHT) == LOW) {
      break;
    }
    move();
  }
  stop();
  waitUntilRetrieve();
}

void goBack() {
  uTurn();
  delay(500);
  while (digitalRead(IR_MIDDLE_RIGHT) == LOW) {continue;}
  while (digitalRead(IR_RIGHT) == LOW) {
    move();
  }
  turnLeft();
  delay(500);
  while (digitalRead(IR_MIDDLE_LEFT) == HIGH) {continue;}
  while (digitalRead(IR_MIDDLE_LEFT) == LOW) {continue;}
  while (true) {
    if (digitalRead(IR_MIDDLE_LEFT) == LOW
      && digitalRead(IR_MIDDLE_RIGHT) == LOW) {
      break;
    }
    move();
  }
  uTurn();
  while (digitalRead(IR_MIDDLE_RIGHT) == LOW) {continue;}
  stop();
  targetCustomer = -1;
}


void setup() {
  Serial.begin(9600);

  motorInit();
  irInit();
  btnInit();
  hcInit();

  targetCustomer = -1;

  digitalWrite(MOTOR1_DIR1, LOW);
  digitalWrite(MOTOR1_DIR2, HIGH);
  digitalWrite(MOTOR2_DIR1, LOW);
  digitalWrite(MOTOR2_DIR2, HIGH);
}

void loop() {
  while (getDistance() < 10) {stop();}

  while (targetCustomer < 0) {stop();}

  goToCustomer1();
  
  goToCustomer2();
  
  goBack();
}

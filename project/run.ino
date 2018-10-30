/* This file controls the robot
* Communication : Bluetooth
* Auto turning : IR
*/

#include <Servo.h>
#include <SoftwareSerial.h>

Servo servoLeft, servoRight;

int val = 0;
int tempSensor = A1;
int lightSensor = A0;

unsigned long lastmove = 0;

// 0:stright 1:turn left 2:turn right
int inprocess = 0;

#define irRightLED 9
#define irRightReceive 10
#define irLeftLED 2
#define irLeftReceive 3

#define delaytime 1200
#define RxD 7
#define TxD 6

#define DEBUG_ENABLED 1

#define STRAIGHT 0
#define LEFT 1
#define RIGHT 2
#define STOP 3
#define BACK 4

SoftwareSerial blueToothSerial(RxD,TxD);

//save rebot state
struct state{
  int irLeft;
  int irRight;
  int light;
  double temp;
};

void setupBlueToothConnection()
{
    //setup BlueTooth use name SlaveABC
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=SlaveABC+\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.

    blueToothSerial.flush();
}

void setup()
{
  //setup serial
  Serial.begin(9600);

  //IR setup
  pinMode(irRightLED, OUTPUT); // IR Sensor
  pinMode(irRightReceive, INPUT);
  pinMode(irLeftLED, OUTPUT);
  pinMode(irLeftReceive, INPUT);

  //servo setup
  servoLeft.attach(13);
  servoRight.attach(12);

  //bluetooth setup
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
}

int irDetect(int irLedPin, int irReceiverPin, long frequency)
{
  //dectect whether there is obstacle
  tone(irLedPin, frequency, 8);
  delay(1);
  int ir = digitalRead(irReceiverPin);
  delay(1);
  return ir;
}

void output()
{
  //just for debug
  //not used
  int val = analogRead(tempSensor);             // Pin of Temp Sensor using
  double tempv = val * 5000.0 / 1024.0; // Convert the unit of Temp
  double c = (tempv - 750) / 10 + 25;
  Serial.println(c); // Print the value of Temp
}

void turn(int direction){
  //control the Servo to turn 
  //0 -> stright
  //1 -> left
  //2 -> right
  //3 -> stop
  inprocess = direction;
  switch (direction)
  {
    //go back 
    //servoLeft.writeMicroseconds(1700);
    //servoRight.writeMicroseconds(1300);
    case STRAIGHT:
      servoLeft.writeMicroseconds(1300);
      servoRight.writeMicroseconds(1700);
      break;
    case LEFT:
      servoLeft.writeMicroseconds(1700);
      servoRight.writeMicroseconds(1500);
      break;
    case RIGHT:
      servoLeft.writeMicroseconds(1500);
      servoRight.writeMicroseconds(1300);
      break;
    case STOP:
      servoLeft.writeMicroseconds(1500);
      servoRight.writeMicroseconds(1500);
  }
}

void control(struct state s){
  //control the robot to turn according to IR results
  if(inprocess != 0 && millis() - lastmove <= delaytime)// if not turn 90 degree yet
    return; // continue turn until 90 degree

  int left = s.irLeft;
  int right = s.irRight;
  if (left == 0 || right == 0)// should turn 
  {
    lastmove = millis();

    turn(0);
    delay(400);
    //go back
    //servoLeft.writeMicroseconds(1700);
    //servoRight.writeMicroseconds(1300);
    if (right == 0)//turn right
    {
      turn(1);
    }
    else if (left == 0)//turn left
    {
      turn(2);
    }
  }
  else //go straight
  {
    turn(0);
  }
}

struct state sense(){
  //use sensors to get temprature and lightness
  struct state c;
  c.irLeft = irDetect(irLeftLED, irLeftReceive, 38000);
  c.irRight = irDetect(irRightLED, irRightReceive, 38000);

/*
  Serial.print(c.irLeft);
  Serial.print(' ');
  Serial.println(c.irRight);
  */
  int val = analogRead(tempSensor);     // Pin of Temp Sensor using
  double tempv = val * 5000.0 / 1024.0; // Convert the unit of Temp
  c.temp = (tempv - 750) / 10 + 25 - 290;
  c.light = analogRead(lightSensor);
  return c;
}

void senddata(struct state c){
  //send data via bluetooth
  blueToothSerial.print(inprocess);
  blueToothSerial.print(' ');
  blueToothSerial.print(c.light);
  blueToothSerial.print(' ');
  blueToothSerial.print(c.temp);
  blueToothSerial.println('@');
}

char recvdata(){
  //receive data via bluetooth
  char recvChar = 255;
  if (blueToothSerial.available())
  {
    recvChar = blueToothSerial.read();
  }
  return recvChar;
}

void loop()
{
  //main loop
  struct state c = sense();//get state
  control(c);//control turning
  senddata(c);//send data

  //get data to see if there is command
  char recvChar = recvdata();
  if (recvChar == 0 || recvChar == 1 || recvChar == 2 || recvChar == 3)
  {
    turn(recvChar);
  }

  delay(100); //100 ms interval
}

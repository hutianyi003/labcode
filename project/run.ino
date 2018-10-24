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

SoftwareSerial blueToothSerial(RxD,TxD);

struct state{
  int irLeft;
  int irRight;
  int light;
  double temp;
};

void setupBlueToothConnection()
{
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

  Serial.begin(9600);
  tone(4, 3000, 10000);
  delay(1000);

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
  tone(irLedPin, frequency, 8);
  delay(1);
  int ir = digitalRead(irReceiverPin);
  delay(1);
  return ir;
}

void output()
{

  int val = analogRead(tempSensor);             // Pin of Temp Sensor using
  double tempv = val * 5000.0 / 1024.0; // Convert the unit of Temp
  double c = (tempv - 750) / 10 + 25;
  Serial.println(c); // Print the value of Temp
}

void control(struct state s){
  //if(inprocess != 0 && millis() - lastmove <= delaytime)
  //  return;
  int left = s.irLeft;
  int right = s.irRight;
  if (left == 0 || right == 0)
  {
    lastmove = millis();

    //go back 
    //servoLeft.writeMicroseconds(1700);
    //servoRight.writeMicroseconds(1300);
    if (left == 0)//turn left
    {
      servoLeft.writeMicroseconds(1500);
      servoRight.writeMicroseconds(1300);
      inprocess = 1;
    }
    else if (right == 0)//turn right
    {
      servoLeft.writeMicroseconds(1700);
      servoRight.writeMicroseconds(1500);
      inprocess = 2;
    }
  }
  else
  {
    inprocess = 0;
    //go stright
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1700);
  }
}

struct state sense(){
  struct state c;
  c.irLeft = irDetect(irLeftLED, irLeftReceive, 38000);
  c.irRight = irDetect(irRightLED, irRightReceive, 38000);

  int val = analogRead(tempSensor);             // Pin of Temp Sensor using
  double tempv = val * 5000.0 / 1024.0; // Convert the unit of Temp
  c.temp = (tempv - 750) / 10 + 25;
  c.light = analogRead(lightSensor);
  return c;
}

void senddata(struct state c){
  blueToothSerial.print(inprocess);
  blueToothSerial.print(' ');
  blueToothSerial.print(c.light);
  blueToothSerial.print(' ');
  blueToothSerial.print(c.temp);
  blueToothSerial.println('@');
}

void loop()
{
  struct state c = sense();
  control(c);
  senddata(c);
  delay(100);
}

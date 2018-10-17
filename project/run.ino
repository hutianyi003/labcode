#include <Servo.h>
#include <SoftwareSerial.h>
Servo servoLeft, servoRight;

int val = 0;
int tempSensor = A1;
int lightSensor = A0;

int irRightLED = 9;
int irRightReceive = 10;
int irLeftLED = 2;
int irLeftReceive = 3;

#define RxD 7
#define TxD 6

#define DEBUG_ENABLED 1

SoftwareSerial blueToothSerial(RxD,TxD);

void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=Slave3+\r\n");    // set the bluetooth name as "SeeedBTSlave"
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

void loop()
{

  int irRight = irDetect(9, 10, 38000);
  Serial.print(irRight);
  //Serial.print(' ');

  int irLeft = irDetect(2, 3, 38000);
  //Serial.println(irLeft);
  delay(100);

  if (irLeft == 0 || irRight == 0)
  {
    tone(4, 3000, 1000);
    servoLeft.writeMicroseconds(1700);
    servoRight.writeMicroseconds(1300);
    delay(1000);

    if (irLeft == 0)
    {
      servoLeft.writeMicroseconds(1500);
      servoRight.writeMicroseconds(1300);
      delay(1200);
    }
    else if (irRight == 0)
    {
      servoLeft.writeMicroseconds(1700);
      servoRight.writeMicroseconds(1500);
      delay(1200);
    }
  }
  else
  {
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1700);
  }
  delay(50);

  val = analogRead(lightSensor); // Light Sensor
  //Serial.print(val);
  //Serial.print(' ');

  //output();

  blueToothSerial.print(irRight);
  blueToothSerial.print(' ');
  blueToothSerial.println(irLeft);
}

/*
This program drives the robot running at full speed.
When it meets obstacle it will tune, reverse for 1 second and then turn left or right accordingly.
Please note the usage of servo.writeMicroseconds.
*/
#include <Servo.h>
Servo servoLeft, servoRight;
void setup() // Built-in initialization block
{
    tone(4, 3000, 1000); // Play tone for 1 second
    delay(1000);         // Delay to finish tone
    pinMode(7, INPUT);   // Set right whisker pin to input
    pinMode(5, INPUT);   // Set left whisker pin to input

    servoLeft.attach(13);  // Attach left signal to pin 13
    servoRight.attach(12); // Attach right signal to pin 12
    Serial.begin(9600);    // Set data rate to 9600 bps
}

void loop() // Main loop auto-repeats
{
    byte wLeft = digitalRead(5);  // Copy left result to wLeft
    byte wRight = digitalRead(7); // Copy right result to wRight

    Serial.print(wLeft);    // Display left whisker state
    Serial.println(wRight); // Display right whisker state
                            // Right wheel clockwise
    if (wLeft == 0 || wRight == 0)
    {
        tone(4, 3000, 1000);
        servoLeft.writeMicroseconds(1700); // Left wheel counterclockwise
        servoRight.writeMicroseconds(1300);
        delay(1000);

        if (wLeft == 0)
        {
            servoRight.writeMicroseconds(1700);
            servoLeft.writeMicroseconds(1500);
            delay(1200);
        }
        else if (wRight == 0)
        {
            servoRight.writeMicroseconds(1500);
            servoLeft.writeMicroseconds(1300);
            delay(1200);
        }
    }
    else
    {
        servoLeft.writeMicroseconds(1300); // Left wheel counterclockwise
        servoRight.writeMicroseconds(1700);
    }
    delay(50); // Pause for 50 ms
}